///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (2013) Alexander Stukowski
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

#include <plugins/particles/Particles.h>
#include <plugins/particles/import/ParticleFrameData.h>
#include <core/utilities/io/NumberParsing.h>
#include <core/utilities/io/CompressedTextReader.h>
#include "POSCARImporter.h"

#include <QRegularExpression>

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Import) OVITO_BEGIN_INLINE_NAMESPACE(Formats)

IMPLEMENT_OVITO_CLASS(POSCARImporter);	

/******************************************************************************
* Checks if the given file has format that can be read by this importer.
******************************************************************************/
bool POSCARImporter::OOMetaClass::checkFileFormat(QFileDevice& input, const QUrl& sourceLocation) const
{
	// Regular expression for whitespace characters.
	QRegularExpression ws_re(QStringLiteral("\\s+"));

	// Open input file.
	CompressedTextReader stream(input, sourceLocation.path());

	// Skip comment line
	stream.readLine();

	// Read global scaling factor
	double scaling_factor;
	stream.readLine();
	if(stream.eof() || sscanf(stream.line(), "%lg", &scaling_factor) != 1 || scaling_factor <= 0)
		return false;

	// Read cell matrix
	for(int i = 0; i < 3; i++) {
		stream.readLine();
		if(stream.lineString().split(ws_re, QString::SkipEmptyParts).size() != 3)
			return false;
		double x,y,z;
		if(sscanf(stream.line(), "%lg %lg %lg", &x, &y, &z) != 3 || stream.eof())
			return false;
	}

	// Parse number of atoms per type.
	int nAtomTypes = 0;
	for(int i = 0; i < 2; i++) {
		stream.readLine();
		QStringList tokens = stream.lineString().split(ws_re, QString::SkipEmptyParts);
		if(i == 0) nAtomTypes = tokens.size();
		else if(nAtomTypes != tokens.size())
			return false;
		int n = 0;
		for(const QString& token : tokens) {
			bool ok;
			n += token.toInt(&ok);
		}
		if(n > 0)
			return true;
	}

	return false;
}

/******************************************************************************
* Determines whether the input file should be scanned to discover all contained frames.
******************************************************************************/
bool POSCARImporter::shouldScanFileForFrames(const QUrl& sourceUrl)
{
	return sourceUrl.fileName().contains(QStringLiteral("XDATCAR"));
}

/******************************************************************************
* Scans the given input file to find all contained simulation frames.
******************************************************************************/
void POSCARImporter::FrameFinder::discoverFramesInFile(QFile& file, const QUrl& sourceUrl, QVector<FileSourceImporter::Frame>& frames)
{
	CompressedTextReader stream(file, sourceUrl.path());
	setProgressText(tr("Scanning file %1").arg(sourceUrl.toString(QUrl::RemovePassword | QUrl::PreferLocalFile | QUrl::PrettyDecoded)));
	setProgressMaximum(stream.underlyingSize() / 1000);

	// Regular expression for whitespace characters.
	QRegularExpression ws_re(QStringLiteral("\\s+"));

	QFileInfo fileInfo(stream.device().fileName());
	QString filename = fileInfo.fileName();
	int frameNumber = 0;

	// Skip comment line
	stream.readLine();

	// Skip scaling factor
	stream.readLine();

	// Skip cell matrix
	for(int i = 0; i < 3; i++)
		stream.readLine();

	// Parse atom type names and atom type counts.
	QStringList atomTypeNames;
	QVector<int> atomCounts;
	parseAtomTypeNamesAndCounts(stream, atomTypeNames, atomCounts);

	// Read successive frames.
	Frame frame;
	frame.sourceFile = sourceUrl;
	frame.lastModificationTime = fileInfo.lastModified();
	while(!stream.eof() && !isCanceled()) {
		frame.byteOffset = stream.byteOffset();
		frame.lineNumber = stream.lineNumber();
		frame.label = QString("%1 (Frame %2)").arg(filename).arg(frameNumber++);
		frames.push_back(frame);

		stream.readLine();
		for(int acount : atomCounts) {
			for(int i = 0; i < acount; i++) {
				stream.readLine();
			}
		}

		setProgressValueIntermittent(stream.underlyingByteOffset() / 1000);
		if(isCanceled())
			return;
	}
}

/******************************************************************************
* Parses the given input file.
******************************************************************************/
FileSourceImporter::FrameDataPtr POSCARImporter::FrameLoader::loadFile(QFile& file)
{
	// Open file for reading.
	CompressedTextReader stream(file, frame().sourceFile.path());
	setProgressText(tr("Reading POSCAR file %1").arg(frame().sourceFile.toString(QUrl::RemovePassword | QUrl::PreferLocalFile | QUrl::PrettyDecoded)));

	// Create the destination container for loaded data.
	std::shared_ptr<ParticleFrameData> frameData = std::make_shared<ParticleFrameData>();

	// Regular expression for whitespace characters.
	QRegularExpression ws_re(QStringLiteral("\\s+"));

	// Read comment line.
	stream.readLine();
	QString trimmedComment = stream.lineString().trimmed();
	if(!trimmedComment.isEmpty())
		frameData->attributes().insert(QStringLiteral("Comment"), QVariant::fromValue(trimmedComment));

	// Read global scaling factor
	FloatType scaling_factor = 0;
	if(sscanf(stream.readLine(), FLOATTYPE_SCANF_STRING, &scaling_factor) != 1 || scaling_factor <= 0)
		throw Exception(tr("Invalid scaling factor (line 1): %1").arg(stream.lineString()));

	// Read cell matrix
	AffineTransformation cell = AffineTransformation::Identity();
	for(size_t i = 0; i < 3; i++) {
		if(sscanf(stream.readLine(),
				FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING,
				&cell(0,i), &cell(1,i), &cell(2,i)) != 3 || cell.column(i) == Vector3::Zero())
			throw Exception(tr("Invalid cell vector (line %1): %2").arg(stream.lineNumber()).arg(stream.lineString()));
	}
	cell = cell * scaling_factor;
	frameData->simulationCell().setMatrix(cell);

	// Parse atom type names and atom type counts.
	QStringList atomTypeNames;
	QVector<int> atomCounts;
	parseAtomTypeNamesAndCounts(stream, atomTypeNames, atomCounts);
	int totalAtomCount = std::accumulate(atomCounts.begin(), atomCounts.end(), 0);
	if(totalAtomCount <= 0)
		throw Exception(tr("Invalid atom counts (line %1): %2").arg(stream.lineNumber()).arg(stream.lineString()));

	// Jump to requested animation frame.
	if(frame().byteOffset != 0)
		stream.seek(frame().byteOffset);

	// Read in 'Selective dynamics' flag
	stream.readLine();
	if(stream.line()[0] == 'S' || stream.line()[0] == 's')
		stream.readLine();

	// Parse coordinate system.
	bool isCartesian = false;
	if(stream.line()[0] == 'C' || stream.line()[0] == 'c' || stream.line()[0] == 'K' || stream.line()[0] == 'k')
		isCartesian = true;

	// Create the particle properties.
	PropertyPtr posProperty = ParticleProperty::createStandardStorage(totalAtomCount, ParticleProperty::PositionProperty, false);
	frameData->addParticleProperty(posProperty);
	PropertyPtr typeProperty = ParticleProperty::createStandardStorage(totalAtomCount, ParticleProperty::TypeProperty, false);
	frameData->addParticleProperty(typeProperty);
	ParticleFrameData::TypeList* typeList = frameData->propertyTypesList(typeProperty);
	
	// Read atom coordinates.
	Point3* p = posProperty->dataPoint3();
	int* a = typeProperty->dataInt();
	for(int atype = 1; atype <= atomCounts.size(); atype++) {
		int typeId = atype;
		if(atomTypeNames.size() == atomCounts.size() && atomTypeNames[atype-1].isEmpty() == false)
			typeId = typeList->addTypeName(atomTypeNames[atype-1]);
		else
			typeList->addTypeId(atype);
		for(int i = 0; i < atomCounts[atype-1]; i++, ++p, ++a) {
			*a = typeId;
			if(sscanf(stream.readLine(), FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING,
					&p->x(), &p->y(), &p->z()) != 3)
				throw Exception(tr("Invalid atom coordinates (line %1): %2").arg(stream.lineNumber()).arg(stream.lineString()));
			if(!isCartesian)
				*p = cell * (*p);
			else
				*p = (*p) * scaling_factor;
		}
	}

	QString statusString = tr("%1 atoms").arg(totalAtomCount);	

	// Parse optional atomic velocity vectors or CHGCAR electron density data. 
	// Do this only for the first frame.
	if(frame().byteOffset == 0) {
		if(!stream.eof())
			stream.readLineTrimLeft();
		if(!stream.eof() && stream.line()[0] > ' ') {
			isCartesian = false;
			if(stream.line()[0] == 'C' || stream.line()[0] == 'c' || stream.line()[0] == 'K' || stream.line()[0] == 'k')
				isCartesian = true;

			// Read atomic velocities.
			PropertyPtr velocityProperty = ParticleProperty::createStandardStorage(totalAtomCount, ParticleProperty::VelocityProperty, false);
			frameData->addParticleProperty(velocityProperty);
			Vector3* v = velocityProperty->dataVector3();
			for(int atype = 1; atype <= atomCounts.size(); atype++) {
				for(int i = 0; i < atomCounts[atype-1]; i++, ++v) {
					if(sscanf(stream.readLine(), FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING,
							&v->x(), &v->y(), &v->z()) != 3)
						throw Exception(tr("Invalid atom velocity vector (line %1): %2").arg(stream.lineNumber()).arg(stream.lineString()));
					if(!isCartesian)
						*v = cell * (*v);
				}
			}
		}
		else if(!stream.eof()) {
			size_t nx, ny, nz;
			// Parse charge density volumetric grid.
			if(sscanf(stream.readLine(), "%zu %zu %zu", &nx, &ny, &nz) == 3 && nx > 0 && ny > 0 && nz > 0) {
				auto parseFieldData = [this, &stream, &frameData](size_t nx, size_t ny, size_t nz, const QString& name) -> PropertyPtr {
					PropertyPtr fieldQuantity = std::make_shared<PropertyStorage>(nx*ny*nz, PropertyStorage::Float, 1, 0, name, false);
					const char* s = stream.readLine();
					FloatType* data = fieldQuantity->dataFloat();
					setProgressMaximum(fieldQuantity->size());
					FloatType cellVolume = frameData->simulationCell().volume3D();
					for(size_t i = 0; i < fieldQuantity->size(); i++, ++data) {
						const char* token;
						for(;;) {
							while(*s == ' ' || *s == '\t') ++s;
							token = s;
							while(*s > ' ' || *s < 0) ++s;
							if(s != token) break;
							s = stream.readLine();
						}
						if(!parseFloatType(token, s, *data))
							throw Exception(tr("Invalid value in charge density section (line %1): \"%2\"").arg(stream.lineNumber()).arg(QString::fromLocal8Bit(token, s - token)));
						*data /= cellVolume;
						if(*s != '\0')
							s++;

						if(!setProgressValueIntermittent(i)) return {};						
					}
					return fieldQuantity;
				};

				frameData->setVoxelGridShape({nx, ny, nz});

				// Parse spin up + spin down denisty.
				PropertyPtr chargeDensity = parseFieldData(nx, ny, nz, tr("Charge density"));
				if(!chargeDensity) return {};
				frameData->addVoxelProperty(chargeDensity);
				statusString += tr("\nCharge density grid: %1 x %2 x %3").arg(nx).arg(ny).arg(nz);

				// Look for spin up - spin down density.
				PropertyPtr magnetizationDensity;
				while(!stream.eof()) {
					if(sscanf(stream.readLine(), "%zu %zu %zu", &nx, &ny, &nz) == 3 && nx > 0 && ny > 0 && nz > 0) {
						if(nx != frameData->voxelGridShape()[0] || ny != frameData->voxelGridShape()[1] || nz != frameData->voxelGridShape()[2])
							throw Exception(tr("Inconsistent voxel grid dimensions in line %1").arg(stream.lineNumber()));
						magnetizationDensity = parseFieldData(nx, ny, nz, tr("Magnetization density"));
						if(!magnetizationDensity) return {};
						statusString += tr("\nMagnetization density grid: %1 x %2 x %3").arg(nx).arg(ny).arg(nz);
						break;
					}
				}

				// Look for more vector components in case file contains vector magnetization. 
				PropertyPtr magnetizationDensityY;
				PropertyPtr magnetizationDensityZ;
				while(!stream.eof()) {
					if(sscanf(stream.readLine(), "%zu %zu %zu", &nx, &ny, &nz) == 3 && nx > 0 && ny > 0 && nz > 0) {
						if(nx != frameData->voxelGridShape()[0] || ny != frameData->voxelGridShape()[1] || nz != frameData->voxelGridShape()[2])
							throw Exception(tr("Inconsistent voxel grid dimensions in line %1").arg(stream.lineNumber()));
						magnetizationDensityY = parseFieldData(nx, ny, nz, tr("Magnetization density"));
						if(!magnetizationDensityY) return {};
						break;
					}
				}
				while(!stream.eof()) {
					if(sscanf(stream.readLine(), "%zu %zu %zu", &nx, &ny, &nz) == 3 && nx > 0 && ny > 0 && nz > 0) {
						if(nx != frameData->voxelGridShape()[0] || ny != frameData->voxelGridShape()[1] || nz != frameData->voxelGridShape()[2])
							throw Exception(tr("Inconsistent voxel grid dimensions in line %1").arg(stream.lineNumber()));
						magnetizationDensityZ = parseFieldData(nx, ny, nz, tr("Magnetization density"));
						if(!magnetizationDensityZ) return {};
						break;
					}
				}

				if(magnetizationDensity && magnetizationDensityY && magnetizationDensityZ) {
					PropertyPtr vectorMagnetization = std::make_shared<PropertyStorage>(nx*ny*nz, PropertyStorage::Float, 3, 0, tr("Magnetization density"), false);
					vectorMagnetization->setComponentNames(QStringList() << "X" << "Y" << "Z");
					for(size_t i = 0; i < vectorMagnetization->size(); i++) 
						vectorMagnetization->setVector3(i, { magnetizationDensity->getFloat(i), magnetizationDensityY->getFloat(i), magnetizationDensityZ->getFloat(i) });
					frameData->addVoxelProperty(std::move(vectorMagnetization));
				}
				else if(magnetizationDensity) {
					frameData->addVoxelProperty(std::move(magnetizationDensity));
				}
			}
		}
	}

	frameData->setStatus(statusString);
	return frameData;
}

/******************************************************************************
* Parses the list of atom types from the POSCAR file.
******************************************************************************/
void POSCARImporter::parseAtomTypeNamesAndCounts(CompressedTextReader& stream, QStringList& atomTypeNames, QVector<int>& atomCounts)
{
	// Regular expression for whitespace characters.
	QRegularExpression ws_re(QStringLiteral("\\s+"));

	for(int i = 0; i < 2; i++) {
		stream.readLine();
		QStringList tokens = stream.lineString().split(ws_re, QString::SkipEmptyParts);
		// Try to convert string tokens to integers.
		atomCounts.clear();
		bool ok = true;
		for(const QString& token : tokens) {
			atomCounts.push_back(token.toInt(&ok));
			if(!ok) {
				// If the casting to integer fails, then the current line contains the element names.
				// Try it again with the next line.
				atomTypeNames = tokens;
				break;
			}
		}
		if(ok)
			break;
		if(i == 1)
			throw Exception(tr("Invalid atom counts (line %1): %2").arg(stream.lineNumber()).arg(stream.lineString()));
	}
}


OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace
