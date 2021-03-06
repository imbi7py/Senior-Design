///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (2016) Alexander Stukowski
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


#include <plugins/stdmod/gui/StdModGui.h>
#include <gui/properties/ModifierPropertiesEditor.h>
#include <gui/viewport/input/ViewportInputMode.h>
#include <gui/viewport/input/ViewportInputManager.h>
#include <plugins/stdmod/modifiers/SliceModifier.h>

namespace Ovito { namespace StdMod {

class PickPlanePointsInputMode;	// Defined below

/**
 * A properties editor for the SliceModifier class.
 */
class SliceModifierEditor : public ModifierPropertiesEditor
{
	Q_OBJECT
	OVITO_CLASS(SliceModifierEditor)

public:

	/// Default constructor.
	Q_INVOKABLE SliceModifierEditor() {}

protected:

	/// Creates the user interface controls for the editor.
	virtual void createUI(const RolloutInsertionParameters& rolloutParams) override;

protected Q_SLOTS:

	/// Aligns the slicing plane to the viewing direction.
	void onAlignPlaneToView();

	/// Aligns the current viewing direction to the slicing plane.
	void onAlignViewToPlane();

	/// Aligns the normal of the slicing plane with the X, Y, or Z axis.
	void onXYZNormal(const QString& link);

	/// Moves the plane to the center of the simulation box.
	void onCenterOfBox();

private:

	PickPlanePointsInputMode* _pickPlanePointsInputMode;
	ViewportModeAction* _pickPlanePointsInputModeAction;
};

/******************************************************************************
* The viewport input mode that lets the user select three points in space
* to define the slicing plane.
******************************************************************************/
class PickPlanePointsInputMode : public ViewportInputMode
{
public:

	/// Constructor.
	PickPlanePointsInputMode(SliceModifierEditor* editor) : ViewportInputMode(editor), _editor(editor) {}

	/// Handles the mouse events for a Viewport.
	virtual void mouseReleaseEvent(ViewportWindow* vpwin, QMouseEvent* event) override;

	/// Handles mouse move events for a Viewport.
	virtual void mouseMoveEvent(ViewportWindow* vpwin, QMouseEvent* event) override;

	/// Lets the input mode render its overlay content in a viewport.
	virtual void renderOverlay3D(Viewport* vp, ViewportSceneRenderer* renderer) override;

	/// Indicates whether this input mode renders into the viewports.
	virtual bool hasOverlay() override { return true; }

protected:

	/// This is called by the system after the input handler has become the active handler.
	virtual void activated(bool temporary) override;

	/// This is called by the system after the input handler is no longer the active handler.
	virtual void deactivated(bool temporary) override;

private:

	/// Aligns the modifier's slicing plane to the three selected points.
	void alignPlane(SliceModifier* mod);

	/// The list of spatial points picked by the user so far.
	Point3 _pickedPoints[3];

	/// The number of points picked so far.
	int _numPickedPoints = 0;

	/// Indicates whether a preliminary point is available.
	bool _hasPreliminaryPoint = false;
	
	/// The properties editor of the SliceModifier.
	SliceModifierEditor* _editor;
};

}	// End of namespace
}	// End of namespace
