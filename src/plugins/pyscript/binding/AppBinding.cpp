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

#include <plugins/pyscript/PyScript.h>
#include <core/oo/CloneHelper.h>
#include <core/dataset/DataSet.h>
#include <core/dataset/DataSetContainer.h>
#include <core/dataset/scene/SceneNode.h>
#include <core/dataset/scene/ObjectNode.h>
#include <core/dataset/scene/SceneRoot.h>
#include <core/dataset/scene/SelectionSet.h>
#include <core/dataset/animation/AnimationSettings.h>
#include <core/viewport/ViewportConfiguration.h>
#include <core/rendering/RenderSettings.h>
#include <core/utilities/concurrent/TaskManager.h>
#include "PythonBinding.h"

namespace PyScript {

using namespace Ovito;

void defineAppSubmodule(py::module m)
{
	py::class_<OvitoObject, OORef<OvitoObject>>(m, "OvitoObject")
		.def("__str__", [](py::object& pyobj) {
			return py::str("<{} at 0x{:x}>").format(pyobj.attr("__class__").attr("__name__"), (std::intptr_t)pyobj.cast<OvitoObject*>());
		})
		.def("__repr__", [](py::object& pyobj) {
			return py::str("{}()").format(pyobj.attr("__class__").attr("__name__"));
		})
		.def("__eq__", [](OvitoObject* o, py::object& other) {
			try { return o == other.cast<OvitoObject*>(); }
			catch(const py::cast_error&) { return false; }
		})
		.def("__ne__", [](OvitoObject* o, py::object& other) {
			try { return o != other.cast<OvitoObject*>(); }
			catch(const py::cast_error&) { return true; }
		})
	;

	ovito_abstract_class<RefMaker, OvitoObject>{m}
		.def_property_readonly("dataset", 
			[](RefMaker& obj) {
				return obj.dataset().data();
			}, py::return_value_policy::reference)
	;

	ovito_abstract_class<RefTarget, RefMaker>{m}
		// This is used by DataCollection.__getitem__():
		.def_property_readonly("object_title", &RefTarget::objectTitle)
		// This internal method is used in various places:
		.def("notify_object_changed", [](RefTarget& target) { target.notifyDependents(ReferenceEvent::TargetChanged); })
	;

	// Note that, for DataSet, we are not using OORef<> as holder type like we normally do for other OvitoObject-derived classes, because
	// we don't want a ScriptEngine to hold a counted reference to a DataSet that it belongs to.
	// This would create a cyclic reference and potentially lead to a memory leak.
	py::class_<DataSet>(m, "DataSet",
			"This class encompasses all data of an OVITO program session (basically everything that gets saved in a ``.ovito`` state file). "
			"It provides access to the interactive viewports, objects that are part of the three-dimensional scene, the current object selection and animation settings. "
			"\n\n"
			"From a script's point of view, there exists exactly one universal instance of this class, which can be accessed through "
			"the :py:data:`ovito.dataset` module-level attribute. A script cannot create another :py:class:`!DataSet` instance. ")
		.def_property_readonly("scene_root", &DataSet::sceneRoot)
		.def_property_readonly("anim", &DataSet::animationSettings,
				"An :py:class:`~ovito.anim.AnimationSettings` object, which manages various animation-related settings in OVITO such as the number of frames, the current frame, playback speed etc.")
		.def_property_readonly("viewports", &DataSet::viewportConfig,
				"The list of :py:class:`~ovito.vis.Viewport` objects in OVITO's main window.")
		.def_property_readonly("render_settings", &DataSet::renderSettings,
				"The global :py:class:`~ovito.vis.RenderSettings` object, which stores the current settings for rendering pictures and movies. "
				"These are the settings the user can edit in the graphical version of OVITO.")
		.def("save", &DataSet::saveToFile,
			"save(filename)"
			"\n\n"
		    "Saves the dataset including the viewports, all pipeline that are currently part of the scene, and other settings to an OVITO state file. "
    		"This function works like the *Save State As* function in OVITO's file menu."
			"\n\n"
			":param str filename: The path of the file to be written\n",
			py::arg("filename"))
		// This is needed for the DataSet.selected_pipeline attribute:
		.def_property_readonly("selection", &DataSet::selection)
		// This is needed by Viewport.render():
		.def("render_scene", &DataSet::renderScene)
		.def_property_readonly("container", &DataSet::container, py::return_value_policy::reference)
	;

	py::class_<DataSetContainer>{m, "DataSetContainer"}
	;

	py::class_<CloneHelper>(m, "CloneHelper")
		.def(py::init<>())
		.def("clone", static_cast<OORef<RefTarget> (CloneHelper::*)(RefTarget*, bool)>(&CloneHelper::cloneObject<RefTarget>))
	;

	py::class_<TaskManager>(m, "TaskManager")
	;
}

};
