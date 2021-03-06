///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (2014) Alexander Stukowski
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


#include <core/Core.h>
#include "ViewportOverlay.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(View) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/**
 * \brief A viewport overlay that displays the coordinate system orientation.
 */
class OVITO_CORE_EXPORT CoordinateTripodOverlay : public ViewportOverlay
{
	Q_OBJECT
	OVITO_CLASS(CoordinateTripodOverlay)
	Q_CLASSINFO("DisplayName", "Coordinate tripod");
	
public:

	/// \brief Constructor.
	Q_INVOKABLE CoordinateTripodOverlay(DataSet* dataset);

	/// \brief This method asks the overlay to paint its contents over the given viewport.
	virtual void render(Viewport* viewport, TimePoint time, QPainter& painter, 
						const ViewProjectionParameters& projParams, RenderSettings* renderSettings,
						bool interactiveViewport, TaskManager& taskManager) override;

	/// Moves the position of the overlay in the viewport by the given amount,
	/// which is specified as a fraction of the viewport render size.
	virtual void moveOverlayInViewport(const Vector2& delta) override {
		setOffsetX(offsetX() + delta.x());
		setOffsetY(offsetY() + delta.y());
	}

private:

	/// The corner of the viewport where the tripod is shown in.
	DECLARE_MODIFIABLE_PROPERTY_FIELD_FLAGS(int, alignment, setAlignment, PROPERTY_FIELD_MEMORIZE);

	/// Controls the size of the tripod.
	DECLARE_MODIFIABLE_PROPERTY_FIELD_FLAGS(FloatType, tripodSize, setTripodSize, PROPERTY_FIELD_MEMORIZE);

	/// Controls the line width.
	DECLARE_MODIFIABLE_PROPERTY_FIELD_FLAGS(FloatType, lineWidth, setLineWidth, PROPERTY_FIELD_MEMORIZE);

	/// Controls the horizontal offset of tripod position.
	DECLARE_MODIFIABLE_PROPERTY_FIELD_FLAGS(FloatType, offsetX, setOffsetX, PROPERTY_FIELD_MEMORIZE);

	/// Controls the vertical offset of tripod position.
	DECLARE_MODIFIABLE_PROPERTY_FIELD_FLAGS(FloatType, offsetY, setOffsetY, PROPERTY_FIELD_MEMORIZE);

	/// Controls the label font.
	DECLARE_MODIFIABLE_PROPERTY_FIELD_FLAGS(QFont, font, setFont, PROPERTY_FIELD_MEMORIZE);

	/// Controls the label font size.
	DECLARE_MODIFIABLE_PROPERTY_FIELD_FLAGS(FloatType, fontSize, setFontSize, PROPERTY_FIELD_MEMORIZE);

	/// Controls the display of the first axis.
	DECLARE_MODIFIABLE_PROPERTY_FIELD(bool, axis1Enabled, setAxis1Enabled);

	/// Controls the display of the second axis.
	DECLARE_MODIFIABLE_PROPERTY_FIELD(bool, axis2Enabled, setAxis2Enabled);

	/// Controls the display of the third axis.
	DECLARE_MODIFIABLE_PROPERTY_FIELD(bool, axis3Enabled, setAxis3Enabled);

	/// Controls the display of the fourth axis.
	DECLARE_MODIFIABLE_PROPERTY_FIELD(bool, axis4Enabled, setAxis4Enabled);

	/// The label of the first axis.
	DECLARE_MODIFIABLE_PROPERTY_FIELD(QString, axis1Label, setAxis1Label);

	/// The label of the second axis.
	DECLARE_MODIFIABLE_PROPERTY_FIELD(QString, axis2Label, setAxis2Label);

	/// The label of the third axis.
	DECLARE_MODIFIABLE_PROPERTY_FIELD(QString, axis3Label, setAxis3Label);

	/// The label of the fourth axis.
	DECLARE_MODIFIABLE_PROPERTY_FIELD(QString, axis4Label, setAxis4Label);

	/// The direction of the first axis.
	DECLARE_MODIFIABLE_PROPERTY_FIELD(Vector3, axis1Dir, setAxis1Dir);

	/// The direction of the second axis.
	DECLARE_MODIFIABLE_PROPERTY_FIELD(Vector3, axis2Dir, setAxis2Dir);

	/// The direction of the third axis.
	DECLARE_MODIFIABLE_PROPERTY_FIELD(Vector3, axis3Dir, setAxis3Dir);

	/// The direction of the fourth axis.
	DECLARE_MODIFIABLE_PROPERTY_FIELD(Vector3, axis4Dir, setAxis4Dir);

	/// The display color of the first axis.
	DECLARE_MODIFIABLE_PROPERTY_FIELD_FLAGS(Color, axis1Color, setAxis1Color, PROPERTY_FIELD_MEMORIZE);

	/// The display color of the second axis.
	DECLARE_MODIFIABLE_PROPERTY_FIELD_FLAGS(Color, axis2Color, setAxis2Color, PROPERTY_FIELD_MEMORIZE);

	/// The display color of the third axis.
	DECLARE_MODIFIABLE_PROPERTY_FIELD_FLAGS(Color, axis3Color, setAxis3Color, PROPERTY_FIELD_MEMORIZE);

	/// The display color of the fourth axis.
	DECLARE_MODIFIABLE_PROPERTY_FIELD_FLAGS(Color, axis4Color, setAxis4Color, PROPERTY_FIELD_MEMORIZE);
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace


