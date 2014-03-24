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

#include <core/Core.h>
#include <core/gui/properties/NumericalParameterUI.h>
#include <core/dataset/UndoStack.h>
#include <core/viewport/ViewportConfiguration.h>
#include <core/animation/AnimationSettings.h>
#include <core/utilities/units/UnitsManager.h>

namespace Ovito {

// Gives the class run-time type information.
IMPLEMENT_OVITO_OBJECT(Core, NumericalParameterUI, PropertyParameterUI);

/******************************************************************************
* Constructor for a Qt property.
******************************************************************************/
NumericalParameterUI::NumericalParameterUI(QObject* parentEditor, const char* propertyName, const QMetaObject* defaultParameterUnitType, const QString& labelText) :
	PropertyParameterUI(parentEditor, propertyName), _parameterUnitType(defaultParameterUnitType)
{
	initUIControls(labelText);	
}

/******************************************************************************
* Constructor for a PropertyField or ReferenceField property.
******************************************************************************/
NumericalParameterUI::NumericalParameterUI(QObject* parentEditor, const PropertyFieldDescriptor& propField, const QMetaObject* defaultParameterUnitType) :
	PropertyParameterUI(parentEditor, propField), _parameterUnitType(defaultParameterUnitType)
{
	// Look up the ParameterUnit type for this parameter.
	if(propField.parameterUnitType())
		_parameterUnitType = propField.parameterUnitType();
	
	initUIControls(propField.displayName() + ":");
}

/******************************************************************************
* Creates the widgets for this property UI.
******************************************************************************/
void NumericalParameterUI::initUIControls(const QString& labelText)
{
	// Create UI widgets.
	_label = new QLabel(labelText);
	_textBox = new QLineEdit();
	_spinner = new SpinnerWidget();
	connect(_spinner, SIGNAL(spinnerValueChanged()), this, SLOT(onSpinnerValueChanged()));	
	connect(_spinner, SIGNAL(spinnerDragStart()), this, SLOT(onSpinnerDragStart()));
	connect(_spinner, SIGNAL(spinnerDragStop()), this, SLOT(onSpinnerDragStop()));
	connect(_spinner, SIGNAL(spinnerDragAbort()), this, SLOT(onSpinnerDragAbort()));
	_spinner->setTextBox(_textBox);
}

/******************************************************************************
* Destructor, that releases all GUI controls.
******************************************************************************/
NumericalParameterUI::~NumericalParameterUI()
{
	// Release GUI controls. 
	delete label();
	delete spinner();
	delete textBox(); 
}

/******************************************************************************
* This method is called when a new editable object has been assigned to the properties owner this
* parameter UI belongs to. 
******************************************************************************/
void NumericalParameterUI::resetUI()
{
	if(spinner()) {
		spinner()->setEnabled(editObject() != NULL && isEnabled());
		if(editObject() != NULL) {
			ParameterUnit* unit;
			if(parameterUnitType())
				unit = dataset()->unitsManager().getUnit(parameterUnitType());
			else
				unit = nullptr;
			spinner()->setUnit(unit);
		}
		else {
			spinner()->setUnit(nullptr);
			spinner()->setFloatValue(0);
		}
	}

	if(isReferenceFieldUI()) {
		// Update the displayed value when the animation time has changed.
		disconnect(_animationTimeChangedConnection);
		if(editObject())
			_animationTimeChangedConnection = connect(dataset()->animationSettings(), &AnimationSettings::timeChanged, this, &NumericalParameterUI::updateUI);
	}

	PropertyParameterUI::resetUI();
}

/******************************************************************************
* Sets the enabled state of the UI.
******************************************************************************/
void NumericalParameterUI::setEnabled(bool enabled)
{
	if(enabled == isEnabled()) return;
	PropertyParameterUI::setEnabled(enabled);
	if(spinner()) {
		if(isReferenceFieldUI()) {
			spinner()->setEnabled(parameterObject() != NULL && isEnabled());
		}
		else {
			spinner()->setEnabled(editObject() != NULL && isEnabled());
		}
	}
}

/******************************************************************************
* Is called when the spinner value has changed.
******************************************************************************/
void NumericalParameterUI::onSpinnerValueChanged()
{
	ViewportSuspender noVPUpdate(dataset()->viewportConfig());
	if(!dataset()->undoStack().isRecording()) {
		UndoableTransaction transaction(dataset()->undoStack(), tr("Change parameter"));
		updatePropertyValue();
		transaction.commit();
	}
	else {
		dataset()->undoStack().resetCurrentCompoundOperation();
		updatePropertyValue();
	}
}

/******************************************************************************
* Is called when the user begins dragging the spinner interactively.
******************************************************************************/
void NumericalParameterUI::onSpinnerDragStart()
{
	dataset()->undoStack().beginCompoundOperation(tr("Change parameter"));
}

/******************************************************************************
* Is called when the user stops dragging the spinner interactively.
******************************************************************************/
void NumericalParameterUI::onSpinnerDragStop()
{
	dataset()->undoStack().endCompoundOperation();
}

/******************************************************************************
* Is called when the user aborts dragging the spinner interactively.
******************************************************************************/
void NumericalParameterUI::onSpinnerDragAbort()
{
	dataset()->undoStack().endCompoundOperation(false);
}

/******************************************************************************
* Creates a QLayout that contains the text box and the spinner widget.
******************************************************************************/
QLayout* NumericalParameterUI::createFieldLayout() const
{
	QHBoxLayout* layout = new QHBoxLayout();
	layout->setContentsMargins(0,0,0,0);
	layout->setSpacing(0);
	layout->addWidget(textBox());
	layout->addWidget(spinner());
	return layout;
}


};

