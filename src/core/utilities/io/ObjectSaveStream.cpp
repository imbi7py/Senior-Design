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
#include <core/oo/OvitoObject.h>
#include <core/dataset/DataSet.h>
#include "ObjectSaveStream.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Util) OVITO_BEGIN_INLINE_NAMESPACE(IO)

using namespace std;

/******************************************************************************
* The destructor closes the stream.
******************************************************************************/
ObjectSaveStream::~ObjectSaveStream()
{
	try {
		close();
	}
	catch(Exception& ex) {
		if(!ex.context()) ex.setContext(_dataset);
		ex.reportError();
	}
}

/******************************************************************************
* Saves an object with runtime type information to the stream.
******************************************************************************/
void ObjectSaveStream::saveObject(OvitoObject* object, bool excludeRecomputableData)
{
	if(object == nullptr) {
		*this << (quint32)0;
	}
	else {
		// Instead of saveing the object's data, we only assign a unique instance ID to the object here
		// and write the ID to the stream. The object's contents will get saved when the stream
		// is closed.
		OVITO_CHECK_OBJECT_POINTER(object);
		OVITO_ASSERT(_objects.size() == _objectMap.size());
		quint32& id = _objectMap[object];
		if(id == 0) {
			_objects.emplace_back(object, excludeRecomputableData);
			id = (quint32)_objects.size();

			if(object->getOOClass() == DataSet::OOClass())
				_dataset = static_object_cast<DataSet>(object);

			OVITO_ASSERT(_dataset == nullptr || !object->getOOClass().isDerivedFrom(RefTarget::OOClass()) || static_object_cast<RefTarget>(object)->dataset() == _dataset);
		}
		else if(!excludeRecomputableData) {
			OVITO_ASSERT(_objects[id-1].first == object);
			_objects[id-1].second = false;
		}
		*this << id;
	}
}

/******************************************************************************
* Closes the stream.
******************************************************************************/
void ObjectSaveStream::close()
{
	if(!isOpen())
		return;

	try {
		// Byte offsets of object instances.
		std::vector<qint64> objectOffsets;

		// Serialize the data of each object.
		// Note that additional objects may be appended to the end of the list
		// as we save objects which are already in the list.
		beginChunk(0x100);
		for(size_t i = 0; i < _objects.size(); i++) {
			OvitoObject* obj = _objects[i].first;
			OVITO_CHECK_OBJECT_POINTER(obj);
			objectOffsets.push_back(filePosition());
			obj->saveToStream(*this, _objects[i].second);
		}
		endChunk();
		
		// Save the class of each object instance.
		qint64 classTableStart = filePosition();
		std::map<OvitoClassPtr, quint32> classes;
		beginChunk(0x200);
		for(const auto& obj : _objects) {
			OvitoClassPtr clazz = &obj.first->getOOClass();
			if(classes.find(clazz) == classes.end()) {
				classes.insert(make_pair(clazz, (quint32)classes.size()));
				// Write the basic runtime type information (name and plugin ID) of the class to the stream.
				beginChunk(0x201);
				OvitoClass::serializeRTTI(*this, clazz);
				endChunk();
				// Let the metaclass save additional information like for example the list of property fields defined 
				// for RefMaker-derived classes.
				beginChunk(0x202);
				clazz->saveClassInfo(*this);
				endChunk();
			}
		}
		endChunk();

		// Save object table.
		qint64 objectTableStart = filePosition();
		beginChunk(0x300);
		auto offsetIterator = objectOffsets.cbegin();
		for(const auto& obj : _objects) {
			*this << classes[&obj.first->getOOClass()];
			*this << *offsetIterator++;
		}
		endChunk();

		// Write index of tables.
		*this << classTableStart << (quint32)classes.size();
		*this << objectTableStart << (quint32)_objects.size();
	}
	catch(...) {
		SaveStream::close();
		throw;
	}
	SaveStream::close();
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
