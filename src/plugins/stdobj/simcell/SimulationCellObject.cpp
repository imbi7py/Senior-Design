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

#include <plugins/stdobj/StdObj.h>
#include <core/utilities/units/UnitsManager.h>
#include "SimulationCellObject.h"
#include "SimulationCellDisplay.h"

namespace Ovito { namespace StdObj {
	
IMPLEMENT_OVITO_CLASS(SimulationCellObject);
DEFINE_PROPERTY_FIELD(SimulationCellObject, cellVector1);
DEFINE_PROPERTY_FIELD(SimulationCellObject, cellVector2);
DEFINE_PROPERTY_FIELD(SimulationCellObject, cellVector3);
DEFINE_PROPERTY_FIELD(SimulationCellObject, cellOrigin);
DEFINE_PROPERTY_FIELD(SimulationCellObject, pbcX);
DEFINE_PROPERTY_FIELD(SimulationCellObject, pbcY);
DEFINE_PROPERTY_FIELD(SimulationCellObject, pbcZ);
DEFINE_PROPERTY_FIELD(SimulationCellObject, is2D);
SET_PROPERTY_FIELD_LABEL(SimulationCellObject, cellVector1, "Cell vector 1");
SET_PROPERTY_FIELD_LABEL(SimulationCellObject, cellVector2, "Cell vector 2");
SET_PROPERTY_FIELD_LABEL(SimulationCellObject, cellVector3, "Cell vector 3");
SET_PROPERTY_FIELD_LABEL(SimulationCellObject, cellOrigin, "Cell origin");
SET_PROPERTY_FIELD_LABEL(SimulationCellObject, pbcX, "Periodic boundary conditions (X)");
SET_PROPERTY_FIELD_LABEL(SimulationCellObject, pbcY, "Periodic boundary conditions (Y)");
SET_PROPERTY_FIELD_LABEL(SimulationCellObject, pbcZ, "Periodic boundary conditions (Z)");
SET_PROPERTY_FIELD_LABEL(SimulationCellObject, is2D, "2D");
SET_PROPERTY_FIELD_UNITS(SimulationCellObject, cellVector1, WorldParameterUnit);
SET_PROPERTY_FIELD_UNITS(SimulationCellObject, cellVector2, WorldParameterUnit);
SET_PROPERTY_FIELD_UNITS(SimulationCellObject, cellVector3, WorldParameterUnit);
SET_PROPERTY_FIELD_UNITS(SimulationCellObject, cellOrigin, WorldParameterUnit);

/******************************************************************************
* Creates the storage for the internal parameters.
******************************************************************************/
void SimulationCellObject::init(DataSet* dataset)
{
	// Attach a display object.
	addDisplayObject(new SimulationCellDisplay(dataset));
}

}	// End of namespace
}	// End of namespace
