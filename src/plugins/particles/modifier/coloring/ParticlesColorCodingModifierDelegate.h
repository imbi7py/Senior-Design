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
#include <plugins/particles/objects/ParticleProperty.h>
#include <plugins/particles/objects/BondProperty.h>
#include <plugins/particles/objects/BondsObject.h>
#include <plugins/stdmod/modifiers/ColorCodingModifier.h>

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Modifiers) OVITO_BEGIN_INLINE_NAMESPACE(Modify)

/**
 * \brief Function for the ColorCodingModifier that operates on particles.
 */
class ParticlesColorCodingModifierDelegate : public ColorCodingModifierDelegate
{
	/// Give the modifier delegate its own metaclass.
	class OOMetaClass : public ColorCodingModifierDelegate::OOMetaClass 
	{
	public:

		/// Inherit constructor from base class.
		using ColorCodingModifierDelegate::OOMetaClass::OOMetaClass;

		/// Asks the metaclass whether the modifier delegate can operate on the given input data.
		virtual bool isApplicableTo(const PipelineFlowState& input) const override {
			return input.findObject<ParticleProperty>() != nullptr;
		}

		/// The name by which Python scripts can refer to this modifier delegate.
		virtual QString pythonDataName() const override { return QStringLiteral("particles"); }
	};	

	Q_OBJECT
	OVITO_CLASS_META(ParticlesColorCodingModifierDelegate, OOMetaClass)

	Q_CLASSINFO("DisplayName", "Particles");

public:

	/// Constructor.
	Q_INVOKABLE ParticlesColorCodingModifierDelegate(DataSet* dataset) : ColorCodingModifierDelegate(dataset) {}

	/// \brief Returns the class of properties that can serve as input for the color coding.
	virtual const PropertyClass& propertyClass() const override { return ParticleProperty::OOClass(); }

protected:
	
	/// \brief Creates the property object that will receive the computed colors.
	virtual PropertyObject* createOutputColorProperty(TimePoint time, InputHelper& ih, OutputHelper& oh, bool initializeWithExistingColors) override;	
};

/**
 * \brief Function for the ColorCodingModifier that operates on particle vectors.
 */
class ParticleVectorsColorCodingModifierDelegate : public ColorCodingModifierDelegate
{
	/// Give the modifier delegate its own metaclass.
	class OOMetaClass : public ColorCodingModifierDelegate::OOMetaClass 
	{
	public:

		/// Inherit constructor from base class.
		using ColorCodingModifierDelegate::OOMetaClass::OOMetaClass;

		/// Asks the metaclass whether the modifier delegate can operate on the given input data.
		virtual bool isApplicableTo(const PipelineFlowState& input) const override;

		/// The name by which Python scripts can refer to this modifier delegate.
		virtual QString pythonDataName() const override { return QStringLiteral("vectors"); }
	};	

	Q_OBJECT
	OVITO_CLASS_META(ParticleVectorsColorCodingModifierDelegate, OOMetaClass)

	Q_CLASSINFO("DisplayName", "Particle vectors");

public:

	/// Constructor.
	Q_INVOKABLE ParticleVectorsColorCodingModifierDelegate(DataSet* dataset) : ColorCodingModifierDelegate(dataset) {}

	/// \brief Returns the class of properties that can serve as input for the color coding.
	virtual const PropertyClass& propertyClass() const override { return ParticleProperty::OOClass(); }

protected:
	
	/// \brief Creates the property object that will receive the computed colors.
	virtual PropertyObject* createOutputColorProperty(TimePoint time, InputHelper& ih, OutputHelper& oh, bool initializeWithExistingColors) override;	
};

/**
 * \brief Function for the ColorCodingModifier that operates on bonds.
 */
class BondsColorCodingModifierDelegate : public ColorCodingModifierDelegate
{
	/// Give the modifier delegate its own metaclass.
	class OOMetaClass : public ColorCodingModifierDelegate::OOMetaClass 
	{
	public:

		/// Inherit constructor from base class.
		using ColorCodingModifierDelegate::OOMetaClass::OOMetaClass;

		/// Asks the metaclass whether the modifier delegate can operate on the given input data.
		virtual bool isApplicableTo(const PipelineFlowState& input) const override {
			return input.findObject<BondsObject>() != nullptr;
		}

		/// The name by which Python scripts can refer to this modifier delegate.
		virtual QString pythonDataName() const override { return QStringLiteral("bonds"); }
	};	

	Q_OBJECT
	OVITO_CLASS_META(BondsColorCodingModifierDelegate, OOMetaClass)

	Q_CLASSINFO("DisplayName", "Bonds");

public:

	/// Constructor.
	Q_INVOKABLE BondsColorCodingModifierDelegate(DataSet* dataset) : ColorCodingModifierDelegate(dataset) {}

	/// \brief Returns the class of properties that can serve as input for the color coding.
	virtual const PropertyClass& propertyClass() const override { return BondProperty::OOClass(); }

protected:
	
	/// \brief Creates the property object that will receive the computed colors.
	virtual PropertyObject* createOutputColorProperty(TimePoint time, InputHelper& ih, OutputHelper& oh, bool initializeWithExistingColors) override;	
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace
