###############################################################################
# 
#  Copyright (2013) Alexander Stukowski
#
#  This file is part of OVITO (Open Visualization Tool).
#
#  OVITO is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  OVITO is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
###############################################################################

# Include the resource in the Win32 executable.
IF(WIN32)
	SET(WINDOWS_RESOURCES resources/ovito.rc)
ENDIF(WIN32)

# Put the executable into the right directory.
SET(EXECUTABLE_OUTPUT_PATH ${OVITO_BINARY_DIRECTORY})

# Builds the main executable of the application
ADD_EXECUTABLE(${PROJECT_NAME} WIN32 MACOSX_BUNDLE Main.cpp ${WINDOWS_RESOURCES})

# The executable directly depends on the core modules.
TARGET_LINK_LIBRARIES(${PROJECT_NAME} Base Core)

# Set name of executable.
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "ovito")

# Link Qt5.
QT5_USE_MODULES(${PROJECT_NAME} Widgets Xml)

# This executable will be part of the installation package.
INSTALL(TARGETS ${PROJECT_NAME} DESTINATION "${OVITO_RELATIVE_BINARY_DIRECTORY}")

# If we build a monolithic executable that includes all plugins then
# we have to make it dependent on these too.
#
# When statically linking the plugin libraries the linker will eliminate
# unreferenced static objects in these libraries. To prevent this from
# happening we have to use the -whole-archive linker option. 
IF(OVITO_MONOLITHIC_BUILD)
	LINK_WHOLE_LIBRARY(${PROJECT_NAME} Core)
	LINK_WHOLE_LIBRARY(${PROJECT_NAME} Viz)
ENDIF()

IF(APPLE)
	# Install the Info.plist file.
	CONFIGURE_FILE("resources/Info.plist" "${OVITO_BINARY_DIRECTORY}/${MACOSX_BUNDLE_NAME}.app/Contents/Info.plist")
	SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${OVITO_BINARY_DIRECTORY}/${MACOSX_BUNDLE_NAME}.app/Contents/Info.plist")

	# Copy the application icon into the resource directory.
	INSTALL(FILES "${CMAKE_CURRENT_SOURCE_DIR}/resources/ovito.icns" DESTINATION "${OVITO_RELATIVE_SHARE_DIRECTORY}")

	SET(QT_PLUGINS_DIR "${_qt5Core_install_prefix}/plugins")

	# Install needed Qt plugins by copying directories from the qt installation
	# One can cull what gets copied by using 'REGEX "..." EXCLUDE'
	SET(plugin_dest_dir "${MACOSX_BUNDLE_NAME}.app/Contents/MacOS")
	SET(qtconf_dest_dir "${MACOSX_BUNDLE_NAME}.app/Contents/Resources")
	INSTALL(DIRECTORY "${QT_PLUGINS_DIR}/imageformats" DESTINATION ${plugin_dest_dir}/plugins COMPONENT Runtime)
	INSTALL(DIRECTORY "${QT_PLUGINS_DIR}/platforms" DESTINATION ${plugin_dest_dir}/plugins COMPONENT Runtime)
	INSTALL(DIRECTORY "${QT_PLUGINS_DIR}/accessible" DESTINATION ${plugin_dest_dir}/plugins COMPONENT Runtime)

	# Install a qt.conf file.
	# This inserts some cmake code into the install script to write the file
	INSTALL(CODE "
	    file(WRITE \"\${CMAKE_INSTALL_PREFIX}/${qtconf_dest_dir}/qt.conf\" \"[Paths]\\nPlugins = MacOS/plugins/\")
	    " COMPONENT Runtime)

	# Use BundleUtilities to get all other dependencies for the application to work.
	# It takes a bundle or executable along with possible plugins and inspects it
	# for dependencies.  If they are not system dependencies, they are copied.
	OVITO_FIXUP_BUNDLE()
	
	# To create a package, one can run "cpack -G DragNDrop CPackConfig.cmake" on Mac OS X
	# where CPackConfig.cmake is created by including CPack.
	SET(CPACK_BINARY_DRAGNDROP ON)		
ENDIF()