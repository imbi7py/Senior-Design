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

#include <core/Core.h>
#include <core/dataset/DataSet.h>
#include <core/dataset/pipeline/Modifier.h>
#include <core/dataset/pipeline/ModifierApplication.h>
#include <core/dataset/pipeline/PipelineObject.h>
#include <core/dataset/animation/AnimationSettings.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(ObjectSystem) OVITO_BEGIN_INLINE_NAMESPACE(Scene)

IMPLEMENT_OVITO_CLASS(Modifier);
DEFINE_PROPERTY_FIELD(Modifier, isEnabled);
DEFINE_PROPERTY_FIELD(Modifier, title);
SET_PROPERTY_FIELD_LABEL(Modifier, isEnabled, "Enabled");
SET_PROPERTY_FIELD_CHANGE_EVENT(Modifier, isEnabled, ReferenceEvent::TargetEnabledOrDisabled);
SET_PROPERTY_FIELD_LABEL(Modifier, title, "Name");
SET_PROPERTY_FIELD_CHANGE_EVENT(Modifier, title, ReferenceEvent::TitleChanged);

/******************************************************************************
* Constructor.
******************************************************************************/
Modifier::Modifier(DataSet* dataset) : RefTarget(dataset), _isEnabled(true)
{
}

/******************************************************************************
* Create a new modifier application that refers to this modifier instance.
******************************************************************************/
OORef<ModifierApplication> Modifier::createModifierApplication()
{
	OORef<ModifierApplication> modApp = new ModifierApplication(dataset());
	modApp->setModifier(this);
	return modApp;
}

/******************************************************************************
* Returns the list of applications associated with this modifier. 
******************************************************************************/
QVector<ModifierApplication*> Modifier::modifierApplications() const
{
	QVector<ModifierApplication*> apps;
	for(RefMaker* dependent : dependents()) {
        ModifierApplication* modApp = dynamic_object_cast<ModifierApplication>(dependent);
		if(modApp != nullptr && modApp->modifier() == this)
			apps.push_back(modApp);
	}
	return apps;
}

/******************************************************************************
* Returns one of the applications of this modifier in a pipeline.
******************************************************************************/
ModifierApplication* Modifier::someModifierApplication() const
{
	for(RefMaker* dependent : dependents()) {
        ModifierApplication* modApp = dynamic_object_cast<ModifierApplication>(dependent);
		if(modApp != nullptr && modApp->modifier() == this)
			return modApp;
	}
	return nullptr;
}

/******************************************************************************
* Asks the modifier for its validity interval at the given time.
******************************************************************************/
TimeInterval Modifier::modifierValidity(TimePoint time)
{
#if 0	
	// Return an empty validity interval if the modifier is currently being edited
	// to let the system create a pipeline cache point just before the modifier.
	// This will speed up re-evaluation of the pipeline if the user adjusts this modifier's parameters interactively.
	if(isObjectBeingEdited())
		return TimeInterval::empty();
#endif
	return TimeInterval::infinite();
}

/******************************************************************************
* Returns the the current status of the modifier's applications.
******************************************************************************/
PipelineStatus Modifier::globalStatus() const
{
	// Combine the status values of all ModifierApplications into a single status.
	PipelineStatus result;
	for(ModifierApplication* modApp : modifierApplications()) {
		PipelineStatus s = modApp->status();
		
		if(result.text().isEmpty())
			result.setText(s.text());
		else if(s.text() != result.text())
			result.setText(result.text() + QStringLiteral("\n") + s.text());

		if(s.type() == PipelineStatus::Pending)
			result.setType(PipelineStatus::Pending);
		else if(result.type() != PipelineStatus::Pending && s.type() == PipelineStatus::Error)
			result.setType(PipelineStatus::Error);
		else if(result.type() != PipelineStatus::Pending && result.type() != PipelineStatus::Error && s.type() == PipelineStatus::Warning)
			result.setType(PipelineStatus::Warning);
	}
	return result;
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
