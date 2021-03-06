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


#include <gui/GUI.h>
#include <gui/properties/PropertiesEditor.h>
#include <gui/widgets/display/StatusWidget.h>
#include <core/utilities/concurrent/PromiseWatcher.h>
#include <core/dataset/pipeline/ModifierApplication.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Params)

/**
 * \brief Base class for properties editors for ParticleModifier derived classes.
 */
class OVITO_GUI_EXPORT ModifierPropertiesEditor : public PropertiesEditor
{
	Q_OBJECT
	OVITO_CLASS(ModifierPropertiesEditor)
	
public:

	/// Constructor.
	ModifierPropertiesEditor();

	/// Returns a widget that displays status messages of the modifier. 
	/// Editor class implementation can add this widget to their user interface.
	StatusWidget* statusLabel();

	/// Returns the list of ModifierApplications of the modifier currently being edited.
	QVector<ModifierApplication*> modifierApplications();

	/// Returns one of the ModifierApplications of the modifier currently being edited.
	ModifierApplication* someModifierApplication();

	/// Return the input data of the Modifier being edited for one of its ModifierApplications.
	PipelineFlowState getSomeModifierInput();

protected:

	/// This method is called when a reference target changes.
	virtual bool referenceEvent(RefTarget* source, ReferenceEvent* event) override;

private Q_SLOTS:

	/// Updates the text of the result label.
	void updateStatusLabel();

private:

	// UI component for displaying the modifier's status.
	QPointer<StatusWidget> _statusLabel;
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
