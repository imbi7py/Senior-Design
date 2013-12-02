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
 * \file OvitoObjectType.h
 * \brief Contains the definition of the Ovito::OvitoObjectType class.
 */

#ifndef __OVITO_OBJECT_TYPE_H
#define __OVITO_OBJECT_TYPE_H

#include <core/Core.h>
#include "OvitoObjectReference.h"

namespace Ovito {

class OvitoObjectType;				// defined below
class OvitoObject;					// defined in OvitoObject.h
class PropertyFieldDescriptor;		// defined in PropertyFieldDescriptor.h
class ObjectSaveStream;				// defined in ObjectSaveStream.h
class ObjectLoadStream;				// defined in ObjectLoadStream.h
class Plugin;						// defined in Plugin.h

/**
 * \brief Stores meta-information about a class in Ovito's object system.
 */
class OVITO_CORE_EXPORT OvitoObjectType
{
public:

	/// \brief Returns the name of the OvitoObject-derived class.
	/// \return The name of the class (without namespace qualifier).
	const QString& name() const { return _name; }

	/// \brief Returns the human-readable display name of this plugin class.
	/// \return The human-readable name of this object type that should be shown in the user interface.
	const QString& displayName() const { return _displayName; }

	/// \brief Changes the the human-readable display name of this plugin class.
	void setDisplayName(const QString& name) { _displayName = name; }

	/// \brief Returns the descriptor of the super class.
	/// \return The descriptor of the base class or \c NULL if this is the descriptor of the root OvitoObject class.
	const OvitoObjectType* superClass() const { return _superClass; }

	/// \brief Returns the plugin that contains the class.
	/// \return The plugin that defines this class.
	Plugin* plugin() const { return _plugin; }

	/// \brief Indicates whether this is an abstract class.
	/// \return \c true if this is an abstract class that cannot be instantiated using createInstance();
	///         \c false otherwise.
	/// \sa createInstance()
	bool isAbstract() const { return _isAbstract; }

	/// \brief Returns whether this class can be written to file.
	/// \return \c true if instances of this class can be serialized to a file;
	///         \c false otherwise.
	/// \note A class is only serializable if all its base classes are also serializable.
	bool isSerializable() const {
		OVITO_ASSERT_MSG(superClass() == nullptr || !(_isSerializable && !superClass()->isSerializable()), "OvitoObjectType::isSerializable", "Class derived from a non-serializable class has to be non-serializable too.");
		return _isSerializable && (superClass() == nullptr || superClass()->isSerializable());
	}

	/// \brief Returns whether this class is directly or indirectly derived from some other class.
	/// \return \c true if this class is a kind of (inherits from) the given class;
	///         \c false otherwise.
	/// \note This method returns \c true if the given class \a other is the class itself.
	bool isDerivedFrom(const OvitoObjectType& other) const {
		for(const OvitoObjectType* c = this; c != NULL; c = c->superClass())
			if(c == &other) return true;
		return false;
	}

	/// \brief Creates an instance of the OvitoObject-derived class.
	/// \return The new instance of the class. The pointer can safely be cast to the appropriate C++ class type.
	/// \throw Exception if a plugin failed to load or the instantiation failed for some other reason.
	OORef<OvitoObject> createInstance() const;

	/// \brief Creates an instance of the OvitoObject-derived class.
        /// This returns a vanilla C++ pointer.
        /// \todo This is not very nice, rather use something like C++11's unique_ptr!
	/// \return The new instance of the class. The pointer can safely be cast to the appropriate C++ class type.
	/// \throw Exception if a plugin failed to load or the instantiation failed for some other reason.
	OvitoObject* createInstancePtr() const;

	/// \brief Returns the first element of the linked list of reference fields defined for this class if it is a RefMaker derived class.
	const PropertyFieldDescriptor* firstPropertyField() const { return _firstPropertyField; }

	/// If this is the descriptor of a RefMaker-derived class then this method will return
	/// the reference field with the given identifier that has been defined in the RefMaker-derived
	/// class. If no such field is defined by that class then NULL is returned.
	/// Note that this method will NOT return reference fields that have been defined in
	/// super-classes.
	const PropertyFieldDescriptor* findPropertyField(const char* identifier) const;

	/// If this is a RefTarget derived classes, this specifies the type of editor to use
	/// when editing objects of this class.
	const OvitoObjectType* editorClass() const { return _editorClass; }

	/// Compares two types.
	bool operator==(const OvitoObjectType& other) const { return (this == &other); }

	/// Compares two types.
	bool operator!=(const OvitoObjectType& other) const { return (this != &other); }

	/// Returns the Qt runtime-type information associated with this object type.
	/// This may be NULL if this is not a native object type.
	virtual const QMetaObject* qtMetaObject() const { return nullptr; }

	/// \brief Writes a type descriptor to the stream.
	/// \note This method is for internal use only.
	static void serializeRTTI(ObjectSaveStream& stream, const OvitoObjectType* type);

	/// \brief Loads a type descriptor from the stream.
	/// \throw Exception if the class is not defined or the required plugin is not installed.
	/// \note This method is for internal use only.
	static OvitoObjectType* deserializeRTTI(ObjectLoadStream& stream);

public:

	/// Internal helper class that is used to assign an editor class to a RefTarget derived class.
	/// Do not use this directly; use the SET_OVITO_OBJECT_EDITOR macro instead.
	struct EditorClassSetter {
		EditorClassSetter(OvitoObjectType& type, const OvitoObjectType* editorClass) {
			OVITO_ASSERT(editorClass != nullptr);
			OVITO_ASSERT(type._editorClass == nullptr);
			OVITO_ASSERT_MSG(type._superClass != nullptr, "SET_OVITO_OBJECT_EDITOR", "Cannot assign editor class to object class that has not been initialized yet. Always use the SET_OVITO_OBJECT_EDITOR macro in the .cpp file of the object class.");
			type._editorClass = editorClass;
		}
	};

protected:

	/// \brief Constructor.
	OvitoObjectType(const QString& name, const OvitoObjectType* superClass, bool isAbstract, bool serializable);

	/// \brief Creates an instance of the class described by this meta object.
	/// \return The new instance of the class. The pointer can safely be cast to the C++ class type.
	/// \throw Exception if the instance could not be created.
	virtual OvitoObject* createInstanceImpl() const = 0;

protected:

	/// The class name.
	QString _name;

	/// The human-readable display name of this plugin class.
	QString _displayName;

	/// The plugin that defined the class.
	Plugin*	_plugin;

	/// The base class descriptor (or NULL if this is the descriptor for the root OvitoObject class).
	const OvitoObjectType* _superClass;

	/// Indicates whether the class is abstract.
	bool _isAbstract;

	/// Indicates whether the objects of the class can be serialized.
	bool _isSerializable;

	/// The linked list of property fields if the class is derived from RefMaker.
	const PropertyFieldDescriptor* _firstPropertyField;

	/// For RefTarget derived classes, this specifies the PropertiesEditor derived
	/// class to use.
	const OvitoObjectType* _editorClass;
};

/// This macro is used to assign a PropertiesEditor-derived class to a RefTarget-derived class.
#define SET_OVITO_OBJECT_EDITOR(RefTargetClass, PropertiesEditorClass)								\
	static Ovito::OvitoObjectType::EditorClassSetter __editorSetter##RefTargetClass(const_cast<Ovito::NativeOvitoObjectType&>(RefTargetClass::OOType), &PropertiesEditorClass::OOType);

};

#endif // __OVITO_OBJECT_TYPE_H
