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

#pragma once


#include <gui/GUI.h>
#include "RefTargetListParameterUI.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Params)

/******************************************************************************
* A list view that shows the delegates of a MultiDelegatingModifier.
******************************************************************************/
class OVITO_GUI_EXPORT ModifierDelegateListParameterUI : public RefTargetListParameterUI
{
	Q_OBJECT
	OVITO_CLASS(ModifierDelegateListParameterUI)
	
public:

	/// Constructor.
	ModifierDelegateListParameterUI(QObject* parentEditor,
			const RolloutInsertionParameters& rolloutParams = RolloutInsertionParameters(), OvitoClassPtr defaultEditorClass = nullptr);
	
	/// This method is called when a new editable object has been activated.
	virtual void resetUI() override {
		RefTargetListParameterUI::resetUI();
		// Clear initial selection by default.
		listWidget()->selectionModel()->clear();
	}	

protected:

	/// Returns a data item from the list data model.
	virtual QVariant getItemData(RefTarget* target, const QModelIndex& index, int role) override;

	/// Returns the model/view item flags for the given entry.
	virtual Qt::ItemFlags getItemFlags(RefTarget* target, const QModelIndex& index) override;
	
	/// Sets the role data for the item at index to value.
	virtual bool setItemData(RefTarget* target, const QModelIndex& index, const QVariant& value, int role) override;
		
	/// Returns the number of columns for the table view.
	virtual int tableColumnCount() override { return 1; }

	/// Returns the header data under the given role for the given RefTarget.
	virtual QVariant getHorizontalHeaderData(int index, int role) override {
		if(role == Qt::DisplayRole) {
			return QVariant::fromValue(tr("Data type"));
		}
		return RefTargetListParameterUI::getHorizontalHeaderData(index, role);
	}

	/// This method is called when a reference target changes.
	virtual bool referenceEvent(RefTarget* source, ReferenceEvent* event) override;
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace


