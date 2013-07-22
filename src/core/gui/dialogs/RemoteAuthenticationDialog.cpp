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

#include <core/Core.h>
#include "RemoteAuthenticationDialog.h"

namespace Ovito {

/******************************************************************************
* Constructs the dialog window.
******************************************************************************/
RemoteAuthenticationDialog::RemoteAuthenticationDialog(QWidget* parent, const QString& title, const QString& labelText) : QDialog(parent)
{
	setWindowTitle(title);

	QVBoxLayout* layout1 = new QVBoxLayout(this);
	layout1->setSpacing(2);

	QLabel* label = new QLabel(labelText);
	//label->setWordWrap(true);
	layout1->addWidget(label);
	layout1->addSpacing(10);

	layout1->addWidget(new QLabel(tr("Login:")));
	_usernameEdit = new QLineEdit(this);
	layout1->addWidget(_usernameEdit);
	layout1->addSpacing(10);

	layout1->addWidget(new QLabel(tr("Password:")));
	_passwordEdit = new QLineEdit(this);
	_passwordEdit->setEchoMode(QLineEdit::Password);
	layout1->addWidget(_passwordEdit);
	layout1->addSpacing(10);

	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
	connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
	layout1->addWidget(buttonBox);
}

/******************************************************************************
* Displays the dialog.
******************************************************************************/
int RemoteAuthenticationDialog::exec()
{
	if(_usernameEdit->text().isEmpty())
		_usernameEdit->setFocus();
	else
		_passwordEdit->setFocus();

	return QDialog::exec();
}


};
