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
#include <core/rendering/SceneRenderer.h>
#include <core/gui/properties/ColorParameterUI.h>
#include <core/gui/properties/FloatParameterUI.h>
#include <core/scene/objects/geometry/TriMeshObject.h>
#include "TriMeshDisplay.h"

namespace Ovito {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, TriMeshDisplay, DisplayObject)
IMPLEMENT_OVITO_OBJECT(Core, TriMeshDisplayEditor, PropertiesEditor)
SET_OVITO_OBJECT_EDITOR(TriMeshDisplay, TriMeshDisplayEditor)
DEFINE_FLAGS_PROPERTY_FIELD(TriMeshDisplay, _color, "Color", PROPERTY_FIELD_MEMORIZE)
DEFINE_REFERENCE_FIELD(TriMeshDisplay, _transparency, "Transparency", FloatController)
SET_PROPERTY_FIELD_LABEL(TriMeshDisplay, _color, "Display color")
SET_PROPERTY_FIELD_LABEL(TriMeshDisplay, _transparency, "Transparency")
SET_PROPERTY_FIELD_UNITS(TriMeshDisplay, _transparency, PercentParameterUnit)

/******************************************************************************
* Constructor.
******************************************************************************/
TriMeshDisplay::TriMeshDisplay(DataSet* dataset) : DisplayObject(dataset),
	_color(0.85, 0.85, 1)
{
	INIT_PROPERTY_FIELD(TriMeshDisplay::_color);
	INIT_PROPERTY_FIELD(TriMeshDisplay::_transparency);

	_transparency = ControllerManager::instance().createDefaultController<FloatController>(dataset);
}

/******************************************************************************
* Computes the bounding box of the object.
******************************************************************************/
Box3 TriMeshDisplay::boundingBox(TimePoint time, SceneObject* sceneObject, ObjectNode* contextNode, const PipelineFlowState& flowState)
{
	// Detect if the input data has changed since the last time we computed the bounding box.
	if(_boundingBoxCacheHelper.updateState(sceneObject, sceneObject ? sceneObject->revisionNumber() : 0) || _cachedBoundingBox.isEmpty()) {
		// Recompute bounding box.
		OORef<TriMeshObject> triMeshObj = sceneObject->convertTo<TriMeshObject>(time);
		if(triMeshObj)
			_cachedBoundingBox = triMeshObj->mesh().boundingBox();
		else
			_cachedBoundingBox.setEmpty();
	}
	return _cachedBoundingBox;
}

/******************************************************************************
* Lets the display object render a scene object.
******************************************************************************/
void TriMeshDisplay::render(TimePoint time, SceneObject* sceneObject, const PipelineFlowState& flowState, SceneRenderer* renderer, ObjectNode* contextNode)
{
	// Do we have to re-create the geometry buffer from scratch?
	bool recreateBuffer = !_buffer || !_buffer->isValid(renderer);

	FloatType transp = 0;
	TimeInterval iv;
	if(_transparency) _transparency->getValue(time, transp, iv);
	ColorA color_mesh(color(), 1.0f - transp);

	// Do we have to update contents of the geometry buffer?
	bool updateContents = _geometryCacheHelper.updateState(
			sceneObject, sceneObject ? sceneObject->revisionNumber() : 0,
					color_mesh) || recreateBuffer;

	// Re-create the geometry buffer if necessary.
	if(recreateBuffer)
		_buffer = renderer->createTriMeshGeometryBuffer();

	// Update buffer contents.
	if(updateContents) {
		OORef<TriMeshObject> triMeshObj = sceneObject->convertTo<TriMeshObject>(time);
		if(triMeshObj)
			_buffer->setMesh(triMeshObj->mesh(), color_mesh);
		else
			_buffer->setMesh(TriMesh(), ColorA(1,1,1,1));
	}

	renderer->beginPickObject(contextNode, sceneObject, this);
	_buffer->render(renderer);
	renderer->endPickObject();
}

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void TriMeshDisplayEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create a rollout.
	QWidget* rollout = createRollout(tr("Mesh display"), rolloutParams);

    // Create the rollout contents.
	QGridLayout* layout = new QGridLayout(rollout);
	layout->setContentsMargins(4,4,4,4);
	layout->setSpacing(4);
	layout->setColumnStretch(1, 1);

	ColorParameterUI* colorUI = new ColorParameterUI(this, PROPERTY_FIELD(TriMeshDisplay::_color));
	layout->addWidget(colorUI->label(), 0, 0);
	layout->addWidget(colorUI->colorPicker(), 0, 1);

	FloatParameterUI* transparencyUI = new FloatParameterUI(this, PROPERTY_FIELD(TriMeshDisplay::_transparency));
	layout->addWidget(new QLabel(tr("Transparency (%):")), 1, 0);
	layout->addLayout(transparencyUI->createFieldLayout(), 1, 1);
	transparencyUI->setMinValue(0);
	transparencyUI->setMaxValue(1);
}

};
