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

#include <QtGui>
#include "PipelineMainWindow.h"
#include "Desktop.h"
#include "ProfileManager.h"
#include "Proteomatic.h"


k_PipelineMainWindow::k_PipelineMainWindow(QWidget* ak_Parent_, k_Proteomatic& ak_Proteomatic)
	: QMainWindow(ak_Parent_)
	, mk_Desktop(this, ak_Proteomatic, *this)
	, mk_Proteomatic(ak_Proteomatic)
	, mk_OutputPrefix_(new QLineEdit(this))
	, mk_OutputDirectory_(new QLineEdit(this))
	, mk_CurrentScriptBox_(NULL)
{
	mk_Proteomatic.setMessageBoxParent(this);

	setWindowIcon(QIcon(":/icons/proteomatic.png"));
	setWindowTitle("Proteomatic Pipeline");
	resize(1000, 600);
	
	QSplitter* mk_HSplitter_ = new QSplitter(this);
	mk_HSplitter_->setStyle(new QPlastiqueStyle());
	mk_HSplitter_->setOrientation(Qt::Horizontal);
	mk_HSplitter_->setHandleWidth(4);
	mk_PaneLayoutWidget_ = new QWidget(this);
	mk_PaneLayout_ = new QVBoxLayout(mk_PaneLayoutWidget_);
	mk_PaneLayoutWidget_->hide();
	
	setCentralWidget(mk_HSplitter_);

	mk_HSplitter_->addWidget(&mk_Desktop);
	mk_HSplitter_->addWidget(mk_PaneLayoutWidget_);
	mk_HSplitter_->setChildrenCollapsible(false);
	mk_HSplitter_->setStretchFactor(0, 1);
	mk_HSplitter_->setStretchFactor(1, 1);

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
	connect(mk_Proteomatic.proteomaticScriptsMenu(), SIGNAL(triggered(QAction*)), this, SLOT(addScript(QAction*)));
	mk_AddScriptAction_ = lk_AddScriptButton_;

	mk_AddFileListAction_ = lk_AddToolBar_->addAction(QIcon(":/icons/document-open-multiple.png"), "Add file list", this, SLOT(addFileListBox()));

	lk_AddToolBar_->addSeparator();

	mk_RefreshAction_ = lk_AddToolBar_->addAction(QIcon(":icons/view-refresh.png"), "Refresh", this, SIGNAL(forceRefresh()));
	mk_StartAction_ = lk_AddToolBar_->addAction(QIcon(":icons/dialog-ok.png"), "Start", this, SLOT(start()));
	mk_AbortAction_ = lk_AddToolBar_->addAction(QIcon(":icons/dialog-cancel.png"), "Abort", this, SLOT(abort()));

	lk_AddToolBar_->addSeparator();
	
	mk_ProfileManagerAction_ = lk_AddToolBar_->addAction(QIcon(":icons/document-properties.png"), "Profiles", this, SLOT(showProfileManager()));
	mk_ResetParametersAction_ = lk_AddToolBar_->addAction(QIcon(":icons/edit-clear.png"), "Reset", this, SLOT(resetParameters()));
	
	QToolBar* lk_OtherToolBar_ = new QToolBar(this);
	lk_OtherToolBar_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	
	lk_OtherToolBar_->addWidget(new QLabel("Output prefix: ", this));
	
	lk_OtherToolBar_->addWidget(mk_OutputPrefix_);
	connect(mk_OutputPrefix_, SIGNAL(textChanged(const QString&)), this, SIGNAL(outputPrefixChanged(const QString&)));
	
	lk_OtherToolBar_->addSeparator();
	
	lk_OtherToolBar_->addWidget(new QLabel("Output directory: ", this));
	
	QString ls_Path = mk_Proteomatic.getConfiguration(CONFIG_REMEMBER_OUTPUT_PATH).toString();
	if (!QFileInfo(ls_Path).isDir())
		ls_Path = QDir::homePath();
	this->setOutputDirectory(ls_Path);
	mk_OutputDirectory_->setReadOnly(true);
	
	lk_OtherToolBar_->addWidget(mk_OutputDirectory_);
	mk_ChooseOutputDirectoryAction_ = lk_OtherToolBar_->addAction(QIcon(":icons/folder.png"), "", this, SLOT(chooseOutputDirectory()));
	
	addToolBar(Qt::TopToolBarArea, lk_AddToolBar_);
	addToolBarBreak(Qt::TopToolBarArea);
	addToolBar(Qt::TopToolBarArea, lk_OtherToolBar_);
		
	connect(&mk_FileSystemWatcher, SIGNAL(directoryChanged(const QString&)), &mk_Desktop, SLOT(refresh()));
	
	//setDockOptions(dockOptions() | QMainWindow::VerticalTabs);
	//setDockOptions(dockOptions() | QMainWindow::ForceTabbedDocks);
	setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::North);

	mk_Log_ = new QTextEdit(this);
	mk_Log_->setReadOnly(true);
	mk_Log_->setFont(mk_Proteomatic.consoleFont());
	//mk_LogDockWidget_->setFeatures(QDockWidget::NoDockWidgetFeatures);
	mk_Log_->hide();
	
	addOutput("Welcome to Proteomatic Pipeline.\n");

	toggleUi();
	show();
}


k_PipelineMainWindow::~k_PipelineMainWindow()
{
}


QString k_PipelineMainWindow::outputDirectory()
{
	return mk_OutputDirectory_->text();
}


QString k_PipelineMainWindow::outputPrefix()
{
	return mk_OutputPrefix_->text();
}


void k_PipelineMainWindow::addScript(QAction* ak_Action_)
{
	QString ls_ScriptUri = ak_Action_->data().toString();
	mk_Desktop.addScriptBox(ls_ScriptUri);
}


void k_PipelineMainWindow::start()
{
	mk_Desktop.start();
}


void k_PipelineMainWindow::abort()
{
	if (mk_Proteomatic.showMessageBox("Abort pipeline", "Are you sure you want to abort the pipeline?", ":/icons/dialog-warning.png", 
		QMessageBox::Yes | QMessageBox::No, QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
	{
		mk_Desktop.abort();
		//addOutput("\nScript aborted by user.");
	}
}


void k_PipelineMainWindow::addFileListBox()
{
	mk_Desktop.addInputFileListBox();
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
	if (!mk_OutputDirectory_->text().isEmpty())
		mk_FileSystemWatcher.removePath(mk_OutputDirectory_->text());

	mk_OutputDirectory_->setText(as_Path);
	mk_FileSystemWatcher.addPath(mk_OutputDirectory_->text());

	emit outputDirectoryChanged(as_Path);
}


void k_PipelineMainWindow::resetParameters()
{
	if (!mk_CurrentScriptBox_)
		return;
	mk_CurrentScriptBox_->script()->reset();
}


void k_PipelineMainWindow::showProfileManager()
{
	IScript* lk_Script_ = NULL;
	if (mk_CurrentScriptBox_)
		lk_Script_ = mk_CurrentScriptBox_->script();
	
	RefPtr<k_ProfileManager> lk_pProfileManager(new k_ProfileManager(mk_Proteomatic, lk_Script_, this));
	lk_pProfileManager->reset();
	if (lk_pProfileManager->exec())
	{
		if (lk_Script_)
			lk_Script_->setConfiguration(lk_pProfileManager->getGoodProfileMix());
	}
}


void k_PipelineMainWindow::toggleUi()
{
	mk_AddScriptAction_->setEnabled(!mk_Desktop.running());
	mk_AddFileListAction_->setEnabled(!mk_Desktop.running());
	mk_StartAction_->setEnabled((!mk_Desktop.running()) && (mk_Desktop.hasBoxes()));
	mk_AbortAction_->setEnabled(mk_Desktop.running());
	mk_RefreshAction_->setEnabled((!mk_Desktop.running()) && (mk_Desktop.hasBoxes()));
	mk_ProfileManagerAction_->setEnabled((!mk_Desktop.running()));
	mk_ResetParametersAction_->setEnabled((!mk_Desktop.running()) && mk_CurrentScriptBox_);
	mk_ChooseOutputDirectoryAction_->setEnabled(!mk_Desktop.running());
	mk_Desktop.setEnabled(!mk_Desktop.running());
}


void k_PipelineMainWindow::addOutput(QString as_Text)
{
	ms_Log.append(as_Text);
	mk_Log_->setText(ms_Log.text());
	mk_Log_->moveCursor(QTextCursor::End);
	mk_Log_->ensureCursorVisible();
}


void k_PipelineMainWindow::clearOutput()
{
	ms_Log.clear();
	mk_Log_->setText(ms_Log.text());
	mk_Log_->moveCursor(QTextCursor::End);
	mk_Log_->ensureCursorVisible();
}


void k_PipelineMainWindow::setCurrentScriptBox(IScriptBox* ak_ScriptBox_)
{
	mk_CurrentScriptBox_ = ak_ScriptBox_;

	QLayoutItem* lk_Item_;
	while ((lk_Item_ = mk_PaneLayout_->takeAt(0)) != NULL)
		lk_Item_->widget()->hide();
	
	if (ak_ScriptBox_)
	{
		mk_PaneLayout_->addWidget(ak_ScriptBox_->paneWidget());
		ak_ScriptBox_->paneWidget()->show();
		mk_PaneLayoutWidget_->show();
	}
	toggleUi();
}
