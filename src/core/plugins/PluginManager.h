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

/**
 * \file PluginManager.h
 * \brief Contains the definition of the Ovito::PluginManager class.
 */

#ifndef __OVITO_PLUGIN_MANAGER_H
#define __OVITO_PLUGIN_MANAGER_H

#include <core/Core.h>
#include "Plugin.h"

namespace Ovito {

/**
 * \brief Loads and manages the installed plugins.
 */
class PluginManager : public QObject
{
	Q_OBJECT

public:

	/// \brief Returns the one and only instance of this class.
	/// \return The predefined instance of the PluginManager singleton class.
	inline static PluginManager& instance() {
		if(!_instance) _instance.reset(new PluginManager());
		return *_instance.data();
	}

	/// \brief Returns the plugin with a given identifier.
	/// \param pluginId The identifier of the plugin to return.
	/// \return The plugin with the given identifier or \c NULL if no such plugin is installed.
	Plugin* plugin(const QString& pluginId);

	/// \brief Returns the list of installed plugins.
	/// \return The list of all installed plugins.
	const QVector<Plugin*>& plugins() const { return _plugins; }

	/// \brief Returns the special built-in core plugin.
	/// \return The core plugin. This is not a real plugin but the core of the
	///         application which provides several PluginClass derived classes
	///         as ordinary plugins do.
	/// \sa Plugin::isCore()
	Plugin* corePlugin() { return _corePlugin; }

	/// \brief Returns all installed plugin classes derived from the given type.
	/// \param superClass Specifies the base class from which all returned classes should be derived.
	/// \param skipAbstract If \c true only non-abstract classes are returned.
	/// \return A list that contains all requested classes.
	QVector<OvitoObjectType*> listClasses(const OvitoObjectType* superClass, bool skipAbstract = true);

	/// \brief Registers a new plugin with the manager.
	/// \param plugin The plugin to be registered.
	/// \throw Exception when the plugin ID is not unique.
	/// \note The PluginManager becomes the owner of the Plugin class instance and will
	///       delete it on application shutdown.
	void registerPlugin(Plugin* plugin);

	/// \brief Destructor that unloads all plugins.
	~PluginManager();

private:

	/////////////////////////////////// Plugins ////////////////////////////////////

	/// The list of installed plugins.
	QVector<Plugin*> _plugins;

	/// The built-in core plugin.
	Plugin* _corePlugin;

	/// Searches the plugin directory for installed plugins and loads their XML manifests.
	void registerPlugins();

	/// Loads the given plugin manifest file and creates a Plugin object for it.
	Plugin* loadPluginManifest(const QString& file);

	/////////////////////////// Maintenance ////////////////////////////////

	/// Private constructor.
	/// This is a singleton class; no public instances are allowed.
	PluginManager();

	/// The singleton instance of this class.
	static QScopedPointer<PluginManager> _instance;

	friend class Plugin;
};

};

#endif // __OVITO_PLUGIN_MANAGER_H