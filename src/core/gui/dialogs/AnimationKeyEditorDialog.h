///////////////////////////////////////////////////////////////////////////////
// 
//  Copyright (2015) Alexander Stukowski
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

#ifndef __OVITO_ANIMATION_KEY_EDITOR_DIALOG_H
#define __OVITO_ANIMATION_KEY_EDITOR_DIALOG_H

#include <core/Core.h>
#include <core/dataset/UndoStack.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/**
 * This dialog box allows to edit the animation keys of an animatable parameter.
 */
class AnimationKeyEditorDialog : public QDialog, private UndoableTransaction
{
	Q_OBJECT
	
public:

	/// Constructor.
	AnimationKeyEditorDialog(KeyframeController* ctrl, const PropertyFieldDescriptor* propertyField, QWidget* parentWindow = nullptr);
	
private Q_SLOTS:	

	/// Event handler for the Ok button.
	void onOk();

private:

	QTableView* _tableWidget;
	QAbstractTableModel* _model;
	QAction* _addKeyAction;
	QAction* _deleteKeyAction;

};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_ANIMATION_KEY_EDITOR_DIALOG_H