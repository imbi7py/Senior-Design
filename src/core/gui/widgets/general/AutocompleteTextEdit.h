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

#ifndef __OVITO_AUTOCOMPLETE_TEXT_EDIT_H
#define __OVITO_AUTOCOMPLETE_TEXT_EDIT_H

#include <core/Core.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Widgets)

/**
 * \brief A text editor widget that provides auto-completion of words.
 */
class OVITO_CORE_EXPORT AutocompleteTextEdit : public QPlainTextEdit
{
	Q_OBJECT
	
public:
	
	/// \brief Constructs the widget.
	AutocompleteTextEdit(QWidget* parent = nullptr);
	
	/// Sets the list of words that can be completed.
	void setWordList(const QStringList& words) { _wordListModel->setStringList(words); }

	/// Returns the preferred size of the widget.
	virtual QSize sizeHint() const override;

Q_SIGNALS:

	/// This signal is emitted when the Return or Enter key is pressed or the widget loses focus.
	void editingFinished();

protected Q_SLOTS:

	/// Inserts a complete word into the text field.
	void onComplete(const QString& completion);

protected:

	/// Handles key-press events.
	virtual void keyPressEvent(QKeyEvent* event) override;

	/// Handles keyboard focus lost events.
	virtual void focusOutEvent(QFocusEvent* event) override;

	/// Creates a list of tokens from the current text string.
	QStringList getTokenList() const;

protected:

	/// The completer object used by the widget.
	QCompleter* _completer;

	/// The list model storing the words that can be completed.
	QStringListModel* _wordListModel;

	/// Regular expression used to split a text into words.
	QRegularExpression _wordSplitter;
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_AUTOCOMPLETE_TEXT_EDIT_H
