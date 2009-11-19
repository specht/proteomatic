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
#include "IFileBox.h"
#include "IScriptBox.h"
#include "ProfileManager.h"
#include "Proteomatic.h"
#include "version.h"
#include "Yaml.h"


k_PipelineMainWindow::k_PipelineMainWindow(QWidget* ak_Parent_, k_Proteomatic& ak_Proteomatic)
	: QMainWindow(ak_Parent_)
	, mk_Desktop_(new k_Desktop(this, ak_Proteomatic, *this))
	, mk_Proteomatic(ak_Proteomatic)
	//, mk_OutputPrefix_(new QLineEdit(this))
	, mk_CurrentScriptBox_(NULL)
	, ms_PipelineFilename(QString())
	, mk_WatchedBoxObject_(NULL)
{
	mk_Proteomatic.setMessageBoxParent(this);
	
	connect(&mk_FileSystemWatcher, SIGNAL(directoryChanged(const QString&)), this, SIGNAL(forceRefresh()));
	connect(mk_Desktop_, SIGNAL(selectionChanged()), this, SLOT(updateStatusBar()));

	setWindowIcon(QIcon(":/icons/proteomatic-pipeline.png"));
	updateWindowTitle();
	resize(1000, 600);
	
	mk_HSplitter_ = new QSplitter(this);
	mk_HSplitter_->setOrientation(Qt::Horizontal);
	mk_PaneLayoutWidget_ = new QWidget(this);
	mk_PaneLayout_ = new QVBoxLayout(mk_PaneLayoutWidget_);
	mk_PaneLayout_->setContentsMargins(0, 0, 0, 0);
	mk_PaneLayoutWidget_->hide();
	
	setCentralWidget(mk_HSplitter_);

	mk_HSplitter_->addWidget(mk_Desktop_);
	mk_HSplitter_->addWidget(mk_PaneLayoutWidget_);
	mk_HSplitter_->setChildrenCollapsible(false);

	statusBar()->show();
	mk_StatusBarMessage_ = new QLabel("", this);
	statusBar()->addWidget(mk_StatusBarMessage_);
	statusBar()->addPermanentWidget(mk_Proteomatic.fileTrackerIconLabel());
	statusBar()->addPermanentWidget(mk_Proteomatic.fileTrackerLabel());
	
	QToolBar* lk_AddToolBar_ = new QToolBar(this);
	lk_AddToolBar_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

	QToolButton* lk_ProteomaticButton_ = new QToolButton(lk_AddToolBar_);
	lk_ProteomaticButton_->setIcon(QIcon(":/icons/proteomatic-pipeline.png"));
	lk_ProteomaticButton_->setText("Pipeline");
	QMenu* lk_ProteomaticMenu_ = new QMenu(this);
	mk_NewPipelineAction_ = lk_ProteomaticMenu_->addAction(QIcon(":icons/document-new.png"), "New pipeline", this, SLOT(newPipeline()), QKeySequence("Ctrl+N"));
	mk_LoadPipelineAction_ = lk_ProteomaticMenu_->addAction(QIcon(":icons/document-open.png"), "Open pipeline...", this, SLOT(loadPipeline()), QKeySequence("Ctrl+O"));
	mk_SavePipelineAction_ = lk_ProteomaticMenu_->addAction(QIcon(":icons/document-save.png"), "Save pipeline", this, SLOT(savePipeline()), QKeySequence("Ctrl+S"));
	mk_SavePipelineAsAction_ = lk_ProteomaticMenu_->addAction(QIcon(":icons/document-save-as.png"), "Save pipeline as...", this, SLOT(savePipelineAs()));
	lk_ProteomaticMenu_->addSeparator();
	mk_QuitAction_ = lk_ProteomaticMenu_->addAction(QIcon(":icons/system-shutdown.png"), "Quit", this, SLOT(quit()));
	lk_ProteomaticButton_->setMenu(lk_ProteomaticMenu_);
	lk_ProteomaticButton_->setPopupMode(QToolButton::InstantPopup);
	lk_ProteomaticButton_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	lk_AddToolBar_->addWidget(lk_ProteomaticButton_);

	mk_AddScriptButton_ = new QToolButton(lk_AddToolBar_);
	mk_AddScriptButton_->setIcon(QIcon(":/icons/proteomatic.png"));
	mk_AddScriptButton_->setText("Add script");
	mk_AddScriptButton_->setPopupMode(QToolButton::InstantPopup);
	mk_AddScriptButton_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	lk_AddToolBar_->addWidget(mk_AddScriptButton_);
	mk_AddScriptButton_->setMenu(mk_Proteomatic.proteomaticScriptsMenu());
	connect(mk_Proteomatic.proteomaticScriptsMenu(), SIGNAL(triggered(QAction*)), this, SLOT(addScript(QAction*)));
	mk_AddScriptAction_ = mk_AddScriptButton_;
	connect(&mk_Proteomatic, SIGNAL(scriptMenuChanged()), this, SLOT(scriptMenuChanged()));

	mk_AddFileListAction_ = lk_AddToolBar_->addAction(QIcon(":/icons/document-open.png"), "Add file list", this, SLOT(addFileListBox()));

	lk_AddToolBar_->addSeparator();
	
	lk_AddToolBar_->addAction(QIcon(":icons/system-search.png"), "Show all", this, SLOT(showAll()));

	lk_AddToolBar_->addSeparator();
	
	mk_ProposePrefixForAllScriptsAction_ = lk_AddToolBar_->addAction(QIcon(":icons/select-continuous-area.png"), "Propose prefixes", mk_Desktop_, SLOT(proposePrefixForAllScripts()));
	mk_RefreshAction_ = lk_AddToolBar_->addAction(QIcon(":icons/view-refresh.png"), "Refresh", this, SIGNAL(forceRefresh()));
	lk_AddToolBar_->addWidget(mk_Proteomatic.startButton());
	connect(mk_Proteomatic.startButton(), SIGNAL(clicked()), this, SLOT(start()));
	
	connect(mk_Proteomatic.startUntrackedAction(), SIGNAL(triggered()), this, SLOT(startUntracked()));
	mk_AbortAction_ = lk_AddToolBar_->addAction(QIcon(":icons/dialog-cancel.png"), "Abort", this, SLOT(abort()));

	lk_AddToolBar_->addSeparator();
	
	mk_ProfileManagerAction_ = lk_AddToolBar_->addAction(QIcon(":icons/document-properties.png"), "Profiles", this, SLOT(showProfileManager()));
	mk_ResetParametersAction_ = lk_AddToolBar_->addAction(QIcon(":icons/edit-clear.png"), "Reset", this, SLOT(resetParameters()));
	lk_AddToolBar_->addSeparator();
	lk_AddToolBar_->addAction(QIcon(":icons/preferences-system.png"), "Preferences", &mk_Proteomatic, SLOT(showConfigurationDialog()));
	
/*	QToolBar* lk_OtherToolBar_ = new QToolBar(this);
	lk_OtherToolBar_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	
	lk_OtherToolBar_->addWidget(new QLabel("Global output prefix: ", this));
	
	lk_OtherToolBar_->addWidget(mk_OutputPrefix_);
	connect(mk_OutputPrefix_, SIGNAL(textChanged(const QString&)), this, SIGNAL(outputPrefixChanged(const QString&)));
	
	mk_ClearPrefixForAllScriptsAction_ = lk_OtherToolBar_->addAction(QIcon(":icons/dialog-cancel.png"), "", mk_Desktop_, SLOT(clearPrefixForAllScripts()));
	mk_ProposePrefixForAllScriptsAction_ = lk_OtherToolBar_->addAction(QIcon(":icons/select-continuous-area.png"), "", mk_Desktop_, SLOT(proposePrefixForAllScripts()));
	mk_ProposePrefixForAllScriptsAction_->setCheckable(true);*/
	
	addToolBar(Qt::TopToolBarArea, lk_AddToolBar_);
/*	addToolBarBreak(Qt::TopToolBarArea);
	addToolBar(Qt::TopToolBarArea, lk_OtherToolBar_);*/
	
	lk_AddToolBar_->setIconSize(QSize(24, 24));
// 	lk_OtherToolBar_->setIconSize(QSize(24, 24));
	
	//setDockOptions(dockOptions() | QMainWindow::VerticalTabs);
	//setDockOptions(dockOptions() | QMainWindow::ForceTabbedDocks);
	//setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::North);
	
	updateStatusBar();
	toggleUi();
	show();
}


k_PipelineMainWindow::~k_PipelineMainWindow()
{
	delete mk_Desktop_;
}


/*QString k_PipelineMainWindow::outputPrefix()
{
	return mk_OutputPrefix_->text();
}
*/

void k_PipelineMainWindow::closeEvent(QCloseEvent* event)
{
	if (askForSaveIfNecessary())
		event->accept();
	else
		event->ignore();
}


void k_PipelineMainWindow::keyPressEvent(QKeyEvent* ak_Event_)
{
	if ((ak_Event_->key() == Qt::Key_R) && ((ak_Event_->modifiers() & Qt::ControlModifier) != 0))
	{
		mk_Proteomatic.reloadScripts();
		return;
	}
	QMainWindow::keyPressEvent(ak_Event_);
}


bool k_PipelineMainWindow::askForSaveIfNecessary()
{
	if (mk_Desktop_->hasUnsavedChanges())
	{
		// save discard cancel
		int li_Button = mk_Proteomatic.showMessageBox("Warning", "There are unsaved changes.", ":/icons/dialog-warning.png", QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save, QMessageBox::Cancel, "Do you want to save the current pipeline?");
		if (li_Button == QMessageBox::Cancel)
			return false;
		if (li_Button == QMessageBox::Save)
		{
			this->savePipelineAs();
			if (mk_Desktop_->hasUnsavedChanges())
				return false;
		}
	}
	return true;
}


void k_PipelineMainWindow::newPipeline()
{
	if (!askForSaveIfNecessary())
		return;
	mk_Desktop_->clearAll();
	mk_Desktop_->setHasUnsavedChanges(false);
	ms_PipelineFilename = QString();
	toggleUi();
}


void k_PipelineMainWindow::loadPipeline()
{
	if (!askForSaveIfNecessary())
		return;
	QString ls_Path;
	ls_Path = QFileDialog::getOpenFileName(this, "Load pipeline", mk_Proteomatic.getConfiguration(CONFIG_REMEMBER_PIPELINE_PATH).toString(), "Proteomatic Pipeline (*.pipeline)");
	if (!ls_Path.isEmpty())
	{
		tk_YamlMap lk_Description = k_Yaml::parseFromFile(ls_Path).toMap();
		ms_PipelineFilename = ls_Path;
		mk_Desktop_->applyPipelineDescription(lk_Description);
		mk_Desktop_->setHasUnsavedChanges(false);
	}
}


void k_PipelineMainWindow::savePipeline()
{
	if (ms_PipelineFilename.isEmpty())
		savePipelineAs();
	else
	{
		k_Yaml::emitToFile(mk_Desktop_->pipelineDescription(), ms_PipelineFilename);
		mk_Desktop_->setHasUnsavedChanges(false);
	}
}


void k_PipelineMainWindow::savePipelineAs()
{
	ms_PipelineFilename = QFileDialog::getSaveFileName(this, "Save pipeline as...", mk_Proteomatic.getConfiguration(CONFIG_REMEMBER_PIPELINE_PATH).toString() + "/" + (ms_PipelineFilename.isEmpty() ? "Unnamed" : QFileInfo(ms_PipelineFilename).completeBaseName()) + ".pipeline", "Proteomatic Pipeline (*.pipeline)");
	if (ms_PipelineFilename != "")
	{
		mk_Proteomatic.getConfigurationRoot()[CONFIG_REMEMBER_PIPELINE_PATH] = QFileInfo(ms_PipelineFilename).absolutePath();
		savePipeline();
	}
}


void k_PipelineMainWindow::quit()
{
	close();
}


void k_PipelineMainWindow::addScript(QAction* ak_Action_)
{
	if (!mk_Desktop_)
		return;
	
	QString ls_ScriptUri = ak_Action_->data().toString();
	mk_Desktop_->addScriptBox(ls_ScriptUri);
}


void k_PipelineMainWindow::start()
{
	if (!mk_Desktop_)
		return;
	
	mk_Desktop_->start();
}


void k_PipelineMainWindow::startUntracked()
{
	if (!mk_Desktop_)
		return;
	
	mk_Desktop_->start(false);
}


void k_PipelineMainWindow::abort()
{
	if (!mk_Desktop_)
		return;
	
	if (mk_Proteomatic.showMessageBox("Abort pipeline", "Are you sure you want to abort the pipeline?", ":/icons/dialog-warning.png", 
		QMessageBox::Yes | QMessageBox::No, QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
		mk_Desktop_->abort();
}


void k_PipelineMainWindow::addFileListBox()
{
	if (!mk_Desktop_)
		return;
	
	mk_Desktop_->addInputFileListBox();
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


void k_PipelineMainWindow::showAll()
{
	if (!mk_Desktop_)
		return;
	
	mk_Desktop_->showAll();
}


void k_PipelineMainWindow::updateStatusBar()
{
	QSet<IDesktopBox*> lk_SelectedBoxes = mk_Desktop_->selectedBoxes();
	mk_StatusBarMessage_->setText("");
/*	QObject* lk_Object_ = dynamic_cast<QObject*>(mk_WatchedBoxObject_);
	if (lk_Object_ != NULL)
		disconnect(lk_Object_, NULL, this, SLOT(updateStatusBar()));*/
	mk_WatchedBoxObject_ = NULL;
	if (lk_SelectedBoxes.size() == 1)
	{
		IDesktopBox* lk_Box_ = lk_SelectedBoxes.toList().first();
		IScriptBox* lk_ScriptBox_ = dynamic_cast<IScriptBox*>(lk_Box_);
		IFileBox* lk_FileBox_ = dynamic_cast<IFileBox*>(lk_Box_);
		if (lk_ScriptBox_)
		{
			QString ls_OutputDirectory = lk_ScriptBox_->outputDirectory();
			if (ls_OutputDirectory.isEmpty())
				ls_OutputDirectory = "<i>undefined</i>";
			mk_StatusBarMessage_->setText("<b>Output directory:</b> " + ls_OutputDirectory);
			QObject* lk_BoxObject_ = dynamic_cast<QObject*>(lk_ScriptBox_);
			//connect(lk_BoxObject_, SIGNAL(outputDirectoryChanged()), this, SLOT(updateStatusBar()));
			mk_WatchedBoxObject_ = lk_BoxObject_;
		}
		else if (lk_FileBox_)
		{
			QStringList lk_Filenames = lk_FileBox_->filenames(); 
			if (lk_Filenames.size() == 1)
			{
				QString ls_Path = lk_Filenames.first();
				mk_StatusBarMessage_->setText(ls_Path);
				QObject* lk_BoxObject_ = dynamic_cast<QObject*>(lk_FileBox_);
				//connect(lk_BoxObject_, SIGNAL(filenamesChanged()), this, SLOT(updateStatusBar()));
				mk_WatchedBoxObject_ = lk_BoxObject_;
			}
		}
	}
}


void k_PipelineMainWindow::scriptMenuChanged()
{
	mk_AddScriptButton_->setMenu(mk_Proteomatic.proteomaticScriptsMenu());
	connect(mk_Proteomatic.proteomaticScriptsMenu(), SIGNAL(triggered(QAction*)), this, SLOT(addScript(QAction*)));
	updateWindowTitle();
}


void k_PipelineMainWindow::updateWindowTitle()
{
	QString ls_ScriptsVersion = "(using scripts develop)";
	if (mk_Proteomatic.scriptsVersion() != "")
	{
		ls_ScriptsVersion = "(using scripts " + mk_Proteomatic.scriptsVersion() + ")";
	}
	QString ls_WindowTitle = QString("%1%2 - Proteomatic Pipeline %3 %4")
		.arg(ms_PipelineFilename.isEmpty()? "Unnamed" : QFileInfo(ms_PipelineFilename).completeBaseName())
		.arg(mk_Desktop_->hasUnsavedChanges() ? " [modified]" : "")
		.arg(mk_Proteomatic.version())
		.arg(ls_ScriptsVersion);
	setWindowTitle(ls_WindowTitle);
}


void k_PipelineMainWindow::toggleUi()
{
	updateWindowTitle();
	mk_NewPipelineAction_->setEnabled(mk_Desktop_ && !mk_Desktop_->running());
	mk_LoadPipelineAction_->setEnabled(mk_Desktop_ && !mk_Desktop_->running());
	mk_SavePipelineAction_->setEnabled(mk_Desktop_ && !mk_Desktop_->running() && mk_Desktop_->hasBoxes());
	mk_SavePipelineAsAction_->setEnabled(mk_Desktop_ && !mk_Desktop_->running() && mk_Desktop_->hasBoxes());
	mk_QuitAction_->setEnabled(mk_Desktop_ && !mk_Desktop_->running());
	mk_AddScriptAction_->setEnabled(mk_Desktop_ && !mk_Desktop_->running());
	mk_AddFileListAction_->setEnabled(mk_Desktop_ && !mk_Desktop_->running());
	mk_Proteomatic.startButton()->setEnabled(mk_Desktop_ && (!mk_Desktop_->running()) && (mk_Desktop_->hasBoxes()));
	mk_AbortAction_->setEnabled(mk_Desktop_ && mk_Desktop_->running());
	mk_RefreshAction_->setEnabled(mk_Desktop_ && (!mk_Desktop_->running()) && (mk_Desktop_->hasBoxes()));
	mk_ProfileManagerAction_->setEnabled(mk_Desktop_ && (!mk_Desktop_->running()));
	mk_ResetParametersAction_->setEnabled(mk_Desktop_ && (!mk_Desktop_->running()) && mk_CurrentScriptBox_);
	//mk_OutputPrefix_->setEnabled(mk_Desktop_ && !mk_Desktop_->running());
	//mk_ClearPrefixForAllScriptsAction_->setEnabled(mk_Desktop_ && !mk_Desktop_->running() && mk_Desktop_->hasBoxes());
	mk_ProposePrefixForAllScriptsAction_->setEnabled(mk_Desktop_ && !mk_Desktop_->running() && mk_Desktop_->hasBoxes());
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
		if (!mk_PaneLayoutWidget_->isVisible())
		{
			mk_PaneLayoutWidget_->show();
			QList<int> lk_Sizes;
			lk_Sizes.push_back(width() - 450);
			lk_Sizes.push_back(450);
			mk_HSplitter_->setSizes(lk_Sizes);
			mk_Desktop_->showAll();
		}
	}
	toggleUi();
}
