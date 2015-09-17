///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (2015) Alexander Stukowski
//
//  This file is part of OVITO (Open Visualization Tool).
//
//  OVITO is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  OVITO is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////

#include <plugins/crystalanalysis/CrystalAnalysis.h>
#include <core/utilities/concurrent/ParallelFor.h>
#include "ElasticStrainEngine.h"
#include "ElasticStrainModifier.h"

namespace Ovito { namespace Plugins { namespace CrystalAnalysis {

/******************************************************************************
* Constructor.
******************************************************************************/
ElasticStrainEngine::ElasticStrainEngine(const TimeInterval& validityInterval,
		ParticleProperty* positions, const SimulationCell& simCell,
		int inputCrystalStructure, std::vector<Matrix3>&& preferredCrystalOrientations,
		bool calculateDeformationGradients, bool calculateStrainTensors,
		FloatType latticeConstant, FloatType caRatio, bool pushStrainTensorsForward) :
	StructureIdentificationModifier::StructureIdentificationEngine(validityInterval, positions, simCell),
	_structureAnalysis(positions, simCell, (StructureAnalysis::LatticeStructureType)inputCrystalStructure, selection(), structures(), std::move(preferredCrystalOrientations)),
	_inputCrystalStructure(inputCrystalStructure),
	_latticeConstant(latticeConstant),
	_caRatio(caRatio),
	_pushStrainTensorsForward(pushStrainTensorsForward),
	_volumetricStrains(new ParticleProperty(positions->size(), qMetaTypeId<FloatType>(), 1, 0, QStringLiteral("Volumetric Strain"), false)),
	_strainTensors(calculateStrainTensors ? new ParticleProperty(positions->size(), qMetaTypeId<FloatType>(), 6, 0, QStringLiteral("Elastic Strain"), false) : nullptr),
	_deformationGradients(calculateDeformationGradients ? new ParticleProperty(positions->size(), qMetaTypeId<FloatType>(), 9, 0, QStringLiteral("Elastic Deformation Gradient"), false) : nullptr)
{

	// Set component names of tensor properties.
	if(strainTensors())
		strainTensors()->setComponentNames(QStringList() << "XX" << "YY" << "ZZ" << "XY" << "XZ" << "YZ");
	if(deformationGradients())
		deformationGradients()->setComponentNames(QStringList() << "XX" << "YX" << "ZX" << "XY" << "YY" << "ZY" << "XZ" << "YZ" << "ZZ");

	// Cubic crystal structures always have a c/a ratio of one.
	if(inputCrystalStructure == StructureAnalysis::LATTICE_FCC || inputCrystalStructure == StructureAnalysis::LATTICE_BCC || inputCrystalStructure == StructureAnalysis::LATTICE_CUBIC_DIAMOND)
		_caRatio = 1;
}

/******************************************************************************
* Performs the actual analysis. This method is executed in a worker thread.
******************************************************************************/
void ElasticStrainEngine::perform()
{
	setProgressText(ElasticStrainModifier::tr("Calculating elastic strain tensors"));

	beginProgressSubSteps({ 35, 6, 1, 1, 20 });
	if(!_structureAnalysis.identifyStructures(*this))
		return;

	nextProgressSubStep();
	if(!_structureAnalysis.buildClusters(*this))
		return;

	nextProgressSubStep();
	if(!_structureAnalysis.connectClusters(*this))
		return;

	nextProgressSubStep();
	if(!_structureAnalysis.formSuperClusters(*this))
		return;

	nextProgressSubStep();
	parallelFor(positions()->size(), *this, [this](size_t particleIndex) {

		Cluster* localCluster = _structureAnalysis.atomCluster(particleIndex);
		if(localCluster->id != 0) {

			// The shape of the ideal unit cell.
			Matrix3 idealUnitCellTM(_latticeConstant, 0, 0,
									0, _latticeConstant, 0,
									0, 0, _latticeConstant * _caRatio);

			// If the cluster is a defect (stacking fault), find the parent crystal cluster.
			Cluster* parentCluster = nullptr;
			if(localCluster->structure != _inputCrystalStructure) {
				for(ClusterTransition* t = localCluster->transitions; t != nullptr; t = t->next) {
					if(t->cluster2->structure == _inputCrystalStructure) {
						parentCluster = t->cluster2;
						idealUnitCellTM = t->tm * idealUnitCellTM;
						break;
					}
				}
			}
			else parentCluster = localCluster;

			if(parentCluster != nullptr) {
				// For calculating the cluster orientation.
				Matrix_3<double> orientationV = Matrix_3<double>::Zero();
				Matrix_3<double> orientationW = Matrix_3<double>::Zero();

				int numneigh = _structureAnalysis.numNeighbors(particleIndex);
				for(int n = 0; n < numneigh; n++) {
					int neighborAtomIndex = _structureAnalysis.getNeighbor(particleIndex, n);
					// Add vector pair to matrices for computing the elastic deformation gradient.
					Vector3 latticeVector = idealUnitCellTM * _structureAnalysis.neighborLatticeVector(particleIndex, n);
					const Vector3& spatialVector = cell().wrapVector(positions()->getPoint3(neighborAtomIndex) - positions()->getPoint3(particleIndex));
					for(size_t i = 0; i < 3; i++) {
						for(size_t j = 0; j < 3; j++) {
							orientationV(i,j) += (double)(latticeVector[j] * latticeVector[i]);
							orientationW(i,j) += (double)(latticeVector[j] * spatialVector[i]);
						}
					}
				}

				// Calculate deformation gradient tensor.
				Matrix_3<double> elasticF = orientationW * orientationV.inverse();
				if(deformationGradients()) {
					for(size_t col = 0; col < 3; col++) {
						for(size_t row = 0; row < 3; row++) {
							deformationGradients()->setFloatComponent(particleIndex, col*3+row, (FloatType)elasticF(row,col));
						}
					}
				}

				// Calculate strain tensor.
				SymmetricTensor2T<double> elasticStrain;
				if(!_pushStrainTensorsForward) {
					// Compute Green strain tensor in material frame.
					elasticStrain = (Product_AtA(elasticF) - SymmetricTensor2T<double>::Identity()) * 0.5;
				}
				else {
					// Compute Euler strain tensor in spatial frame.
					Matrix_3<double> inverseF;
					if(!elasticF.inverse(inverseF))
						throw Exception(ElasticStrainModifier::tr("Cannot compute strain tensor in spatial reference frame, because the elastic deformation gradient at atom index %1 is singular.").arg(particleIndex+1));
					elasticStrain = (SymmetricTensor2T<double>::Identity() - Product_AtA(inverseF)) * 0.5;
				}

				// Store strain tensor in output property.
				if(strainTensors()) {
					strainTensors()->setSymmetricTensor2(particleIndex, (SymmetricTensor2)elasticStrain);
				}

				// Calculate volumetric strain component.
				double volumetricStrain = (elasticStrain(0,0) + elasticStrain(1,1) + elasticStrain(2,2)) / 3.0;
				OVITO_ASSERT(std::isfinite(volumetricStrain));
				volumetricStrains()->setFloat(particleIndex, (FloatType)volumetricStrain);

				return;
			}
		}

		// Mark atom as invalid.
		volumetricStrains()->setFloat(particleIndex, 0);
		if(strainTensors()) {
			for(size_t component = 0; component < 6; component++)
				strainTensors()->setFloatComponent(particleIndex, component, 0);
		}
		if(deformationGradients()) {
			for(size_t component = 0; component < 9; component++)
				deformationGradients()->setFloatComponent(particleIndex, component, 0);
		}
	});

	endProgressSubSteps();
}

}	// End of namespace
}	// End of namespace
}	// End of namespace

