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

#ifndef __OVITO_VIEWPORT_SCENE_RENDERER_EDITOR_H
#define __OVITO_VIEWPORT_SCENE_RENDERER_EDITOR_H

#include <core/Core.h>
#include <core/gui/properties/PropertiesEditor.h>
#include <core/reference/RefTarget.h>

namespace Ovito {
	
class ViewportSceneRenderer;		// defined in ViewportSceneRenderer.h

/******************************************************************************
* The editor component for the PreviewRenderer class.
******************************************************************************/
class ViewportSceneRendererEditor : public PropertiesEditor
{
public:

	/// Default constructor.
	Q_INVOKABLE ViewportSceneRendererEditor() {}

protected:
	
	/// Creates the user interface controls for the editor.
	virtual void createUI(const RolloutInsertionParameters& rolloutParams);
	
private:

	Q_OBJECT
	OVITO_OBJECT
};

};

#endif // __OVITO_VIEWPORT_SCENE_RENDERER_EDITOR_H
