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

#include <gui/GUI.h>
#include <gui/actions/ActionManager.h>
#include <gui/mainwin/MainWindow.h>
#include <gui/dialogs/ApplicationSettingsDialog.h>
#include <gui/dialogs/ImportFileDialog.h>
#include <gui/dialogs/ImportRemoteFileDialog.h>
#include <gui/dialogs/FileExporterSettingsDialog.h>
#include <gui/utilities/concurrent/ProgressDialog.h>
#include <opengl_renderer/OpenGLSceneRenderer.h>
#include <core/app/PluginManager.h>
#include <core/dataset/DataSetContainer.h>
#include <core/dataset/io/FileImporter.h>
#include <core/dataset/io/FileExporter.h>
#include <core/dataset/scene/SelectionSet.h>
#include <core/dataset/animation/AnimationSettings.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui)

/******************************************************************************
* Handles the ACTION_LIBRARY_FILE_IMPORT command.
******************************************************************************/

void ActionManager::on_LibraryFileImport_triggered()
{

    std::string selectedStruct = "";
    
    QDialog dlg(mainWindow());
    dlg.setWindowTitle(tr("Library Structures"));

    QVBoxLayout* list1 = new QVBoxLayout();

    QPushButton* bccButton = new QPushButton();
    bccButton->setIcon(QIcon( ":/gui/actions/file/file_import.png"));
    bccButton->setText(tr("BCC"));
    bccButton->setIconSize(QSize(65,65));
    connect(bccButton, SIGNAL(clicked()), this, SLOT(StructureMenu()));

    QPushButton* fccButton = new QPushButton();
    fccButton->setIcon(QIcon( ":/gui/actions/file/file_import.png"));
    fccButton->setText(tr("FCC"));
    fccButton->setIconSize(QSize(65,65));
    connect(fccButton, SIGNAL(clicked()), this, SLOT(StructureMenu()));

    QPushButton* diamondButton = new QPushButton();
    diamondButton->setIcon(QIcon( ":/gui/actions/file/file_import.png"));
    diamondButton->setText(tr("Diamond"));
    diamondButton->setIconSize(QSize(65,65));

    list1->addWidget(bccButton);
    list1->addWidget(fccButton);
    list1->addWidget(diamondButton);

    QVBoxLayout* list2 = new QVBoxLayout();
    QPushButton* placeHolder = new QPushButton();
    placeHolder->setIcon(QIcon( ":/gui/actions/file/file_import.png"));
    placeHolder->setText(tr("test"));
    placeHolder->setIconSize(QSize(65,65));

    list2->addWidget(placeHolder);
    list2->addWidget(placeHolder);
    list2->addWidget(placeHolder);

    QHBoxLayout *MainLayout = new QHBoxLayout(&dlg);
    MainLayout->addLayout(list1);
    MainLayout->addLayout(list2);

    dlg.exec();
}

void ActionManager::StructureMenu(){

    QDialog dlg(mainWindow());
    dlg.setWindowTitle(tr("Library Structures"));

    QLineEdit *xInput = new QLineEdit;
    QLineEdit *yInput = new QLineEdit;
    QLineEdit *zInput = new QLineEdit;
    QLineEdit *atomNum = new QLineEdit;
	
    atomNum->setEnabled(false);    

    QFormLayout* formLayout = new QFormLayout();
    formLayout->addRow(tr("X :"), xInput);
    formLayout->addRow(tr("Y :"), yInput);
    formLayout->addRow(tr("Z :"), zInput);
    formLayout->addRow(tr("atom size:"), atomNum);

    int xValue = xInput->text().toInt();
    int yValue = xInput->text().toInt(); 
    int zValue = zInput->text().toInt();  

    QHBoxLayout *MainLayout = new QHBoxLayout(&dlg);

    MainLayout->addLayout(formLayout);

    QDialogButtonBox* acceptBox = new QDialogButtonBox(QDialogButtonBox::Ok, &dlg);
    connect(acceptBox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    MainLayout->addWidget(acceptBox);

    dlg.exec();
  
}
