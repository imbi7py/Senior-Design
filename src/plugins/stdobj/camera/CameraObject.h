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

#pragma once


#include <plugins/stdobj/StdObj.h>
#include <core/dataset/data/camera/AbstractCameraObject.h>
#include <core/dataset/data/VersionedDataObjectRef.h>
#include <core/dataset/animation/controller/Controller.h>
#include <core/dataset/data/DisplayObject.h>
#include <core/rendering/LinePrimitive.h>

namespace Ovito { namespace StdObj {

/**
 * The default camera object.
 */
class OVITO_STDOBJ_EXPORT CameraObject : public AbstractCameraObject
{
	Q_OBJECT
	OVITO_CLASS(CameraObject)
	
public:

	/// Constructor.
	Q_INVOKABLE CameraObject(DataSet* dataset);

	/// \brief Returns the title of this object.
	virtual QString objectTitle() override { return tr("Camera"); }

	/// Returns whether this camera is a target camera directory at a target object.
	bool isTargetCamera() const;

	/// Changes the type of the camera to a target camera or a free camera.
	void setIsTargetCamera(bool enable);

	/// With a target camera, indicates the distance between the camera and its target.
	FloatType targetDistance() const;

	/// \brief Returns a structure describing the camera's projection.
	/// \param[in] time The animation time for which the camera's projection parameters should be determined.
	/// \param[in,out] projParams The structure that is to be filled with the projection parameters.
	///     The following fields of the ViewProjectionParameters structure are already filled in when the method is called:
	///   - ViewProjectionParameters::aspectRatio (The aspect ratio (height/width) of the viewport)
	///   - ViewProjectionParameters::viewMatrix (The world to view space transformation)
	///   - ViewProjectionParameters::boundingBox (The bounding box of the scene in world space coordinates)
	virtual void projectionParameters(TimePoint time, ViewProjectionParameters& projParams) override;

	/// \brief Returns whether this camera uses a perspective projection.
	virtual bool isPerspectiveCamera() const override { return isPerspective(); }
	
	/// \brief Sets whether this camera uses a perspective projection.
	virtual void setPerspectiveCamera(bool perspective) override { setIsPerspective(perspective); }
	
	/// \brief Returns the field of view of the camera.
	virtual FloatType fieldOfView(TimePoint time, TimeInterval& validityInterval) override;

	/// \brief Changes the field of view of the camera.
	virtual void setFieldOfView(TimePoint time, FloatType newFOV) override;

	/// Asks the object for its validity interval at the given time.
	virtual TimeInterval objectValidity(TimePoint time) override;

public:

	Q_PROPERTY(bool isTargetCamera READ isTargetCamera WRITE setIsTargetCamera);
	Q_PROPERTY(bool isPerspective READ isPerspective WRITE setIsPerspective);

private:

	/// Determines if this camera uses a perspective projection.
	DECLARE_MODIFIABLE_PROPERTY_FIELD(bool, isPerspective, setIsPerspective);

	/// This controller stores the field of view of the camera if it uses a perspective projection.
	DECLARE_MODIFIABLE_REFERENCE_FIELD(Controller, fovController, setFovController);

	/// This controller stores the field of view of the camera if it uses an orthogonal projection.
	DECLARE_MODIFIABLE_REFERENCE_FIELD(Controller, zoomController, setZoomController);
};

/**
 * \brief A scene display object for camera objects.
 */
class OVITO_STDOBJ_EXPORT CameraDisplayObject : public DisplayObject
{
	OVITO_CLASS(CameraDisplayObject)
	Q_OBJECT
	
public:

	/// \brief Constructor.
	Q_INVOKABLE CameraDisplayObject(DataSet* dataset) : DisplayObject(dataset) {}

	/// \brief Lets the display object render a camera object.
	virtual void render(TimePoint time, DataObject* dataObject, const PipelineFlowState& flowState, SceneRenderer* renderer, ObjectNode* contextNode) override;

	/// \brief Computes the bounding box of the object.
	virtual Box3 boundingBox(TimePoint time, DataObject* dataObject, ObjectNode* contextNode, const PipelineFlowState& flowState, TimeInterval& validityInterval) override;

	/// \brief Returns the title of this object.
	virtual QString objectTitle() override { return tr("Camera icon"); }

protected:

	/// The buffered geometry used to render the icon.
	std::shared_ptr<LinePrimitive> _cameraIcon;

	/// The icon geometry to be rendered in object picking mode.
	std::shared_ptr<LinePrimitive> _pickingCameraIcon;

	/// The geometry for the camera's viewing cone and target line.
	std::shared_ptr<LinePrimitive> _cameraCone;

	/// This helper structure is used to detect any changes in the input data
	/// that require updating the geometry buffer.
	SceneObjectCacheHelper<
		VersionedDataObjectRef,		// Camera object + revision number
		Color						// Display color
		> _geometryCacheHelper;

	/// This helper structure is used to detect any changes in the input data
	/// that require updating the geometry buffer.
	SceneObjectCacheHelper<
		Color,						// Display color
		FloatType,					// Camera target distance
		bool,						// Target line visible
		FloatType,					// Cone aspect ratio
		FloatType					// Cone angle
		> _coneCacheHelper;
};

}	// End of namespace
}	// End of namespace
