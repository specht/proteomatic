/*
Copyright (c) 2007-2008 Michael Specht

This file is part of Proteomatic.

Proteomatic is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Proteomatic is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Proteomatic.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "PipelineMainWindow.h"
#include "Desktop.h"
#include <QtGui>
#include "Proteomatic.h"


k_PipelineMainWindow::k_PipelineMainWindow(QWidget* ak_Parent_, k_Proteomatic& ak_Proteomatic)
	: QMainWindow(ak_Parent_)
	, mk_Desktop(this, ak_Proteomatic, *this)
	, mk_Proteomatic(ak_Proteomatic)
{
	mk_Proteomatic.setMessageBoxParent(this);

	setWindowIcon(QIcon(":/icons/proteomatic.png"));
	setWindowTitle("Proteomatic Pipeline");
	resize(800, 600);
	setCentralWidget(&mk_Desktop);
	statusBar()->show();

	QToolBar* lk_AddToolBar_ = new QToolBar(this);
	lk_AddToolBar_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

	QToolButton* lk_AddScriptButton_ = new QToolButton(lk_AddToolBar_);
	lk_AddScriptButton_->setIcon(QIcon(":/icons/folder.png"));
	lk_AddScriptButton_->setText("Add script");
	lk_AddScriptButton_->setMenu(mk_Proteomatic.proteomaticScriptsMenu());
	lk_AddScriptButton_->setPopupMode(QToolButton::InstantPopup);
	lk_AddScriptButton_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	lk_AddToolBar_->addWidget(lk_AddScriptButton_);
	connect(mk_Proteomatic.proteomaticScriptsMenu(), SIGNAL(triggered(QAction*)), &mk_Desktop, SLOT(addScriptBox(QAction*)));
	mk_AddScriptAction_ = lk_AddScriptButton_;

	mk_AddFilesAction_ = lk_AddToolBar_->addAction(QIcon(":/icons/document-open.png"), "Add files", this, SLOT(addFiles()));
	mk_AddFileListAction_ = lk_AddToolBar_->addAction(QIcon(":/icons/document-open-multiple.png"), "Add file list", this, SLOT(addFileListBox()));

	lk_AddToolBar_->addSeparator();

	mk_RefreshAction_ = lk_AddToolBar_->addAction(QIcon(":icons/view-refresh.png"), "Refresh", this, SIGNAL(forceRefresh()));
	mk_StartAction_ = lk_AddToolBar_->addAction(QIcon(":icons/dialog-ok.png"), "Start", this, SLOT(start()));

	lk_AddToolBar_->addSeparator();
	
	lk_AddToolBar_->addWidget(new QLabel("Output directory:", this));
	
	QString ls_Path = mk_Proteomatic.getConfiguration(CONFIG_REMEMBER_OUTPUT_PATH).toString();
	if (!QFileInfo(ls_Path).isDir())
		ls_Path = QDir::homePath();
	this->setOutputDirectory(ls_Path);
	mk_OutputDirectory.setReadOnly(true);
	
	lk_AddToolBar_->addWidget(&mk_OutputDirectory);
	mk_ChooseOutputDirectoryAction_ = lk_AddToolBar_->addAction(QIcon(":icons/folder.png"), "", this, SLOT(chooseOutputDirectory()));
	
	lk_AddToolBar_->addSeparator();

	addToolBar(Qt::TopToolBarArea, lk_AddToolBar_);
	
	connect(&mk_FileSystemWatcher, SIGNAL(directoryChanged(const QString&)), &mk_Desktop, SLOT(forceRefresh()));

	show();
}


k_PipelineMainWindow::~k_PipelineMainWindow()
{
}


QString k_PipelineMainWindow::outputDirectory()
{
	return mk_OutputDirectory.text();
}


void k_PipelineMainWindow::start()
{
	mk_Desktop.start();
}


void k_PipelineMainWindow::addFiles()
{
	QStringList lk_Files = QFileDialog::getOpenFileNames(this, tr("Add files"), mk_Proteomatic.getConfiguration(CONFIG_REMEMBER_INPUT_FILES_PATH).toString());
	if (!lk_Files.empty())
		mk_Proteomatic.getConfigurationRoot()[CONFIG_REMEMBER_INPUT_FILES_PATH] = QFileInfo(lk_Files[0]).absolutePath();

	foreach (QString ls_Path, lk_Files)
		mk_Desktop.addFileBox(ls_Path);
}


void k_PipelineMainWindow::addFileListBox()
{
	k_InputFileListBox* lk_InputFileListBox_ = new k_InputFileListBox(&mk_Desktop, mk_Proteomatic);
	mk_Desktop.addBox(lk_InputFileListBox_);
}


void k_PipelineMainWindow::chooseOutputDirectory()
{
	QString ls_Path = QFileDialog::getExistingDirectory(this, tr("Select output directory"), mk_Proteomatic.getConfiguration(CONFIG_REMEMBER_OUTPUT_PATH).toString());
	if (ls_Path.length() > 0)
	{
		this->setOutputDirectory(ls_Path);
		mk_Proteomatic.getConfigurationRoot()[CONFIG_REMEMBER_OUTPUT_PATH] = ls_Path;
	}
}


void k_PipelineMainWindow::setOutputDirectory(QString as_Path)
{
	if (!mk_OutputDirectory.text().isEmpty())
		mk_FileSystemWatcher.removePath(mk_OutputDirectory.text());

	mk_OutputDirectory.setText(as_Path);
	mk_FileSystemWatcher.addPath(mk_OutputDirectory.text());

	emit outputDirectoryChanged(as_Path);
}


void k_PipelineMainWindow::toggleUi()
{
	mk_AddScriptAction_->setEnabled(!mk_Desktop.running());
	mk_AddFilesAction_->setEnabled(!mk_Desktop.running());
	mk_AddFileListAction_->setEnabled(!mk_Desktop.running());
	mk_StartAction_->setEnabled(!mk_Desktop.running());
	mk_RefreshAction_->setEnabled(!mk_Desktop.running());
	mk_ChooseOutputDirectoryAction_->setEnabled(!mk_Desktop.running());
}
