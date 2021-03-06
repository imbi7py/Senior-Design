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

#include <plugins/particles/Particles.h>
#include <plugins/particles/objects/ParticleProperty.h>
#include <plugins/stdobj/simcell/SimulationCellObject.h>
#include <core/utilities/concurrent/Promise.h>
#include "AMBERNetCDFExporter.h"

#include <QtMath>
#include <netcdf.h>

#define NCERR(x) ncerr(x, __FILE__, __LINE__)
#define NCERRI(x, info) ncerr_with_info(x, __FILE__, __LINE__, info)

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Export) OVITO_BEGIN_INLINE_NAMESPACE(Formats)

const char NC_FRAME_STR[]         = "frame";
const char NC_SPATIAL_STR[]       = "spatial";
const char NC_VOIGT_STR[]         = "Voigt";
const char NC_ATOM_STR[]          = "atom";
const char NC_CELL_SPATIAL_STR[]  = "cell_spatial";
const char NC_CELL_ANGULAR_STR[]  = "cell_angular";
const char NC_LABEL_STR[]         = "label";

const char NC_TIME_STR[]          = "time";
const char NC_CELL_ORIGIN_STR[]   = "cell_origin";
const char NC_CELL_LENGTHS_STR[]  = "cell_lengths";
const char NC_CELL_ANGLES_STR[]   = "cell_angles";

const char NC_UNITS_STR[]         = "units";
const char NC_SCALE_FACTOR_STR[]  = "scale_factor";

#ifdef FLOATTYPE_FLOAT 
	#define NC_OVITO_FLOATTYPE NC_FLOAT
#else
	#define NC_OVITO_FLOATTYPE NC_DOUBLE
#endif

IMPLEMENT_OVITO_CLASS(AMBERNetCDFExporter);

/******************************************************************************
* Check for NetCDF error and throw exception
******************************************************************************/
void AMBERNetCDFExporter::ncerr(int err, const char* file, int line)
{
	if(err != NC_NOERR)
		throwException(tr("NetCDF I/O error: %1 (line %2 of %3)").arg(QString(nc_strerror(err))).arg(line).arg(file));
}

/******************************************************************************
* Check for NetCDF error and throw exception (and attach additional information
* to exception string)
******************************************************************************/
void AMBERNetCDFExporter::ncerr_with_info(int err, const char* file, int line, const QString& info)
{
	if(err != NC_NOERR)
		throwException(tr("NetCDF I/O error: %1 %2 (line %3 of %4)").arg(QString(nc_strerror(err))).arg(info).arg(line).arg(file));
}

/******************************************************************************
 * This is called once for every output file to be written and before
 * exportFrame() is called.
 *****************************************************************************/
bool AMBERNetCDFExporter::openOutputFile(const QString& filePath, int numberOfFrames)
{
	OVITO_ASSERT(!outputFile().isOpen());
	outputFile().setFileName(filePath);

	// Open the input file for writing.
	NCERR(nc_create(filePath.toLocal8Bit().constData(), NC_64BIT_DATA, &_ncid));

	// Define dimensions.
	NCERR(nc_def_dim(_ncid, NC_FRAME_STR, NC_UNLIMITED, &_frame_dim));
	NCERR(nc_def_dim(_ncid, NC_SPATIAL_STR, 3, &_spatial_dim));
	NCERR(nc_def_dim(_ncid, NC_VOIGT_STR, 6, &_Voigt_dim));
	NCERR(nc_def_dim(_ncid, NC_CELL_SPATIAL_STR, 3, &_cell_spatial_dim));
	NCERR(nc_def_dim(_ncid, NC_CELL_ANGULAR_STR, 3, &_cell_angular_dim));
	NCERR(nc_def_dim(_ncid, NC_LABEL_STR, 10, &_label_dim));

	// Default variables.
	int dims[NC_MAX_VAR_DIMS];
	dims[0] = _spatial_dim;
	NCERR(nc_def_var(_ncid, NC_SPATIAL_STR, NC_CHAR, 1, dims, &_spatial_var));
	NCERR(nc_def_var(_ncid, NC_CELL_SPATIAL_STR, NC_CHAR, 1, dims, &_cell_spatial_var));
	dims[0] = _spatial_dim;
	dims[1] = _label_dim;
	NCERR(nc_def_var(_ncid, NC_CELL_ANGULAR_STR, NC_CHAR, 2, dims, &_cell_angular_var));
	dims[0] = _frame_dim;
	NCERR(nc_def_var(_ncid, NC_TIME_STR, NC_DOUBLE, 1, dims, &_time_var));
	dims[0] = _frame_dim;
	dims[1] = _cell_spatial_dim;
	NCERR(nc_def_var(_ncid, NC_CELL_ORIGIN_STR, NC_DOUBLE, 2, dims, &_cell_origin_var));
	NCERR(nc_def_var(_ncid, NC_CELL_LENGTHS_STR, NC_DOUBLE, 2, dims, &_cell_lengths_var));
	dims[0] = _frame_dim;
	dims[1] = _cell_angular_dim;
	NCERR(nc_def_var(_ncid, NC_CELL_ANGLES_STR, NC_DOUBLE, 2, dims, &_cell_angles_var));

	// Global attributes.
	NCERR(nc_put_att_text(_ncid, NC_GLOBAL, "Conventions", 5, "AMBER"));
	NCERR(nc_put_att_text(_ncid, NC_GLOBAL, "ConventionVersion", 3, "1.0"));
	NCERR(nc_put_att_text(_ncid, NC_GLOBAL, "program", 5, "OVITO"));
	QByteArray programVersion = QCoreApplication::applicationVersion().toLocal8Bit();
	NCERR(nc_put_att_text(_ncid, NC_GLOBAL, "programVersion", programVersion.length(), programVersion.constData()));

	NCERR(nc_put_att_text(_ncid, _cell_angles_var, NC_UNITS_STR, 6, "degree"));

	// Done with definitions.
	NCERR(nc_enddef(_ncid));

	// Write label variables.
	size_t index[NC_MAX_VAR_DIMS], count[NC_MAX_VAR_DIMS];
	NCERR(nc_put_var_text(_ncid, _spatial_var, "xyz"));
	NCERR(nc_put_var_text(_ncid, _cell_spatial_var, "abc"));
	index[0] = 0;
	index[1] = 0;
	count[0] = 1;
	count[1] = 5;
	NCERR(nc_put_vara_text(_ncid, _cell_angular_var, index, count, "alpha"));
	index[0] = 1;	
	count[1] = 4;
	NCERR(nc_put_vara_text(_ncid, _cell_angular_var, index, count, "beta"));
	index[0] = 2;
	count[1] = 5;
	NCERR(nc_put_vara_text(_ncid, _cell_angular_var, index, count, "gamma"));

	// Number of simulation frames written so far.
	_frameCounter = 0;

	return true;
}

/******************************************************************************
 * This is called once for every output file written after exportFrame()
 * has been called.
 *****************************************************************************/
void AMBERNetCDFExporter::closeOutputFile(bool exportCompleted)
{
	OVITO_ASSERT(!outputFile().isOpen());

	if(_ncid != -1) {
		NCERR(nc_close(_ncid));
		_ncid = -1;
	}
	_atom_dim = -1;

	if(!exportCompleted)
		outputFile().remove();
}

/******************************************************************************
* Writes the particles of one animation frame to the current output file.
******************************************************************************/
bool AMBERNetCDFExporter::exportObject(SceneNode* sceneNode, int frameNumber, TimePoint time, const QString& filePath, TaskManager& taskManager)
{
	// Get particle data to be exported.
	PipelineFlowState state;
	if(!getParticleData(sceneNode, time, state, taskManager))
		return false;

	Promise<> exportTask = Promise<>::createSynchronous(&taskManager, true, true);
	exportTask.setProgressText(tr("Writing file %1").arg(filePath));
	
	// Get particle positions.
	ParticleProperty* posProperty = ParticleProperty::findInState(state, ParticleProperty::PositionProperty);

	// Get simulation cell info.
	SimulationCellObject* simulationCell = state.findObject<SimulationCellObject>();
	const AffineTransformation simCell = simulationCell ? simulationCell->cellMatrix() : AffineTransformation::Zero();
	size_t atomsCount = posProperty->size();
	
	// Define the "atom" dimension when writing first frame and the number of atoms is known.
	if(_atom_dim == -1) {
		NCERR(nc_redef(_ncid));
		NCERR(nc_def_dim(_ncid, NC_ATOM_STR, atomsCount, &_atom_dim));

		// Define NetCDF variables for global attributes.
		const QVariantMap& attributes = state.attributes();
		for(auto entry = attributes.constBegin(); entry != attributes.constEnd(); ++entry) {
			int var;
			if(entry.key() == NC_TIME_STR || entry.key() == QStringLiteral("SourceFrame"))
				continue;
			else if((QMetaType::Type)entry.value().type() == QMetaType::Double || (QMetaType::Type)entry.value().type() == QMetaType::Float)
				NCERR(nc_def_var(_ncid, entry.key().toUtf8().constData(), NC_DOUBLE, 1, &_frame_dim, &var));
			else if(entry.value().canConvert<int>())
				NCERR(nc_def_var(_ncid, entry.key().toUtf8().constData(), NC_INT, 1, &_frame_dim, &var));
			else
				continue;
			_attributes_vars.insert(entry.key(), var);
		}

		// Define NetCDF variable for atomic positions.
		int dims[3] = { _frame_dim, _atom_dim, _spatial_dim };
		NCERR(nc_def_var(_ncid, "coordinates", NC_OVITO_FLOATTYPE, 3, dims, &_coords_var));

		// Define a NetCDF variable for every per-particle property to be exported.
		for(auto c = columnMapping().begin(); c != columnMapping().end(); ++c) {

			// Skip the particle position property. It has already been emitted above.
			if(c->type() == ParticleProperty::PositionProperty)
				continue;
				
			// We can export a particle property only as a whole to a NetCDF file, not individual components.
			// Skip this column if we have already emitted an entry for the same particle property before.
			if(std::find_if(columnMapping().begin(), c, [c](const ParticlePropertyReference& pr) { return pr.name() == c->name(); }) != c)
				continue;

			ParticleProperty* prop = c->findInState(state);
			if(!prop)
				throwException(tr("Invalid list of particle properties to be exported. The property '%1' does not exist.").arg(c->name()));
			if((int)prop->componentCount() <= std::max(0, c->vectorComponent()))
				throwException(tr("The output vector component selected for column %1 is out of range. The particle property '%2' has only %3 component(s).").arg(c - columnMapping().begin() + 1).arg(c->name()).arg(prop->componentCount()));

			// For certain standard properties we need to use NetCDF variables according to the AMBER convention.
			// All other properties are output as NetCDF variables under their normal name.
			const char* mangledName = nullptr;
			dims[2] = 0;
			if(prop->type() != ParticleProperty::UserProperty) {
				if(prop->type() == ParticleProperty::ForceProperty) {
					mangledName = "forces";
					dims[2] = _spatial_dim;
				}
				else if(prop->type() == ParticleProperty::VelocityProperty) {
					mangledName = "velocities";
					dims[2] = _spatial_dim;
				}
				else if(prop->type() == ParticleProperty::TypeProperty) {
					mangledName = "atom_types";
				}
			}

			// Create the dimension for the NetCDF variable if the property is a vector property.
			if(dims[2] == 0 && prop->componentCount() > 1) {
				NCERR(nc_def_dim(_ncid, qPrintable(QStringLiteral("dim_") + prop->name()), prop->componentCount(), &dims[2]));
			}
			
			// Create the NetCDF variable for the property.
			nc_type ncDataType;
			if(prop->dataType() == PropertyStorage::Int) ncDataType = NC_INT;
			else if(prop->dataType() == PropertyStorage::Int64) ncDataType = NC_INT64;
			else if(prop->dataType() == PropertyStorage::Float) ncDataType = NC_OVITO_FLOATTYPE;
			else continue;
			// For scalar OVITO properties we define a NetCDF variable with 2 dimensions.
			// For vector OVITO properties we define a NetCDF variable with 3 dimensions.
			int ncvar;
			NCERR(nc_def_var(_ncid, mangledName ? mangledName : qPrintable(c->name()), ncDataType, (prop->componentCount() > 1) ? 3 : 2, dims, &ncvar));
			_columns.emplace_back(*c, prop->dataType(), prop->componentCount(), ncvar);
		}
		
		NCERR(nc_enddef(_ncid));
	}
	else {
		// If this is not the first frame we are writing, make sure the number of atoms remained the same.
		size_t na;
		NCERR(nc_inq_dimlen(_ncid, _atom_dim, &na));
		if(na != atomsCount)
			throwException(tr("Number of particles did change between animation frames. Writing a NetCDF trajectory file with "
				"a varying number of atoms is not supported by the AMBER format convention."));
	}

	// Write global attributes to the NetCDF file.
	const QVariantMap& attributes = state.attributes();
	for(auto entry = _attributes_vars.constBegin(); entry != _attributes_vars.constEnd(); ++entry) {		
		QVariant val = attributes.value(entry.key());
		if(val.type() == (int)QMetaType::Double || val.type() == (int)QMetaType::Float) {
			double d = val.toDouble();
			NCERR(nc_put_var1_double(_ncid, entry.value(), &_frameCounter, &d));
		}
		else if(val.canConvert<int>()) {
			int i = val.toInt();
			NCERR(nc_put_var1_int(_ncid, entry.value(), &_frameCounter, &i));
		}
	}

	// Write "time" variable.
	double t = _frameCounter;
	if(attributes.contains(NC_TIME_STR)) t = attributes.value(NC_TIME_STR).toDouble();
	else if(state.sourceFrame() >= 0) t = state.sourceFrame();
    NCERR(nc_put_var1_double(_ncid, _time_var, &_frameCounter, &t));

	// Write simulation cell.
	double cell_origin[3], cell_lengths[3], cell_angles[3];

	cell_origin[0] = simCell.translation().x();
	cell_origin[1] = simCell.translation().y();
	cell_origin[2] = simCell.translation().z();

	cell_lengths[0] = simCell.column(0).length();
	cell_lengths[1] = simCell.column(1).length();
	cell_lengths[2] = simCell.column(2).length();

	double h[6] = { simCell(0,0), simCell(1,1), simCell(2,2), simCell(0,1), simCell(0,2), simCell(1,2) };
	double cosalpha = (h[5]*h[4]+h[1]*h[3])/sqrt((h[1]*h[1]+h[5]*h[5])*(h[2]*h[2]+h[3]*h[3]+h[4]*h[4]));
	double cosbeta = h[4]/sqrt(h[2]*h[2]+h[3]*h[3]+h[4]*h[4]);
	double cosgamma = h[5]/sqrt(h[1]*h[1]+h[5]*h[5]);

	if(simCell(2,1) != 0 || simCell(2,0) != 0 || simCell(1,0) != 0)
		qWarning() << "Warning: Simulation cell vectors are not compatible with the AMBER file specification. Generated NetCDF file may be invalid.";

	cell_angles[0] = qRadiansToDegrees(acos(cosalpha));
	cell_angles[1] = qRadiansToDegrees(acos(cosbeta));
	cell_angles[2] = qRadiansToDegrees(acos(cosgamma));

	// AMBER convention says that nonperiodic boundaries should have 'cell_lengths' set to zero.
	if(!simulationCell->pbcX()) cell_lengths[0] = 0;
	if(!simulationCell->pbcY()) cell_lengths[1] = 0;
	if(!simulationCell->pbcZ()) cell_lengths[2] = 0;

	size_t start[3] = { _frameCounter, 0, 0 };
    size_t count[3] = { 1, 3, 0 };
    NCERR(nc_put_vara_double(_ncid, _cell_origin_var, start, count, cell_origin));
    NCERR(nc_put_vara_double(_ncid, _cell_lengths_var, start, count, cell_lengths));
    NCERR(nc_put_vara_double(_ncid, _cell_angles_var, start, count, cell_angles));

	// Write atomic coordinates.
	count[1] = atomsCount;
	count[2] = 3;
#ifdef FLOATTYPE_FLOAT 
	NCERR(nc_put_vara_float(_ncid, _coords_var, start, count, posProperty->constDataFloat()));
#else
	NCERR(nc_put_vara_double(_ncid, _coords_var, start, count, posProperty->constDataFloat()));	
#endif

	// Write out other particle properties.
	exportTask.setProgressMaximum(_columns.size());
	for(const NCOutputColumn& outColumn : _columns) {
		
		// Look up the property to be exported.
		ParticleProperty* prop = outColumn.property.findInState(state);
		if(!prop)
			throwException(tr("The property '%1' cannot be exported, because it does not exist at frame %2.").arg(outColumn.property.name()).arg(frameNumber));
		if((int)prop->componentCount() != outColumn.componentCount)
			throwException(tr("Particle property '%1' cannot be exported, because its number of components has changed at frame %2.").arg(outColumn.property.name()).arg(frameNumber));
		if(prop->dataType() != outColumn.dataType)
			throwException(tr("Particle property '%1' cannot be exported, because its data type has changed at frame %2.").arg(outColumn.property.name()).arg(frameNumber));

		// Write property data to file.
		count[2] = outColumn.componentCount;
		if(outColumn.dataType == PropertyStorage::Int) {
			NCERR(nc_put_vara_int(_ncid, outColumn.ncvar, start, count, prop->constDataInt()));
		}
		else if(outColumn.dataType == PropertyStorage::Int64) {
			NCERR(nc_put_vara_longlong(_ncid, outColumn.ncvar, start, count, prop->constDataInt64()));
		}
		else if(outColumn.dataType == PropertyStorage::Float) {
#ifdef FLOATTYPE_FLOAT 
			NCERR(nc_put_vara_float(_ncid, outColumn.ncvar, start, count, prop->constDataFloat()));
#else
			NCERR(nc_put_vara_double(_ncid, outColumn.ncvar, start, count, prop->constDataFloat()));	
#endif
		}

		if(!exportTask.incrementProgressValue())
			return false;
	}

	_frameCounter++;
	return !exportTask.isCanceled();
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace
