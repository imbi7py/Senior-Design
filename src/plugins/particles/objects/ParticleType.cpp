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
#include <plugins/particles/objects/ParticleType.h>
#include <core/utilities/units/UnitsManager.h>
#include "ParticleType.h"

namespace Ovito { namespace Particles {

IMPLEMENT_OVITO_CLASS(ParticleType);	
DEFINE_PROPERTY_FIELD(ParticleType, radius);
SET_PROPERTY_FIELD_LABEL(ParticleType, radius, "Radius");
SET_PROPERTY_FIELD_UNITS_AND_MINIMUM(ParticleType, radius, WorldParameterUnit, 0);

/******************************************************************************
* Constructs a new ParticleType.
******************************************************************************/
ParticleType::ParticleType(DataSet* dataset) : ElementType(dataset), _radius(0)
{
}

// Define default names, colors, and radii for some predefined particle types.
std::array<ParticleType::PredefinedTypeInfo, ParticleType::NUMBER_OF_PREDEFINED_PARTICLE_TYPES> ParticleType::_predefinedParticleTypes{{
	ParticleType::PredefinedTypeInfo{ QString("H"), Color(255.0f/255.0f, 255.0f/255.0f, 255.0f/255.0f), 0.46f },
	ParticleType::PredefinedTypeInfo{ QString("He"), Color(217.0f/255.0f, 255.0f/255.0f, 255.0f/255.0f), 1.22f },
	ParticleType::PredefinedTypeInfo{ QString("Li"), Color(204.0f/255.0f, 128.0f/255.0f, 255.0f/255.0f), 1.57f },
	ParticleType::PredefinedTypeInfo{ QString("C"), Color(144.0f/255.0f, 144.0f/255.0f, 144.0f/255.0f), 0.77f },
	ParticleType::PredefinedTypeInfo{ QString("N"), Color(48.0f/255.0f, 80.0f/255.0f, 248.0f/255.0f), 0.74f },
	ParticleType::PredefinedTypeInfo{ QString("O"), Color(255.0f/255.0f, 13.0f/255.0f, 13.0f/255.0f), 0.74f },
	ParticleType::PredefinedTypeInfo{ QString("Na"), Color(171.0f/255.0f, 92.0f/255.0f, 242.0f/255.0f), 1.91f },
	ParticleType::PredefinedTypeInfo{ QString("Mg"), Color(138.0f/255.0f, 255.0f/255.0f, 0.0f/255.0f), 1.60f },
	ParticleType::PredefinedTypeInfo{ QString("Al"), Color(191.0f/255.0f, 166.0f/255.0f, 166.0f/255.0f), 1.43f },
	ParticleType::PredefinedTypeInfo{ QString("Si"), Color(240.0f/255.0f, 200.0f/255.0f, 160.0f/255.0f), 1.18f },
	ParticleType::PredefinedTypeInfo{ QString("K"), Color(143.0f/255.0f, 64.0f/255.0f, 212.0f/255.0f), 2.35f },
	ParticleType::PredefinedTypeInfo{ QString("Ca"), Color(61.0f/255.0f, 255.0f/255.0f, 0.0f/255.0f), 1.97f },
	ParticleType::PredefinedTypeInfo{ QString("Ti"), Color(191.0f/255.0f, 194.0f/255.0f, 199.0f/255.0f), 1.47f },
	ParticleType::PredefinedTypeInfo{ QString("Cr"), Color(138.0f/255.0f, 153.0f/255.0f, 199.0f/255.0f), 1.29f },
	ParticleType::PredefinedTypeInfo{ QString("Fe"), Color(224.0f/255.0f, 102.0f/255.0f, 51.0f/255.0f), 1.26f },
	ParticleType::PredefinedTypeInfo{ QString("Co"), Color(240.0f/255.0f, 144.0f/255.0f, 160.0f/255.0f), 1.25f },
	ParticleType::PredefinedTypeInfo{ QString("Ni"), Color(80.0f/255.0f, 208.0f/255.0f, 80.0f/255.0f), 1.25f },
	ParticleType::PredefinedTypeInfo{ QString("Cu"), Color(200.0f/255.0f, 128.0f/255.0f, 51.0f/255.0f), 1.28f },
	ParticleType::PredefinedTypeInfo{ QString("Zn"), Color(125.0f/255.0f, 128.0f/255.0f, 176.0f/255.0f), 1.37f },
	ParticleType::PredefinedTypeInfo{ QString("Ga"), Color(194.0f/255.0f, 143.0f/255.0f, 143.0f/255.0f), 1.53f },
	ParticleType::PredefinedTypeInfo{ QString("Ge"), Color(102.0f/255.0f, 143.0f/255.0f, 143.0f/255.0f), 1.22f },
	ParticleType::PredefinedTypeInfo{ QString("Kr"), Color(92.0f/255.0f, 184.0f/255.0f, 209.0f/255.0f), 1.98f },
	ParticleType::PredefinedTypeInfo{ QString("Sr"), Color(0.0f, 1.0f, 0.15259f), 2.15f },
	ParticleType::PredefinedTypeInfo{ QString("Y"), Color(0.40259f, 0.59739f, 0.55813f), 1.82f },
	ParticleType::PredefinedTypeInfo{ QString("Zr"), Color(0.0f, 1.0f, 0.0f), 1.60f },
	ParticleType::PredefinedTypeInfo{ QString("Nb"), Color(0.29992f, 0.7f, 0.46459f), 1.47f },
	ParticleType::PredefinedTypeInfo{ QString("Pd"), Color(0.0f/255.0f, 105.0f/255.0f, 133.0f/255.0f), 1.37f },
	ParticleType::PredefinedTypeInfo{ QString("Pt"), Color(0.79997f, 0.77511f, 0.75068f), 1.39f },
	ParticleType::PredefinedTypeInfo{ QString("W"), Color(0.55616f, 0.54257f, 0.50178f), 1.41f },
	ParticleType::PredefinedTypeInfo{ QString("Au"), Color(255.0f/255.0f, 209.0f/255.0f, 35.0f/255.0f), 1.44f }
}};

// Define default names, colors, and radii for predefined structure types.
std::array<ParticleType::PredefinedTypeInfo, ParticleType::NUMBER_OF_PREDEFINED_STRUCTURE_TYPES> ParticleType::_predefinedStructureTypes{{
	ParticleType::PredefinedTypeInfo{ QString("Other"), Color(0.95f, 0.95f, 0.95f), 0 },
	ParticleType::PredefinedTypeInfo{ QString("FCC"), Color(0.4f, 1.0f, 0.4f), 0 },
	ParticleType::PredefinedTypeInfo{ QString("HCP"), Color(1.0f, 0.4f, 0.4f), 0 },
	ParticleType::PredefinedTypeInfo{ QString("BCC"), Color(0.4f, 0.4f, 1.0f), 0 },
	ParticleType::PredefinedTypeInfo{ QString("ICO"), Color(0.95f, 0.8f, 0.2f), 0 },
	ParticleType::PredefinedTypeInfo{ QString("Cubic diamond"), Color(19.0f/255.0f, 160.0f/255.0f, 254.0f/255.0f), 0 },
	ParticleType::PredefinedTypeInfo{ QString("Cubic diamond (1st neighbor)"), Color(0.0f/255.0f, 254.0f/255.0f, 245.0f/255.0f), 0 },
	ParticleType::PredefinedTypeInfo{ QString("Cubic diamond (2nd neighbor)"), Color(126.0f/255.0f, 254.0f/255.0f, 181.0f/255.0f), 0 },
	ParticleType::PredefinedTypeInfo{ QString("Hexagonal diamond"), Color(254.0f/255.0f, 137.0f/255.0f, 0.0f/255.0f), 0 },
	ParticleType::PredefinedTypeInfo{ QString("Hexagonal diamond (1st neighbor)"), Color(254.0f/255.0f, 220.0f/255.0f, 0.0f/255.0f), 0 },
	ParticleType::PredefinedTypeInfo{ QString("Hexagonal diamond (2nd neighbor)"), Color(204.0f/255.0f, 229.0f/255.0f, 81.0f/255.0f), 0 },
	ParticleType::PredefinedTypeInfo{ QString("Simple cubic"), Color(160.0f/255.0f, 20.0f/255.0f, 254.0f/255.0f), 0 }
}};

/******************************************************************************
* Returns the default color for a particle type ID.
******************************************************************************/
Color ParticleType::getDefaultParticleColorFromId(ParticleProperty::Type typeClass, int particleTypeId)
{
	// Assign initial standard color to new particle types.
	static const Color defaultTypeColors[] = {
		Color(0.4f,1.0f,0.4f),
		Color(1.0f,0.4f,0.4f),
		Color(0.4f,0.4f,1.0f),
		Color(1.0f,1.0f,0.7f),
		Color(0.97f,0.97f,0.97f),
		Color(1.0f,1.0f,0.0f),
		Color(1.0f,0.4f,1.0f),
		Color(0.7f,0.0f,1.0f),
		Color(0.2f,1.0f,1.0f),
	};
	return defaultTypeColors[std::abs(particleTypeId) % (sizeof(defaultTypeColors) / sizeof(defaultTypeColors[0]))];
}

/******************************************************************************
* Returns the default color for a particle type name.
******************************************************************************/
Color ParticleType::getDefaultParticleColor(ParticleProperty::Type typeClass, const QString& particleTypeName, int particleTypeId, bool userDefaults)
{
	if(userDefaults) {
		QSettings settings;
		settings.beginGroup("particles/defaults/color");
		settings.beginGroup(QString::number((int)typeClass));
		QVariant v = settings.value(particleTypeName);
		if(v.isValid() && v.canConvert<Color>())
			return v.value<Color>();
	}

	if(typeClass == ParticleProperty::StructureTypeProperty) {
		for(const PredefinedTypeInfo& predefType : _predefinedStructureTypes) {
			if(std::get<0>(predefType) == particleTypeName)
				return std::get<1>(predefType);
		}
		return Color(1,1,1);
	}
	else if(typeClass == ParticleProperty::TypeProperty) {
		for(const PredefinedTypeInfo& predefType : _predefinedParticleTypes) {
			if(std::get<0>(predefType) == particleTypeName)
				return std::get<1>(predefType);
		}

		// Sometime atom type names have additional letters/numbers appended.
		if(particleTypeName.length() > 1 && particleTypeName.length() <= 3) {
			return getDefaultParticleColor(typeClass, particleTypeName.left(particleTypeName.length() - 1), particleTypeId, userDefaults);
		}
	}
	return getDefaultParticleColorFromId(typeClass, particleTypeId);
}

/******************************************************************************
* Changes the default color for a particle type name.
******************************************************************************/
void ParticleType::setDefaultParticleColor(ParticleProperty::Type typeClass, const QString& particleTypeName, const Color& color)
{
	QSettings settings;
	settings.beginGroup("particles/defaults/color");
	settings.beginGroup(QString::number((int)typeClass));

	if(getDefaultParticleColor(typeClass, particleTypeName, 0, false) != color)
		settings.setValue(particleTypeName, QVariant::fromValue(color));
	else
		settings.remove(particleTypeName);
}

/******************************************************************************
* Returns the default radius for a particle type name.
******************************************************************************/
FloatType ParticleType::getDefaultParticleRadius(ParticleProperty::Type typeClass, const QString& particleTypeName, int particleTypeId, bool userDefaults)
{
	if(userDefaults) {
		QSettings settings;
		settings.beginGroup("particles/defaults/radius");
		settings.beginGroup(QString::number((int)typeClass));
		QVariant v = settings.value(particleTypeName);
		if(v.isValid() && v.canConvert<FloatType>())
			return v.value<FloatType>();
	}

	if(typeClass == ParticleProperty::TypeProperty) {
		for(const PredefinedTypeInfo& predefType : _predefinedParticleTypes) {
			if(std::get<0>(predefType) == particleTypeName)
				return std::get<2>(predefType);
		}

		// Sometime atom type names have additional letters/numbers appended.
		if(particleTypeName.length() > 1 && particleTypeName.length() <= 3) {
			return getDefaultParticleRadius(typeClass, particleTypeName.left(particleTypeName.length() - 1), particleTypeId, userDefaults);
		}
	}

	return 0;
}

/******************************************************************************
* Changes the default radius for a particle type name.
******************************************************************************/
void ParticleType::setDefaultParticleRadius(ParticleProperty::Type typeClass, const QString& particleTypeName, FloatType radius)
{
	QSettings settings;
	settings.beginGroup("particles/defaults/radius");
	settings.beginGroup(QString::number((int)typeClass));

	if(getDefaultParticleRadius(typeClass, particleTypeName, 0, false) != radius)
		settings.setValue(particleTypeName, QVariant::fromValue(radius));
	else
		settings.remove(particleTypeName);
}

}	// End of namespace
}	// End of namespace
