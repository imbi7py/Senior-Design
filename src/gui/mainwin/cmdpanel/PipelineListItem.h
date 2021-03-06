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


#include <gui/GUI.h>
#include <core/oo/RefMaker.h>
#include <core/oo/RefTarget.h>
#include <core/dataset/pipeline/ModifierApplication.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/**
 * An item of the ModificationListModel.
 *
 * Holds a reference to an object/modifier.
 */
class PipelineListItem : public RefMaker
{
	Q_OBJECT
	OVITO_CLASS(PipelineListItem)

public:

	/// Constructor.
	PipelineListItem(RefTarget* object, PipelineListItem* parent = nullptr, const QString& title = QString());

	/// Returns true if this is a sub-object entry.
	bool isSubObject() const { return _parent != nullptr; }

	/// Returns the parent entry if this item represents a sub-object.
	PipelineListItem* parent() const { return _parent; }

	/// Returns the status of the object represented by the list item.
	PipelineStatus status() const;

	/// Returns the title text if this is a section header item.
	const QString title() const { return _title; }

Q_SIGNALS:

	/// This signal is emitted when this item has changed.
	void itemChanged(PipelineListItem* item);

	/// This signal is emitted when the list of sub-items of this item has changed.
	void subitemsChanged(PipelineListItem* parent);

protected:

	/// This method is called when a reference target changes.
	virtual bool referenceEvent(RefTarget* source, ReferenceEvent* event) override;

private:

	/// The object represented by this item in the list box.
	DECLARE_REFERENCE_FIELD_FLAGS(RefTarget, object, PROPERTY_FIELD_NO_UNDO | PROPERTY_FIELD_WEAK_REF | PROPERTY_FIELD_NO_CHANGE_MESSAGE);

	/// The list of modifier application if this is a modifier item.
	DECLARE_MODIFIABLE_VECTOR_REFERENCE_FIELD_FLAGS(ModifierApplication, modifierApplications, setModifierApplications, PROPERTY_FIELD_NO_UNDO | PROPERTY_FIELD_WEAK_REF | PROPERTY_FIELD_NO_CHANGE_MESSAGE);

	/// If this is a sub-object entry then this points to the parent.
	PipelineListItem* _parent;

	/// Title text if this is a section header item.
	QString _title;
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace


