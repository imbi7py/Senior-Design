///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (2017) Alexander Stukowski
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

#pragma once


#include <plugins/particles/Particles.h>
#include <plugins/particles/modifier/analysis/StructureIdentificationModifier.h>
#include <plugins/particles/objects/ParticleProperty.h>
#include <core/dataset/pipeline/ModifierApplication.h>

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Modifiers) OVITO_BEGIN_INLINE_NAMESPACE(Analysis)

/**
 * \brief A modifier that uses the Polyhedral Template Matching (PTM) method to identify
 *        local coordination structures.
 */
class OVITO_PARTICLES_EXPORT PolyhedralTemplateMatchingModifier : public StructureIdentificationModifier
{
	Q_OBJECT
	OVITO_CLASS(PolyhedralTemplateMatchingModifier)

	Q_CLASSINFO("DisplayName", "Polyhedral template matching");
	Q_CLASSINFO("ModifierCategory", "Analysis");

public:

#ifndef Q_CC_MSVC
	/// The maximum number of neighbor atoms taken into account for the PTM analysis.
	static constexpr int MAX_NEIGHBORS = 18;
#else
	enum { MAX_NEIGHBORS = 18 };
#endif

	/// The structure types recognized by the PTM library.
	enum StructureType {
		OTHER = 0,				//< Unidentified structure
		FCC,					//< Face-centered cubic
		HCP,					//< Hexagonal close-packed
		BCC,					//< Body-centered cubic
		ICO,					//< Icosahedral structure
		SC,						//< Simple cubic structure

		NUM_STRUCTURE_TYPES 	//< This just counts the number of defined structure types.
	};
	Q_ENUMS(StructureType);

	/// The alloy types recognized by the PTM library.
	enum AlloyType {
		ALLOY_NONE = 0,
		ALLOY_PURE = 1,
		ALLOY_L10 = 2,
		ALLOY_L12_CU = 3,
		ALLOY_L12_AU = 4,
		ALLOY_B2 = 5,

		NUM_ALLOY_TYPES 	//< This just counts the number of defined alloy types.
	};
	Q_ENUMS(AlloyType);

public:

	/// Constructor.
	Q_INVOKABLE PolyhedralTemplateMatchingModifier(DataSet* dataset);

	/// Creates a new modifier application that refers to this modifier instance.
	virtual OORef<ModifierApplication> createModifierApplication() override;
		
protected:

	/// Creates a computation engine that will compute the modifier's results.
	virtual Future<ComputeEnginePtr> createEngine(TimePoint time, ModifierApplication* modApp, const PipelineFlowState& input) override;
	
private:

	/// Holds the modifier's results.
	class PTMResults : public StructureIdentificationResults
	{
	public:

		/// Constructor.
		PTMResults(size_t particleCount, bool outputInteratomicDistance, bool outputOrientation, bool outputDeformationGradient, bool outputAlloyTypes) :
			StructureIdentificationResults(particleCount),
			_rmsd(std::make_shared<PropertyStorage>(particleCount, PropertyStorage::Float, 1, 0, tr("RMSD"), false)),
			_interatomicDistances(outputInteratomicDistance ? std::make_shared<PropertyStorage>(particleCount, PropertyStorage::Float, 1, 0, tr("Interatomic Distance"), true) : nullptr),
			_orientations(outputOrientation ? ParticleProperty::createStandardStorage(particleCount, ParticleProperty::OrientationProperty, true) : nullptr),
			_deformationGradients(outputDeformationGradient ? ParticleProperty::createStandardStorage(particleCount, ParticleProperty::ElasticDeformationGradientProperty, true) : nullptr),
			_alloyTypes(outputAlloyTypes ? std::make_shared<PropertyStorage>(particleCount, PropertyStorage::Int, 1, 0, tr("Alloy Type"), true) : nullptr) {}

		/// Injects the computed results into the data pipeline.
		virtual PipelineFlowState apply(TimePoint time, ModifierApplication* modApp, const PipelineFlowState& input) override;

		const PropertyPtr& rmsd() const { return _rmsd; }
		const PropertyPtr& interatomicDistances() const { return _interatomicDistances; }
		const PropertyPtr& orientations() const { return _orientations; }
		const PropertyPtr& deformationGradients() const { return _deformationGradients; }
		const PropertyPtr& alloyTypes() const { return _alloyTypes; }
		
		/// Returns the histogram of computed RMSD values.
		const QVector<int>& rmsdHistogramData() const { return _rmsdHistogramData; }

		/// Returns the bin size of the RMSD histogram.
		FloatType rmsdHistogramBinSize() const { return _rmsdHistogramBinSize; }
	
		/// Replaces the stored histogram data.
		void setRmsdHistogram(QVector<int> counts, FloatType rmsdHistogramBinSize) {
			_rmsdHistogramData = std::move(counts);
			_rmsdHistogramBinSize = rmsdHistogramBinSize;
		}

	protected:
		
		/// Post-processes the per-particle structure types before they are output to the data pipeline.
		virtual PropertyPtr postProcessStructureTypes(TimePoint time, ModifierApplication* modApp, const PropertyPtr& structures) override;

	private:

		const PropertyPtr _rmsd;
		const PropertyPtr _interatomicDistances;
		const PropertyPtr _orientations;
		const PropertyPtr _deformationGradients;
		const PropertyPtr _alloyTypes;
		/// The histogram of computed RMSD values.
		QVector<int> _rmsdHistogramData;	
		/// The bin size of the RMSD histogram;
		FloatType _rmsdHistogramBinSize = 0;
	};
	
	/// Analysis engine that performs the PTM.
	class PTMEngine : public StructureIdentificationEngine
	{
	public:

		/// Constructor.
		PTMEngine(ConstPropertyPtr positions, ConstPropertyPtr particleTypes, const SimulationCell& simCell,
				QVector<bool> typesToIdentify, ConstPropertyPtr selection,
				bool outputInteratomicDistance, bool outputOrientation, bool outputDeformationGradient, bool outputAlloyTypes) :
			StructureIdentificationEngine(positions, simCell, std::move(typesToIdentify), std::move(selection)),
			_particleTypes(std::move(particleTypes)),
			_results(std::make_shared<PTMResults>(positions->size(), outputInteratomicDistance, outputOrientation, outputDeformationGradient, outputAlloyTypes)) {}

		/// Computes the modifier's results.
		virtual void perform() override;

	private:

		const ConstPropertyPtr _particleTypes;
		std::shared_ptr<PTMResults> _results;
	};

private:

	/// The RMSD cutoff.
	DECLARE_MODIFIABLE_PROPERTY_FIELD_FLAGS(FloatType, rmsdCutoff, setRmsdCutoff, PROPERTY_FIELD_MEMORIZE);

	/// Controls the output of the per-particle RMSD values.
	DECLARE_MODIFIABLE_PROPERTY_FIELD(bool, outputRmsd, setOutputRmsd);

	/// Controls the output of local interatomic distances.
	DECLARE_MODIFIABLE_PROPERTY_FIELD_FLAGS(bool, outputInteratomicDistance, setOutputInteratomicDistance, PROPERTY_FIELD_MEMORIZE);

	/// Controls the output of local orientations.
	DECLARE_MODIFIABLE_PROPERTY_FIELD_FLAGS(bool, outputOrientation, setOutputOrientation, PROPERTY_FIELD_MEMORIZE);

	/// Controls the output of elastic deformation gradients.
	DECLARE_MODIFIABLE_PROPERTY_FIELD(bool, outputDeformationGradient, setOutputDeformationGradient);

	/// Controls the output of alloy structure types.
	DECLARE_MODIFIABLE_PROPERTY_FIELD_FLAGS(bool, outputAlloyTypes, setOutputAlloyTypes, PROPERTY_FIELD_MEMORIZE);
};


/**
 * \brief The type of ModifierApplication created for a PolyhedralTemplateMatchingModifier 
 *        when it is inserted into in a data pipeline. It stores the last computation results
 *        so that they can be displayed in the modifier's user interface.
 */
class OVITO_PARTICLES_EXPORT PolyhedralTemplateMatchingModifierApplication : public StructureIdentificationModifierApplication
{
	Q_OBJECT
	OVITO_CLASS(PolyhedralTemplateMatchingModifierApplication)

public:
 
	/// Constructor.
	Q_INVOKABLE PolyhedralTemplateMatchingModifierApplication(DataSet* dataset) : StructureIdentificationModifierApplication(dataset) {}
 
	/// Returns the histogram of computed RMSD values.
	const QVector<int>& rmsdHistogramData() const { return _rmsdHistogramData; }

	/// Returns the bin size of the RMSD histogram.
	FloatType rmsdHistogramBinSize() const { return _rmsdHistogramBinSize; }
	 
	/// Replaces the stored histogram data.
	void setRmsdHistogram(QVector<int> counts, FloatType rmsdHistogramBinSize) {
		_rmsdHistogramData = std::move(counts);
		_rmsdHistogramBinSize = rmsdHistogramBinSize;
		notifyDependents(ReferenceEvent::ObjectStatusChanged);
	}
 
private:
 
	/// The histogram of computed RMSD values.
	QVector<int> _rmsdHistogramData;	

	/// The bin size of the RMSD histogram;
	FloatType _rmsdHistogramBinSize = 0;
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace


