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

	/// data used to generate sturct
	bool confirmed;
	bool cutSphere;
	int xValue;
	int yValue;
	int zValue;
	float radius;
	float numOfAtoms;
	QLineEdit *sphereRadius;

/******************************************************************************
* Handles the ACTION_QUIT command.
******************************************************************************/
void ActionManager::on_Quit_triggered()
{
	mainWindow()->close();
}

/******************************************************************************
* Handles the ACTION_HELP_ABOUT command.
******************************************************************************/
void ActionManager::on_HelpAbout_triggered()
{
	QMessageBox msgBox(QMessageBox::NoIcon, QCoreApplication::applicationName(),
			tr("<h3>Ovito (Open Visualization Tool)</h3>"
				"<p>Version %1</p>").arg(QCoreApplication::applicationVersion()),
			QMessageBox::Ok, mainWindow());
	msgBox.setInformativeText(tr(
			"<p>A visualization and analysis software for atomistic simulation data.</p>"
			"<p>Copyright (C) 2017, Alexander Stukowski</p>"
			"<p>"
			"This is free, open-source software, and you are welcome to redistribute\n"
			"it under certain conditions. See the source for copying conditions.</p>"
			"<p><a href=\"http://www.ovito.org/\">http://www.ovito.org/</a></p>"));
	msgBox.setDefaultButton(QMessageBox::Ok);
	QPixmap icon = QApplication::windowIcon().pixmap(64 * mainWindow()->devicePixelRatio());
	icon.setDevicePixelRatio(mainWindow()->devicePixelRatio());
	msgBox.setIconPixmap(icon);
	msgBox.exec();
}

/******************************************************************************
* Handles the ACTION_HELP_SHOW_ONLINE_HELP command.
******************************************************************************/
void ActionManager::on_HelpShowOnlineHelp_triggered()
{
	mainWindow()->openHelpTopic(QString());
}

/******************************************************************************
* Handles the ACTION_HELP_OPENGL_INFO command.
******************************************************************************/
void ActionManager::on_HelpOpenGLInfo_triggered()
{
	QDialog dlg(mainWindow());
	dlg.setWindowTitle(tr("OpenGL Information"));
	QVBoxLayout* layout = new QVBoxLayout(&dlg);
	QTextEdit* textEdit = new QTextEdit(&dlg);
	textEdit->setReadOnly(true);
	QString text;
	QTextStream stream(&text, QIODevice::WriteOnly | QIODevice::Text);
	stream << "======= System info =======" << endl;
	stream << "Date: " << QDateTime::currentDateTime().toString() << endl;
	stream << "Application: " << QApplication::applicationName() << " " << QApplication::applicationVersion() << endl;
#if defined(Q_OS_MAC)
	stream << "OS: Mac OS X (" << QSysInfo::macVersion() << ")" << endl;
#elif defined(Q_OS_WIN)
	stream << "OS: Windows (" << QSysInfo::windowsVersion() << ")" << endl;
#elif defined(Q_OS_LINUX)
	stream << "OS: Linux" << endl;
	// Get 'uname' output.
	QProcess unameProcess;
	unameProcess.start("uname -m -i -o -r -v", QIODevice::ReadOnly);
	unameProcess.waitForFinished();
	QByteArray unameOutput = unameProcess.readAllStandardOutput();
	unameOutput.replace('\n', ' ');
	stream << "uname output: " << unameOutput << endl;
	// Get 'lsb_release' output.
	QProcess lsbProcess;
	lsbProcess.start("lsb_release -s -i -d -r", QIODevice::ReadOnly);
	lsbProcess.waitForFinished();
	QByteArray lsbOutput = lsbProcess.readAllStandardOutput();
	lsbOutput.replace('\n', ' ');
	stream << "LSB output: " << lsbOutput << endl;
#endif
	stream << "Architecture: " << (QT_POINTER_SIZE*8) << " bit" << endl;
	stream << "Floating-point size: " << (sizeof(FloatType)*8) << " bit" << endl;
	stream << "Qt version: " << QT_VERSION_STR << endl;
	stream << "Command line: " << QCoreApplication::arguments().join(' ') << endl;
	stream << "======= OpenGL info =======" << endl;
	const QSurfaceFormat& format = OpenGLSceneRenderer::openglSurfaceFormat();
	stream << "Version: " << format.majorVersion() << QStringLiteral(".") << format.minorVersion() << endl;
	stream << "Profile: " << (format.profile() == QSurfaceFormat::CoreProfile ? "core" : (format.profile() == QSurfaceFormat::CompatibilityProfile ? "compatibility" : "none")) << endl;
	stream << "Alpha: " << format.hasAlpha() << endl;
	stream << "Vendor: " << OpenGLSceneRenderer::openGLVendor() << endl;
	stream << "Renderer: " << OpenGLSceneRenderer::openGLRenderer() << endl;
	stream << "Version string: " << OpenGLSceneRenderer::openGLVersion() << endl;
	stream << "Swap behavior: " << (format.swapBehavior() == QSurfaceFormat::SingleBuffer ? QStringLiteral("single buffer") : (format.swapBehavior() == QSurfaceFormat::DoubleBuffer ? QStringLiteral("double buffer") : (format.swapBehavior() == QSurfaceFormat::TripleBuffer ? QStringLiteral("triple buffer") : QStringLiteral("other")))) << endl;
	stream << "Depth buffer size: " << format.depthBufferSize() << endl;
	stream << "Stencil buffer size: " << format.stencilBufferSize() << endl;
	stream << "Shading language: " << OpenGLSceneRenderer::openGLSLVersion() << endl;
	stream << "Geometry shaders supported: " << (OpenGLSceneRenderer::geometryShadersSupported() ? "yes" : "no") << endl;
	stream << "Using deprecated functions: " << (format.testOption(QSurfaceFormat::DeprecatedFunctions) ? "yes" : "no") << endl;
	stream << "Using point sprites: " << (OpenGLSceneRenderer::pointSpritesEnabled() ? "yes" : "no") << endl;
	stream << "Using geometry shaders: " << (OpenGLSceneRenderer::geometryShadersEnabled() ? "yes" : "no") << endl;
	stream << "Context sharing enabled: " << (OpenGLSceneRenderer::contextSharingEnabled() ? "yes" : "no") << endl;
	if(!text.isEmpty())
		textEdit->setPlainText(text);
	else
		textEdit->setPlainText(tr("Could not obtain OpenGL information."));
	textEdit->setMinimumSize(QSize(600, 400));
	layout->addWidget(textEdit);
	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, Qt::Horizontal, &dlg);
	connect(buttonBox, &QDialogButtonBox::rejected, &dlg, &QDialog::accept);
	connect(buttonBox->addButton(tr("Copy to clipboard"), QDialogButtonBox::ActionRole), &QPushButton::clicked, [text]() { QApplication::clipboard()->setText(text); });
	layout->addWidget(buttonBox);
	dlg.exec();
}



/******************************************************************************
* Handles the ACTION_FILE_NEW_WINDOW command.
******************************************************************************/
void ActionManager::on_FileNewWindow_triggered()
{
	try {
		MainWindow* mainWin = new MainWindow();
		mainWin->show();
		mainWin->restoreLayout();
		mainWin->datasetContainer().fileNew();
	}
	catch(const Exception& ex) {
		ex.reportError();
	}
}

/******************************************************************************
* Handles the ACTION_FILE_NEW command.
******************************************************************************/
void ActionManager::on_FileNew_triggered()
{
	try {
		if(mainWindow()->datasetContainer().askForSaveChanges()) {
			mainWindow()->datasetContainer().fileNew();
		}
	}
	catch(const Exception& ex) {
		ex.reportError();
	}
}

/******************************************************************************
* Handles the ACTION_FILE_OPEN command.
******************************************************************************/
void ActionManager::on_FileOpen_triggered()
{

	try {
		if(!mainWindow()->datasetContainer().askForSaveChanges())
			return;

		QSettings settings;
		settings.beginGroup("file/scene");

		// Go to the last directory used.
		QString defaultPath;
		OORef<DataSet> dataSet = mainWindow()->datasetContainer().currentSet();
		if(dataSet == NULL || dataSet->filePath().isEmpty())
			defaultPath = settings.value("last_directory").toString();
		else
			defaultPath = dataSet->filePath();

		QString filename = QFileDialog::getOpenFileName(mainWindow(), tr("Load Program State"),
				defaultPath, tr("OVITO State Files (*.Ovito);;All Files (*)"));
		if(filename.isEmpty())
			return;

		// Remember directory for the next time...
		settings.setValue("last_directory", QFileInfo(filename).absolutePath());


		mainWindow()->datasetContainer().fileLoad(filename);
	}
	catch(const Exception& ex) {
		ex.reportError();
	}

/*
     try{

	QString path = "/home/ovito/ovito/examples/data/MgSnCu4N8.xyz";
	mainWindow()->datasetContainer().fileLoad(path);
    }catch(const Exception& ex) {
	ex.reportError();
    }
*/

}


/******************************************************************************
* Handles the ACTION_LIBRARY_FILE_IMPORT command.
******************************************************************************/

void ActionManager::on_LibraryFileImport_triggered()
{

    QDialog dlg(mainWindow());

    dlg.setWindowTitle(tr("Library Structures"));


    //QWidget window;
    QVBoxLayout *list1 = new QVBoxLayout();
    QVBoxLayout *list2 = new QVBoxLayout();
    QVBoxLayout *list3 = new QVBoxLayout();
//


    QToolButton* bccButton = new QToolButton();
    bccButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    bccButton->setIcon(QIcon( ":/gui/actions/file/BCC_icon.png"));
    bccButton->setText("BCC");
    bccButton->setIconSize(QSize(100,100));
    connect(bccButton, &QToolButton::clicked, &dlg, &QDialog::accept);
    connect(bccButton, SIGNAL(clicked()), this, SLOT(LoadBCC()));


    QToolButton* CaB6Button = new QToolButton();
    CaB6Button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    CaB6Button->setIcon(QIcon( ":/gui/actions/file/CaB6_icon.png"));
    CaB6Button->setText(QString::fromUtf8("CaB\342\202\206"));
    CaB6Button->setIconSize(QSize(100,100));
    connect(CaB6Button, &QToolButton::clicked, &dlg, &QDialog::accept);
    connect(CaB6Button, SIGNAL(clicked()), this, SLOT(LoadCaB6()));

    QToolButton* ZincBlendeButton = new QToolButton();
    ZincBlendeButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    ZincBlendeButton->setIcon(QIcon( ":/gui/actions/file/ZincBlende_icon.png"));
    ZincBlendeButton->setText("Zinc Blende");
    ZincBlendeButton->setIconSize(QSize(100,100));
    connect(ZincBlendeButton, &QToolButton::clicked, &dlg, &QDialog::accept);
    connect(ZincBlendeButton, SIGNAL(clicked()), this, SLOT(LoadZincBlende()));

		// Nick Added
		QToolButton* csclButton = new QToolButton();
		csclButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
		csclButton->setIcon(QIcon( ":/gui/actions/file/BCC_icon.png"));
		csclButton->setText("CsCl");
		csclButton->setIconSize(QSize(100,100));
		connect(csclButton, &QToolButton::clicked, &dlg, &QDialog::accept);
		connect(csclButton, SIGNAL(clicked()), this, SLOT(LoadCsCl()));

    list1->addWidget(bccButton);
    list1->addWidget(CaB6Button);
    list1->addWidget(ZincBlendeButton);
		// Nick Added
		list1->addWidget(csclButton);

    QToolButton* Cu3AuButton = new QToolButton();
    Cu3AuButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    Cu3AuButton->setIcon(QIcon( ":/gui/actions/file/Cu3Au_Icon.png"));
    Cu3AuButton->setText(QString::fromUtf8("Cu\342\202\203Au"));
    Cu3AuButton->setIconSize(QSize(100,100));
    connect(Cu3AuButton, &QToolButton::clicked, &dlg, &QDialog::accept);
    connect(Cu3AuButton, SIGNAL(clicked()), this, SLOT(LoadCu3Au()));

    QToolButton* DiamondButton = new QToolButton();
    DiamondButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    DiamondButton->setIcon(QIcon( ":/gui/actions/file/Diamond_icon.png"));
    DiamondButton->setText("Diamond");
    DiamondButton->setIconSize(QSize(100,100));
    connect(DiamondButton, &QToolButton::clicked, &dlg, &QDialog::accept);
    connect(DiamondButton, SIGNAL(clicked()), this, SLOT(LoadDiamond()));

    QToolButton* FCCButton = new QToolButton();
    FCCButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    FCCButton->setIcon(QIcon( ":/gui/actions/file/FCC_icon.png"));
    FCCButton->setText("FCC");
    FCCButton->setIconSize(QSize(100,100));
    connect(FCCButton, &QToolButton::clicked, &dlg, &QDialog::accept);
    connect(FCCButton, SIGNAL(clicked()), this, SLOT(LoadFCC()));


    list2->addWidget(Cu3AuButton);
    list2->addWidget(DiamondButton);
    list2->addWidget(FCCButton);

    QToolButton* NaClButton = new QToolButton();
    NaClButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    NaClButton->setIcon(QIcon( ":/gui/actions/file/NaCl-icon.png"));
    NaClButton->setText("NaCl");
    NaClButton->setIconSize(QSize(100,100));
    connect(NaClButton, &QToolButton::clicked, &dlg, &QDialog::accept);
    connect(NaClButton, SIGNAL(clicked()), this, SLOT(LoadNaCl()));

    QToolButton* SCButton = new QToolButton();
    SCButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    SCButton->setIcon(QIcon( ":/gui/actions/file/SimpleCubic_icon.png"));
    SCButton->setText("Simple Cubic");
    SCButton->setIconSize(QSize(100,100));
    connect(SCButton, &QToolButton::clicked, &dlg, &QDialog::accept);
    connect(SCButton, SIGNAL(clicked()), this, SLOT(LoadSC()));

    QToolButton* MgSnCuButton = new QToolButton();
    MgSnCuButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    MgSnCuButton->setIcon(QIcon( ":/gui/actions/file/LaVeS PHASE_icon.png"));
    MgSnCuButton->setText("Laves");
    MgSnCuButton->setIconSize(QSize(100,100));
    connect(MgSnCuButton, &QToolButton::clicked, &dlg, &QDialog::accept);
    connect(MgSnCuButton, SIGNAL(clicked()), this, SLOT(loadMgSnCu()));


    list3->addWidget(NaClButton);
    list3->addWidget(SCButton);
    list3->addWidget(MgSnCuButton);

    QHBoxLayout *MainLayout = new QHBoxLayout(&dlg);
    MainLayout->addLayout(list1);
    MainLayout->addLayout(list2);
    MainLayout->addLayout(list3);

    dlg.exec();
}

void ActionManager::loadMgSnCu(){
    QDialog dlg(mainWindow());

    dlg.setWindowTitle(tr("Laves"));

    QToolButton* MgSnCu2Button = new QToolButton();
    MgSnCu2Button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    MgSnCu2Button->setIcon(QIcon( ":/gui/actions/file/MgCu4_icon.png"));
    MgSnCu2Button->setText(QString::fromUtf8("MgCu\342\202\202"));
    MgSnCu2Button->setIconSize(QSize(100,100));
    connect(MgSnCu2Button, &QToolButton::clicked, &dlg, &QDialog::accept);
    connect(MgSnCu2Button, SIGNAL(clicked()), this, SLOT(loadMgCu2()));

    QToolButton* MgSnCu4Button = new QToolButton();
    MgSnCu4Button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    MgSnCu4Button->setIcon(QIcon( ":/gui/actions/file/MgSnCu4.png"));
    MgSnCu4Button->setText(QString::fromUtf8("MgSnCu\342\202\204"));
    MgSnCu4Button->setIconSize(QSize(100,100));
    connect(MgSnCu4Button, &QToolButton::clicked, &dlg, &QDialog::accept);
    connect(MgSnCu4Button, SIGNAL(clicked()), this, SLOT(loadMgSnCu4()));

    QVBoxLayout *list1 = new QVBoxLayout();
    list1->addWidget(MgSnCu2Button);

    QVBoxLayout *list2 = new QVBoxLayout();
    list2->addWidget(MgSnCu4Button);

    QHBoxLayout *MainLayout = new QHBoxLayout(&dlg);
    MainLayout->addLayout(list1);
    MainLayout->addLayout(list2);


    dlg.exec();

}
void ActionManager::LoadA1B2(){

    //  create file
    StructureMenu();
    if(confirmed){
	    A1B2(xValue, yValue, zValue, numOfAtoms);
    }else{
	return;
    }
    /// load file
    try{
    	QString path = "A1B2.xyz";
	if(radius > 0){
		cut(path, 0, 0, 0, radius);
	}
    	mainWindow()->datasetContainer().importFile(QUrl::fromLocalFile(path));
    }catch(const Exception& ex) {
    	ex.reportError();
    }
}

void ActionManager::LoadBCC(){

    //  create file
    StructureMenu();
    if(confirmed){
    	BCC(xValue, yValue, zValue, numOfAtoms);
    }else{
	return;
    }

    /// load file
    try{
    	QString path = "bcc.xyz";
	if(radius > 0){
		cut(path, 0, 0, 0, radius);
	}
    	mainWindow()->datasetContainer().importFile(QUrl::fromLocalFile(path));
    }catch(const Exception& ex) {
    	ex.reportError();
    }
}

void ActionManager::LoadCaB6(){

    //  create file
    StructureMenu();
    if(confirmed){
         CaB6(xValue, yValue, zValue, numOfAtoms);
    }else{
	return;
    }

    /// load file
    try{
    	QString path = "CaB6.xyz";
	if(radius > 0){
		cut(path, 0, 0, 0, radius);
	}
    	mainWindow()->datasetContainer().importFile(QUrl::fromLocalFile(path));
    }catch(const Exception& ex) {
    	ex.reportError();
    }
}

void ActionManager::LoadCu3Au(){

    //  create file
    StructureMenu();
    if(confirmed){
         Cu3Au(xValue, yValue, zValue, numOfAtoms);
    }else{
	return;
    }

    /// load file
    try{
    	QString path = "Cu3Au.xyz";
	if(radius > 0){
		cut(path, 0, 0, 0, radius);
	}
    	mainWindow()->datasetContainer().importFile(QUrl::fromLocalFile(path));
    }catch(const Exception& ex) {
    	ex.reportError();
    }
}

void ActionManager::LoadDiamond(){

    //  create file
    StructureMenu();
    if(confirmed){
         Diamond(xValue, yValue, zValue, numOfAtoms);
    }else{
	return;
    }
    /// load file
    try{

    	QString path = "Diamond.xyz";
	if(radius > 0){
		cut(path, 0, 0, 0, radius);
	}
    	mainWindow()->datasetContainer().importFile(QUrl::fromLocalFile(path));
    }catch(const Exception& ex) {
    	ex.reportError();
    }
}

void ActionManager::LoadFCC(){


    StructureMenu();
    if(confirmed){
         FCC(xValue, yValue, zValue, numOfAtoms);
    }else{
	return;
    }
    /// load file
    try{

    	QString path = "fcc.xyz";
	if(radius > 0){
		cut(path, 0, 0, 0, radius);
	}
    	mainWindow()->datasetContainer().importFile(QUrl::fromLocalFile(path));
    }catch(const Exception& ex) {
   	 ex.reportError();
    }
}

void ActionManager::LoadNaCl(){

    //  create file
    StructureMenu();
    if(confirmed){
         NaCl(xValue, yValue, zValue, numOfAtoms);
    }else{
	return;
    }
    /// load file
    try{

    	QString path = "NaCl.xyz";
	if(radius > 0){
		cut(path, 0, 0, 0, radius);
	}
    	mainWindow()->datasetContainer().importFile(QUrl::fromLocalFile(path));
    }catch(const Exception& ex) {
   	 ex.reportError();
    }
}

void ActionManager::LoadSC(){

    //  create file
    StructureMenu();
    if(confirmed){
         SC(xValue, yValue, zValue, numOfAtoms);
    }else{
	return;
    }
    /// load file
    try{

    	QString path = "SC.xyz";
	if(radius > 0){
		cut(path, 0, 0, 0, radius);
	}
    	mainWindow()->datasetContainer().importFile(QUrl::fromLocalFile(path));
    }catch(const Exception& ex) {
   	 ex.reportError();
    }
}


void ActionManager::LoadZincBlende(){

    //  create file
    StructureMenu();
    if(confirmed){
    	ZincBlende(xValue, yValue, zValue, numOfAtoms);
    }else{
	return;
    }

    /// load file
    try{

    	QString path = "ZincBlende.xyz";
	if(radius > 0){
		cut(path, 0, 0, 0, radius);
	}
    	mainWindow()->datasetContainer().importFile(QUrl::fromLocalFile(path));
    }catch(const Exception& ex) {
   	 ex.reportError();
    }
}
void ActionManager::loadMgSnCu4(){

    //  create file
    StructureMenu();
    if(confirmed){
    	MgSnCu4(xValue, yValue, zValue, numOfAtoms);
    }else{
	return;
    }

    /// load file
    try{

    	QString path = "MgSnCu4.xyz";
	if(radius > 0){
		cut(path, 0, 0, 0, radius);
	}
    	mainWindow()->datasetContainer().importFile(QUrl::fromLocalFile(path));
    }catch(const Exception& ex) {
   	 ex.reportError();
    }
}
void ActionManager::loadMgCu2(){

    //  create file
    StructureMenu();
    if(confirmed){
    	MgCu2(xValue, yValue, zValue, numOfAtoms);
    }else{
	return;
    }

    /// load file
    try{
    	QString path = "MgCu2.xyz";
	if(radius > 0){
		cut(path, 0, 0, 0, radius);
	}
    	mainWindow()->datasetContainer().importFile(QUrl::fromLocalFile(path));
    }catch(const Exception& ex) {
   	 ex.reportError();
    }
}

void ActionManager::LoadCsCl(){

    //  create file
    StructureMenu();
    if(confirmed){
    	CsCl(xValue, yValue, zValue, numOfAtoms);
    }else{
	return;
    }

    /// load file
    try{
    	QString path = "CsCl.xyz";
	if(radius > 0){
		cut(path, 0, 0, 0, radius);
	}
    	mainWindow()->datasetContainer().importFile(QUrl::fromLocalFile(path));
    }catch(const Exception& ex) {
   	 ex.reportError();
    }
}


void ActionManager::StructureMenu(){

    //  create file
    confirmed = false;
    cutSphere = false;
    numOfAtoms = 1;
    xValue = 2;
    yValue = 2;
    zValue = 2;
    radius = -1;

    QDialog dlg(mainWindow());
    dlg.setWindowTitle(tr("Library Structures"));

    QLineEdit *xInput = new QLineEdit();
    QLineEdit *yInput = new QLineEdit();
    QLineEdit *zInput = new QLineEdit();
    QLineEdit *atomNum = new QLineEdit();
    sphereRadius = new QLineEdit();
    QCheckBox *cut = new QCheckBox();

    xInput->setText("2");
    yInput->setText("2");
    zInput->setText("2");
    atomNum->setText("1");
    cut ->setChecked(false);



    QFormLayout* formLayout = new QFormLayout();
    formLayout->addRow(tr("X :"), xInput);
    formLayout->addRow(tr("Y :"), yInput);
    formLayout->addRow(tr("Z :"), zInput);
    formLayout->addRow(tr("Acell:"), atomNum);
    formLayout->addRow(tr("Spherical Domain:"), cut);
    formLayout->addRow(tr("Radius:"), sphereRadius);
    sphereRadius->setEnabled(cutSphere);

    connect(xInput, &QLineEdit::textChanged,this, &ActionManager::changeX);
    connect(yInput, &QLineEdit::textChanged,this, &ActionManager::changeY);
    connect(zInput, &QLineEdit::textChanged,this, &ActionManager::changeZ);
    connect(atomNum, &QLineEdit::textChanged,this, &ActionManager::changeNumOfAtoms);
    connect(sphereRadius, &QLineEdit::textChanged,this, &ActionManager::changeRadius);
    connect(cut, SIGNAL(clicked(bool)),this, SLOT(changeCutSphere(bool)));



    QHBoxLayout *MainLayout = new QHBoxLayout(&dlg);

    QPushButton * ok = new QPushButton(tr("Ok"));
    connect(ok, &QPushButton::clicked, &dlg, &QDialog::accept);
    connect(ok, SIGNAL(clicked()), this, SLOT(StructureConfirmed()));

    QPushButton * cancel = new QPushButton(tr("Cancel"));
    connect(cancel, &QPushButton::clicked, &dlg, &QDialog::reject);

    QHBoxLayout *okLayout = new QHBoxLayout();

    okLayout->addWidget(ok);
    okLayout->addWidget(cancel);

    formLayout->addRow(okLayout);


    MainLayout->addLayout(formLayout);
    dlg.exec();

}
void ActionManager::changeX(const QString & newX){
	xValue = newX.toInt();
}
void ActionManager::changeY(const QString & newY){
	yValue = newY.toInt();
}
void ActionManager::changeZ(const QString & newZ){
	zValue = newZ.toInt();
}
void ActionManager::changeRadius(const QString & r){
	radius = r.toFloat();
}
void ActionManager::changeNumOfAtoms(const QString &  newNumOfAtoms){
	numOfAtoms = newNumOfAtoms.toFloat();
}
void ActionManager::changeCutSphere(bool cut){
	sphereRadius->setEnabled(cut);
}
void ActionManager::StructureConfirmed(){
	confirmed = true;
}

/******************************************************************************
* Handles the ACTION_FILE_SAVE command.
******************************************************************************/
void ActionManager::on_FileSave_triggered()
{
	// Set focus to main window.
	// This will process any pending user inputs in QLineEdit fields.
	mainWindow()->setFocus();

	try {
		mainWindow()->datasetContainer().fileSave();
	}
	catch(const Exception& ex) {
		ex.reportError();
	}
}

/******************************************************************************
* Handles the ACTION_FILE_SAVEAS command.
******************************************************************************/
void ActionManager::on_FileSaveAs_triggered()
{
	try {
		mainWindow()->datasetContainer().fileSaveAs();
	}
	catch(const Exception& ex) {
		ex.reportError();
	}
}

/******************************************************************************
* Handles the ACTION_SETTINGS_DIALOG command.
******************************************************************************/
void ActionManager::on_Settings_triggered()
{
	ApplicationSettingsDialog dlg(mainWindow());
	dlg.exec();
}

/******************************************************************************
* Handles the ACTION_FILE_IMPORT command.
******************************************************************************/
void ActionManager::on_FileImport_triggered()
{
	try {
		//Let the user select a file.
		ImportFileDialog dialog(PluginManager::instance().metaclassMembers<FileImporter>(), _dataset, mainWindow(), tr("Load File"));
		if(dialog.exec() != QDialog::Accepted)
			return;

		//Import file.
		mainWindow()->datasetContainer().importFile(QUrl::fromLocalFile(dialog.fileToImport()), dialog.selectedFileImporterType());

	}
	catch(const Exception& ex) {
		ex.reportError();
	}
}

/******************************************************************************
* Handles the ACTION_FILE_REMOTE_IMPORT command.
******************************************************************************/
void ActionManager::on_FileRemoteImport_triggered()
{
	try {
		// Let the user enter the URL of the remote file.
		ImportRemoteFileDialog dialog(PluginManager::instance().metaclassMembers<FileImporter>(), _dataset, mainWindow(), tr("Load Remote File"));
		if(dialog.exec() != QDialog::Accepted)
			return;

		// Import URL.
		mainWindow()->datasetContainer().importFile(dialog.fileToImport(), dialog.selectedFileImporterType());
	}
	catch(const Exception& ex) {
		ex.reportError();
	}
}

/******************************************************************************
* Handles the ACTION_FILE_EXPORT command.
******************************************************************************/
void ActionManager::on_FileExport_triggered()
{
	// Build filter string.
	QStringList filterStrings;
	QVector<const FileExporterClass*> exporterTypes = PluginManager::instance().metaclassMembers<FileExporter>();
	if(exporterTypes.empty()) {
		Exception(tr("This function is disabled, because there are no export services available."), _dataset).reportError();
		return;
	}
	for(const FileExporterClass* exporterClass : exporterTypes) {
		filterStrings << QString("%1 (%2)").arg(exporterClass->fileFilterDescription(), exporterClass->fileFilter());
	}

	QSettings settings;
	settings.beginGroup("file/export");

	// Let the user select a destination file.
	HistoryFileDialog dialog("export", mainWindow(), tr("Export Data"));
	dialog.setNameFilters(filterStrings);
	dialog.setAcceptMode(QFileDialog::AcceptSave);
	dialog.setFileMode(QFileDialog::AnyFile);
	dialog.setConfirmOverwrite(true);

	// Go to the last directory used.
	QString lastExportDirectory = settings.value("last_export_dir").toString();
	if(!lastExportDirectory.isEmpty())
		dialog.setDirectory(lastExportDirectory);
	// Select the last export filter being used ...
	QString lastExportFilter = settings.value("last_export_filter").toString();
	if(!lastExportFilter.isEmpty())
		dialog.selectNameFilter(lastExportFilter);

	if(!dialog.exec())
		return;

	QStringList files = dialog.selectedFiles();
	if(files.isEmpty())
		return;

	QString exportFile = files.front();

	// Remember directory for the next time...
	settings.setValue("last_export_dir", dialog.directory().absolutePath());
	// Remember export filter for the next time...
	settings.setValue("last_export_filter", dialog.selectedNameFilter());

	// Export to selected file.
	try {
		int exportFilterIndex = filterStrings.indexOf(dialog.selectedNameFilter());
		OVITO_ASSERT(exportFilterIndex >= 0 && exportFilterIndex < exporterTypes.size());

		// Create exporter.
		OORef<FileExporter> exporter = static_object_cast<FileExporter>(exporterTypes[exportFilterIndex]->createInstance(_dataset));

		// Load user-defined default settings.
		exporter->loadUserDefaults();

		// Pass output filename to exporter.
		exporter->setOutputFilename(exportFile);

		// Wait until the scene is ready.
		{
			ProgressDialog progressDialog(mainWindow(), tr("File export"));
			if(!progressDialog.taskManager().waitForTask(_dataset->whenSceneReady()))
				return;
		}

		// Choose the scene nodes to be exported.
		exporter->selectStandardOutputData();

		// Let the user adjust the settings of the exporter.
		FileExporterSettingsDialog settingsDialog(mainWindow(), exporter);
		if(settingsDialog.exec() != QDialog::Accepted)
			return;

		// Show progress dialog.
		ProgressDialog progressDialog(mainWindow(), tr("File export"));

		// Let the exporter do its work.
		exporter->exportNodes(progressDialog.taskManager());
	}
	catch(const Exception& ex) {
		ex.reportError();
	}
}

OVITO_END_INLINE_NAMESPACE
}	// End of namespace
