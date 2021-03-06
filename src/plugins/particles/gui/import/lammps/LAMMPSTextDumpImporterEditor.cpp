///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (2016) Alexander Stukowski
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

#include <plugins/particles/gui/ParticlesGui.h>
#include <plugins/particles/import/lammps/LAMMPSTextDumpImporter.h>
#include <plugins/particles/gui/import/InputColumnMappingDialog.h>
#include <gui/properties/BooleanParameterUI.h>
#include <gui/properties/BooleanRadioButtonParameterUI.h>
#include <gui/mainwin/MainWindow.h>
#include <core/dataset/io/FileSource.h>
#include "LAMMPSTextDumpImporterEditor.h"

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Import) OVITO_BEGIN_INLINE_NAMESPACE(Formats) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

IMPLEMENT_OVITO_CLASS(LAMMPSTextDumpImporterEditor);
SET_OVITO_OBJECT_EDITOR(LAMMPSTextDumpImporter, LAMMPSTextDumpImporterEditor);

/******************************************************************************
 * Displays a dialog box that allows the user to edit the custom file column to particle
 * property mapping.
 *****************************************************************************/
bool LAMMPSTextDumpImporterEditor::showEditColumnMappingDialog(LAMMPSTextDumpImporter* importer, const QUrl& sourceFile, QWidget* parent)
{
	Future<InputColumnMapping> inspectFuture = importer->inspectFileHeader(FileSourceImporter::Frame(sourceFile));
	if(!importer->dataset()->container()->taskManager().waitForTask(inspectFuture))
		return false;
	InputColumnMapping mapping = inspectFuture.result();

	if(!importer->customColumnMapping().empty()) {
		InputColumnMapping customMapping = importer->customColumnMapping();
		customMapping.resize(mapping.size());
		for(size_t i = 0; i < customMapping.size(); i++)
			customMapping[i].columnName = mapping[i].columnName;
		mapping = customMapping;
	}

	InputColumnMappingDialog dialog(mapping, parent);
	if(dialog.exec() == QDialog::Accepted) {
		importer->setCustomColumnMapping(dialog.mapping());
		importer->setUseCustomColumnMapping(true);
		return true;
	}
	else {
		return false;
	}
}

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void LAMMPSTextDumpImporterEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create a rollout.
	QWidget* rollout = createRollout(tr("LAMMPS dump"), rolloutParams);

    // Create the rollout contents.
	QVBoxLayout* layout = new QVBoxLayout(rollout);
	layout->setContentsMargins(4,4,4,4);
	layout->setSpacing(4);

	QGroupBox* animFramesBox = new QGroupBox(tr("Timesteps"), rollout);
	QVBoxLayout* sublayout = new QVBoxLayout(animFramesBox);
	sublayout->setContentsMargins(4,4,4,4);
	layout->addWidget(animFramesBox);

	// Multi-timestep file
	BooleanParameterUI* multitimestepUI = new BooleanParameterUI(this, PROPERTY_FIELD(ParticleImporter::isMultiTimestepFile));
	sublayout->addWidget(multitimestepUI->checkBox());

	QGroupBox* columnMappingBox = new QGroupBox(tr("File columns"), rollout);
	sublayout = new QVBoxLayout(columnMappingBox);
	sublayout->setContentsMargins(4,4,4,4);
	layout->addWidget(columnMappingBox);

	BooleanRadioButtonParameterUI* useCustomMappingUI = new BooleanRadioButtonParameterUI(this, PROPERTY_FIELD(LAMMPSTextDumpImporter::useCustomColumnMapping));
	useCustomMappingUI->buttonFalse()->setText(tr("Automatic mapping"));
	sublayout->addWidget(useCustomMappingUI->buttonFalse());
	useCustomMappingUI->buttonTrue()->setText(tr("User-defined mapping to particle properties"));
	sublayout->addWidget(useCustomMappingUI->buttonTrue());	
	connect(useCustomMappingUI->buttonFalse(), &QRadioButton::clicked, this, [this]() {
		if(LAMMPSTextDumpImporter* importer = static_object_cast<LAMMPSTextDumpImporter>(editObject()))
			importer->requestReload();
	}, Qt::QueuedConnection);

	QPushButton* editMappingButton = new QPushButton(tr("Edit column mapping..."));
	sublayout->addWidget(editMappingButton);
	connect(editMappingButton, &QPushButton::clicked, this, &LAMMPSTextDumpImporterEditor::onEditColumnMapping);
}

/******************************************************************************
* Is called when the user pressed the "Edit column mapping" button.
******************************************************************************/
void LAMMPSTextDumpImporterEditor::onEditColumnMapping()
{
	if(LAMMPSTextDumpImporter* importer = static_object_cast<LAMMPSTextDumpImporter>(editObject())) {

		// Determine URL of current input file.
		FileSource* fileSource = nullptr;
		for(RefMaker* refmaker : importer->dependents()) {
			fileSource = dynamic_object_cast<FileSource>(refmaker);
			if(fileSource) break;
		}
		if(!fileSource || fileSource->frames().empty()) return;

		QUrl sourceUrl;
		if(fileSource->storedFrameIndex() >= 0)
			sourceUrl = fileSource->frames()[fileSource->storedFrameIndex()].sourceFile;
		else
			sourceUrl = fileSource->frames().front().sourceFile;

		UndoableTransaction::handleExceptions(importer->dataset()->undoStack(), tr("Change file column mapping"), [this, &sourceUrl, importer]() {
			if(showEditColumnMappingDialog(importer, sourceUrl, mainWindow())) {
				importer->requestReload();
			}
		});
	}
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace
