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

#include "ScriptHelper.h"
#include "ScriptFactory.h"
#include "ClickableLabel.h"
#include "CiListWidgetItem.h"
#include "IScript.h"
#include "Proteomatic.h"
#include "LocalScript.h"
#include "RemoteScript.h"
#include "TicketWindow.h"
#include <limits.h>
#include <float.h>


k_ScriptHelper::k_ScriptHelper(QWidget* ak_Parent_, k_Proteomatic& ak_Proteomatic)
	: QMainWindow(ak_Parent_)
	, ms_WindowTitle("Proteomatic")
	, mk_MainLayout(this)
	, mb_VersionChanged(false)
	, mk_Proteomatic(ak_Proteomatic)
	, mk_pProfileManager(new k_ProfileManager(ak_Proteomatic, NULL, this))
	, mk_ProgressDialog_(NULL)
{
	mk_Proteomatic.setMessageBoxParent(this);
	connect(&mk_Proteomatic, SIGNAL(remoteHubLineBatch(QStringList)), this, SLOT(remoteHubLineBatch(QStringList)));
	connect(&mk_Proteomatic, SIGNAL(remoteHubRequestFinished(int, bool, QString)), this, SLOT(remoteHubRequestFinished(int, bool, QString)));
	setWindowIcon(QIcon(":/icons/proteomatic.png"));
	this->updateWindowTitle();
	
	//mk_ProteomaticScriptVersionLabel_ = new QLabel(this);
	//mk_ProteomaticScriptVersionLabel_->setText("Scripts version: " + mk_Proteomatic.scriptsVersion());
	//QStatusBar* lk_StatusBar_ = new QStatusBar(this);
	//lk_StatusBar_->addWidget(0, mk_ProteomaticScriptVersionLabel_);
	//this->setStatusBar(lk_StatusBar_);
	
	QWidget* lk_LowerLayoutWidget_ = new QWidget(this);
	mk_LowerLayout_ = new QVBoxLayout(lk_LowerLayoutWidget_);
	mk_LowerLayout_->setContentsMargins(0, 0, 0, 0);

	mk_VSplitter_ = new QSplitter(this);
	mk_VSplitter_->setOrientation(Qt::Vertical);
	mk_VSplitter_->hide();
	mk_HSplitter_ = new QSplitter(this);
	mk_HSplitter_->setOrientation(Qt::Horizontal);

/*
	QFrame* lk_Frame_ = new QFrame(this);
	lk_Frame_->setFrameStyle(QFrame::HLine | QFrame::Sunken);

	QWidget* lk_Container_ = new QWidget(this);
	lk_Container_->setContentsMargins(0, 0, 0, 0);
	QLayout* lk_GroupBoxLayout_ = new QHBoxLayout(lk_Container_);
	lk_GroupBoxLayout_->setMargin(0);
	lk_GroupBoxLayout_->addWidget(&mk_Program);
	mk_LoadScriptButton.setIcon(QIcon(":/icons/drop-box.png"));
	lk_GroupBoxLayout_->addWidget(&mk_LoadScriptButton);
	lk_Container_->setLayout(lk_GroupBoxLayout_);
	mk_UpperLayout_->addWidget(lk_Container_);
	mk_UpperLayout_->addWidget(lk_Frame_);
*/
	QWidget* lk_Container_;
	QBoxLayout* lk_GroupBoxLayout_;
	

	QToolBar* lk_ToolBar_ = new QToolBar(this);
	lk_ToolBar_->setIconSize(QSize(24, 24));
	lk_ToolBar_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

	mk_LoadScriptButton_ = new QToolButton(lk_ToolBar_);
	mk_LoadScriptButton_->setIcon(QIcon(":/icons/document-open.png"));
	mk_LoadScriptButton_->setText("Load script");
	mk_LoadScriptButton_->setMenu(mk_Proteomatic.proteomaticScriptsMenu());
	connect(&mk_Proteomatic, SIGNAL(scriptMenuChanged()), this, SLOT(scriptMenuChanged()));
	mk_LoadScriptButton_->setPopupMode(QToolButton::InstantPopup);
	mk_LoadScriptButton_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	lk_ToolBar_->addWidget(mk_LoadScriptButton_);
	
	mk_ProfilesAction_ = lk_ToolBar_->addAction(QIcon(":/icons/document-properties.png"), "Profiles", this, SLOT(showProfileManager()));
	/*
	mk_ProfilesAction_->setEnabled(false);
	mk_ProfilesAction_->setVisible(false);
	*/
	
	//connect(mk_LoadScriptButton_, SIGNAL(clicked()), this, SLOT(showScriptMenu()));
	connect(&mk_Proteomatic, SIGNAL(scriptMenuScriptClicked(QAction*)), this, SLOT(scriptMenuScriptClicked(QAction*)));
	mk_ReloadScriptAction_ = lk_ToolBar_->addAction(QIcon(":/icons/edit-clear.png"), "Reset", this, SLOT(resetParameters()));
	
	lk_ToolBar_->addSeparator();
	
	mk_StartAction_ = lk_ToolBar_->addAction(QIcon(":/icons/dialog-ok.png"), "Start", this, SLOT(start()));
	mk_AbortAction_ = lk_ToolBar_->addAction(QIcon(":/icons/dialog-cancel.png"), "Abort", this, SLOT(abortScript()));
	
	// TODO: revive remote stuff
	mk_CheckTicketAction_ = new QAction(this);
	//mk_CheckTicketAction_ = lk_ToolBar_->addAction(QIcon(":/icons/ticket.png"), "&Check ticket");
	//connect(mk_CheckTicketAction_, SIGNAL(triggered()), this, SLOT(checkTicket()));
	mk_CheckTicketAction_->setEnabled(false);
	
	/*
	lk_ToolBar_->addSeparator();
	
	//QAction* lk_PreferencesOptions_ = lk_ToolBar_->addAction(QIcon(":/icons/preferences-system.png"), "Preferences...");
	lk_PreferencesOptions_->setEnabled(false);
	lk_PreferencesOptions_->setVisible(false);
	*/
	
	addToolBar(Qt::TopToolBarArea, lk_ToolBar_);

	//connect(&mk_LoadScriptButton, SIGNAL(clicked()), this, SLOT(showScriptMenu()));

	QLabel* lk_Label_ = new QLabel("<b>Output</b>", this);
	mk_LowerLayout_->addWidget(lk_Label_);

	lk_Container_ = new QWidget(this);
	lk_Label_->setBuddy(lk_Container_);
	lk_Container_->setContentsMargins(0, 0, 0, 0);
	lk_GroupBoxLayout_ = new QVBoxLayout(lk_Container_);
	lk_GroupBoxLayout_->setMargin(0);
	lk_GroupBoxLayout_->addWidget(&mk_Output);

	lk_Container_->setLayout(lk_GroupBoxLayout_);
	mk_LowerLayout_->addWidget(lk_Container_);

	mk_VSplitter_->addWidget(lk_LowerLayoutWidget_);

	mk_VSplitter_->setChildrenCollapsible(false);
	mk_VSplitter_->setStretchFactor(0, 3);
	mk_VSplitter_->setStretchFactor(1, 6);
	mk_HSplitter_->setChildrenCollapsible(false);
	mk_HSplitter_->setStretchFactor(0, 1);
	mk_HSplitter_->setStretchFactor(1, 1);


	mk_MainLayout.addWidget(mk_VSplitter_);
	//mk_TopLevelLayout_->addLayout(&mk_MainLayout);
	QWidget* lk_RightLayoutWidget_ = new QWidget(this);
	lk_RightLayoutWidget_->setLayout(&mk_MainLayout);
	mk_ScrollArea_ = new QScrollArea(this);
	mk_ScrollArea_->setWidgetResizable(true);
	mk_ScrollArea_->setFrameStyle(QFrame::NoFrame);
	mk_HSplitter_->addWidget(mk_ScrollArea_);
	mk_HSplitter_->addWidget(lk_RightLayoutWidget_);
	mk_ParameterLayout_ = new QVBoxLayout();
	mk_ParameterLayoutWidget_ = new QWidget(this);
	mk_ParameterLayoutWidget_->setLayout(mk_ParameterLayout_);
	mk_ParameterLayout_->setContentsMargins(8, 8, 8, 8);
	mk_ScrollArea_->setWidget(mk_ParameterLayoutWidget_);
	setCentralWidget(mk_HSplitter_);
	mk_ScrollArea_->setVisible(false);

	mk_Output.setReadOnly(true);
	mk_Output.setFont(mk_Proteomatic.consoleFont());

	QHBoxLayout* lk_HLayout_ = new QHBoxLayout();
	lk_HLayout_->addStretch();
	
	mk_LoadParametersAction_ = new QAction(this);
	mk_SaveParametersAction_ = new QAction(this);
	mk_ResetAction_ = new QAction(this);
	

/*
	mk_LoadParametersButton.setText("Load parameters");
	mk_LoadParametersButton.setIcon(QIcon(":/icons/drop-box.png"));
	connect(&mk_LoadParametersButton, SIGNAL(clicked()), this, SLOT(loadParameters()));
	lk_HLayout_->addWidget(&mk_LoadParametersButton);

	mk_SaveParametersButton.setText("Save parameters");
	mk_SaveParametersButton.setIcon(QIcon(":/icons/filesave.png"));
	connect(&mk_SaveParametersButton, SIGNAL(clicked()), this, SLOT(saveParameters()));
	lk_HLayout_->addWidget(&mk_SaveParametersButton);

	mk_ResetButton.setText("Reset");
	mk_ResetButton.setIcon(QIcon(":/icons/button_cancel.png"));
	connect(&mk_ResetButton, SIGNAL(clicked()), this, SLOT(reset()));
	lk_HLayout_->addWidget(&mk_ResetButton);

	mk_ResetParametersButton.setText("Reload");
	mk_ResetParametersButton.setIcon(QIcon(":/icons/reload.png"));
	connect(&mk_ResetParametersButton, SIGNAL(clicked()), this, SLOT(resetParameters()));
	lk_HLayout_->addWidget(&mk_ResetParametersButton);
	*/
	
	/*
	mk_AbortScriptButton.setText("&Abort");
	mk_AbortScriptButton.setIcon(QIcon(":/icons/clanbomber.png"));
	mk_AbortScriptButton.setShortcut(QKeySequence("Alt+A"));
	connect(&mk_AbortScriptButton, SIGNAL(clicked()), this, SLOT(abortScript()));
	lk_HLayout_->addWidget(&mk_AbortScriptButton);

	mk_StartButton.setText("&Start");
	mk_StartButton.setIcon(QIcon(":/icons/apply.png"));
	mk_StartButton.setShortcut(QKeySequence("Alt+S"));
	connect(&mk_StartButton, SIGNAL(clicked()), this, SLOT(start()));
	lk_HLayout_->addWidget(&mk_StartButton);
	*/


	mk_MainLayout.addLayout(lk_HLayout_);
	resize(800, 640);
	
	toggleUi();
}


k_ScriptHelper::~k_ScriptHelper()
{
}


void k_ScriptHelper::dragEnterEvent(QDragEnterEvent* ak_Event_)
{
	ak_Event_->acceptProposedAction();
}


void k_ScriptHelper::dropEvent(QDropEvent* ak_Event_)
{
	ak_Event_->accept();
	foreach (QUrl lk_Url, ak_Event_->mimeData()->urls())
	{
		QString ls_Path = lk_Url.toLocalFile();
		if (ls_Path != "" && mk_pScript)
		{
			QFileInfo lk_FileInfo(ls_Path);
			if (lk_FileInfo.isDir())
			{
				if (mk_pScript)
					mk_pScript->setOutputDirectory(ls_Path);
			}
		}
	}
}


void k_ScriptHelper::setScript(QString as_Filename)
{
	reset();
	resetDialog();
	if (checkVersionChanged())
		return;
	
	mk_pProfileManager = RefPtr<k_ProfileManager>(NULL);
	mk_pScript = RefPtr<IScript>(NULL);

	mk_pScript = k_ScriptFactory::makeScript(as_Filename, mk_Proteomatic, true);
	
	if (mk_pScript)
	{
		mk_pProfileManager = RefPtr<k_ProfileManager>(new k_ProfileManager(mk_Proteomatic, mk_pScript.get_Pointer(), this));
		activateScript();
	}
	
	toggleUi();
}


void k_ScriptHelper::activateScript()
{
	if (mk_pScript)
	{
		QString ls_Text = "<b>" + mk_pScript->title() + "</b>";
		if (mk_pScript->description().length() > 0)
			ls_Text += "<br /><br />" + mk_pScript->description();
		mk_pScript->parameterWidget()->layout()->setContentsMargins(0, 0, 0, 0);
		connect(dynamic_cast<QObject*>(mk_pScript.get_Pointer()), SIGNAL(proposePrefixButtonClicked()), this, SLOT(proposePrefix()));
		//mk_UpperLayout_->insertWidget(0, mk_pScript->parameterWidget());
		//mk_HSplitter_->insertWidget(0, mk_pScript->parameterWidget());
		mk_ParameterLayout_->addWidget(mk_pScript->parameterWidget());
		mk_ScrollArea_->setVisible(true);
		mk_ScrollArea_->resize(300, 10);
		mk_HSplitter_->setStretchFactor(0, 1);
		mk_HSplitter_->setStretchFactor(1, 1);

		// create input file boxes
		mk_FileLists.clear();
		mk_FileListsRemoveButtons.clear();
		mk_pInputFilesContainer = RefPtr<QWidget>(NULL);

		if (!mk_pScript->inputGroupKeys().empty())
		{
			mk_pInputFilesContainer = RefPtr<QWidget>(new QWidget(this));
			mk_pInputFilesContainer->setContentsMargins(0, 0, 0, 8);

			mk_VSplitter_->insertWidget(0, mk_pInputFilesContainer.get_Pointer());
			
			QSet<QString> lk_AvailableInputGroups = mk_pScript->inputGroupKeys().toSet();
			QStringList lk_GroupKeys;
			bool lb_HasAmbiguousInputGroups = !mk_pScript->ambiguousInputGroups().empty();
			foreach (QString ls_Key, mk_pScript->ambiguousInputGroups())
			{
				lk_GroupKeys.push_back(ls_Key);
				lk_AvailableInputGroups.remove(ls_Key);
			}
			if (!((lb_HasAmbiguousInputGroups) && (lk_AvailableInputGroups.empty())))
			{
				// if it's not like (we have amibiguous input groups and nothing's left), add the
				// remaining input files group
				lk_GroupKeys.push_back("");
			}
			
			QBoxLayout* lk_VLayout_ = new QVBoxLayout(mk_pInputFilesContainer.get_Pointer());
			lk_VLayout_->setMargin(0);
			foreach (QString ls_Key, lk_GroupKeys)
			{
				QBoxLayout* lk_GroupBoxLayout_ = new QHBoxLayout(NULL);
				lk_GroupBoxLayout_->setMargin(0);
				QString ls_GroupLabel = lb_HasAmbiguousInputGroups ? "Remaining input files" : "Input files";
				if (ls_Key != "")
					ls_GroupLabel = mk_pScript->inputGroupLabel(ls_Key);
				
				QLabel* lk_Label_ = new QLabel("<b>" + ls_GroupLabel + "</b>", mk_pInputFilesContainer.get_Pointer());
				lk_VLayout_->addWidget(lk_Label_);

				mk_FileLists[ls_Key] = RefPtr<k_FileList>(new k_FileList(NULL, true, true));
				lk_GroupBoxLayout_->addWidget(mk_FileLists[ls_Key].get_Pointer());
				QToolButton* lk_AddFilesButton_ = new QToolButton(mk_pInputFilesContainer.get_Pointer());
				lk_AddFilesButton_->setProperty("group", QVariant(ls_Key));
				lk_AddFilesButton_->setIcon(QIcon(":/icons/document-open.png"));
				connect(lk_AddFilesButton_, SIGNAL(clicked()), this, SLOT(loadFilesButtonClicked()));
				QToolButton* lk_RemoveFilesButton_ = new QToolButton(mk_pInputFilesContainer.get_Pointer());
				mk_FileListsRemoveButtons[ls_Key] = lk_RemoveFilesButton_;
				lk_RemoveFilesButton_->setIcon(QIcon(":/icons/list-remove.png"));
				connect(lk_RemoveFilesButton_, SIGNAL(clicked()), mk_FileLists[ls_Key].get_Pointer(), SLOT(removeSelection()));
				connect(mk_FileLists[ls_Key].get_Pointer(), SIGNAL(itemSelectionChanged()), this, SLOT(toggleUi()));
				QBoxLayout* lk_SubLayout_ = new QVBoxLayout();
				lk_SubLayout_->addWidget(lk_AddFilesButton_);
				lk_SubLayout_->addWidget(lk_RemoveFilesButton_);
				lk_SubLayout_->addStretch();
				lk_GroupBoxLayout_->addLayout(lk_SubLayout_);
				lk_VLayout_->addLayout(lk_GroupBoxLayout_);
			}
		}

		mk_VSplitter_->setStretchFactor(0, 3);
		mk_VSplitter_->setStretchFactor(1, 6);
		resetParameters();
		
		if (mk_pScript->location() == r_ScriptLocation::Local)
		{
			k_LocalScript* lk_LocalScript_ = dynamic_cast<k_LocalScript*>(mk_pScript.get_Pointer());
			connect(lk_LocalScript_, SIGNAL(scriptStarted()), this, SLOT(processStarted()));
			connect(lk_LocalScript_, SIGNAL(scriptFinished(int)), this, SLOT(processFinished(int)));
			connect(lk_LocalScript_, SIGNAL(readyRead()), this, SLOT(processReadyRead()));
		}
	
		setAcceptDrops(true);
	}
	else
		mk_pScript = RefPtr<IScript>(NULL);
	
	toggleUi();
}


void k_ScriptHelper::start()
{
	if (!mk_pScript)
		return;

	if (mk_pScript->status() == r_ScriptStatus::Running)
		return;

	if (checkVersionChanged())
		return;

	ms_Output.clear();
	mk_Output.clear();

	QStringList lk_Files;

	// first add general files
	if (mk_FileLists.contains(""))
		lk_Files += mk_FileLists[""]->files();
	
	// then add assigned files
	foreach (QString ls_GroupKey, mk_FileLists.keys())
	{
		if (!ls_GroupKey.isEmpty())
		{
			QStringList lk_Filenames = mk_FileLists[ls_GroupKey]->files();
			if (!lk_Filenames.empty())
			{
				lk_Files << "-" + ls_GroupKey;
				lk_Files += lk_Filenames;
			}
		}
	}

	if (mk_pScript->location() == r_ScriptLocation::Local)
		mk_pScript->start(lk_Files);
	//else
		//mk_RemoteRequests[mk_Proteomatic.queryRemoteHub(mk_pScript->uri(), (QStringList() << "---gui") + mk_pScript->commandLineArguments() + lk_Arguments)] = r_RemoteRequest(r_RemoteRequestType::SubmitJob);
}


void k_ScriptHelper::processStarted()
{
	addOutput(mk_pScript->title() + ":\n");
	processReadyRead();
	toggleUi();
}


void k_ScriptHelper::processFinished(int ai_ExitCode)
{
	processReadyRead();
	toggleUi();
	if (ai_ExitCode == 0)
	{
		addOutput("\n-----------------------------------\n");
		addOutput(QString("Script finished successfully.\n"));
	}
	else
	{
		addOutput("\n-----------------------------------\n");
		addOutput(QString("Script failed with exit code %1\n").arg(ai_ExitCode));
	}
}


void k_ScriptHelper::processReadyRead()
{
	addOutput(mk_pScript->readAll());
}


void k_ScriptHelper::addOutput(QString as_Text)
{
	ms_Output.append(as_Text);
	mk_Output.setText(ms_Output.text());
	mk_Output.moveCursor(QTextCursor::End);
	mk_Output.ensureCursorVisible();
}


void k_ScriptHelper::reset()
{
	setAcceptDrops(false);

	mk_pScript = RefPtr<IScript>(NULL);
	mk_pProfileManager = RefPtr<k_ProfileManager>(NULL);
	
	foreach (QString ls_GroupKey, mk_FileLists.keys())
		mk_FileLists[ls_GroupKey]->resetAll();

	ms_Output.clear();
	mk_Output.clear();
	
	toggleUi();
}


void k_ScriptHelper::parameterLabelClicked(const QString& as_Id)
{
	QMessageBox::about(this, "Click!", as_Id);
}


void k_ScriptHelper::loadFilesButtonClicked()
{
	QString ls_Group = sender()->property("group").toString();
	QStringList lk_Files = QFileDialog::getOpenFileNames(this, tr("Add files"), mk_Proteomatic.getConfiguration(CONFIG_REMEMBER_INPUT_FILES_PATH).toString(), tr("All files (*.*)"));
	QString ls_FirstPath = "";
	foreach (QString ls_Path, lk_Files)
	{
		if (ls_FirstPath.isEmpty())
			ls_FirstPath = ls_Path;
		mk_FileLists[ls_Group]->addInputFile(ls_Path);
	}
	if (!ls_FirstPath.isEmpty())
		mk_Proteomatic.getConfigurationRoot()[CONFIG_REMEMBER_INPUT_FILES_PATH] = QFileInfo(ls_FirstPath).absolutePath();
}


void k_ScriptHelper::resetParameters()
{
	foreach (QString ls_GroupKey, mk_FileLists.keys())
		mk_FileLists[ls_GroupKey]->resetAll();

	if (mk_pScript)
	{
		mk_pScript->reset();
		if (mk_FileLists.contains(""))
		{
			QSet<QString> lk_AmbiguousGroups = mk_pScript->ambiguousInputGroups().toSet();
			foreach (QString ls_Key, mk_pScript->inputGroupKeys())
				if (!lk_AmbiguousGroups.contains(ls_Key))
					mk_FileLists[""]->addInputFileGroup(ls_Key, mk_pScript->inputGroupLabel(ls_Key), mk_pScript->inputGroupExtensions(ls_Key));
		}
	}

	ms_Output.clear();
	mk_Output.clear();
	toggleUi();
}


void k_ScriptHelper::toggleUi()
{
	mk_ScrollArea_->setVisible(mk_pScript);
	this->setEnabled(mk_RemoteRequests.empty());

	bool lb_ProcessRunning = mk_pScript && mk_pScript->status() == r_ScriptStatus::Running;
	bool lb_RemoteScriptLoaded = mk_pScript && mk_pScript->location() == r_ScriptLocation::Remote;

	mk_ProfilesAction_->setEnabled(!lb_ProcessRunning);
	
	this->updateWindowTitle();
	
	//mk_CheckTicketAction_->setEnabled(mk_Proteomatic.remoteHub().isReady());

	mk_AbortAction_->setEnabled(lb_ProcessRunning);
	if (mk_pScript)
		mk_pScript->parameterWidget()->setEnabled(!lb_ProcessRunning);
	mk_LoadScriptButton_->setEnabled(!lb_ProcessRunning);
	if (lb_ProcessRunning)
	{
		mk_ResetAction_->setEnabled(false);
		mk_ReloadScriptAction_->setEnabled(false);
		mk_StartAction_->setEnabled(false);
		mk_CheckTicketAction_->setEnabled(false);
		mk_LoadParametersAction_->setEnabled(false);
		mk_SaveParametersAction_->setEnabled(false);
		if (mk_pInputFilesContainer)
			mk_pInputFilesContainer->setEnabled(false);
	}
	else
	{
		foreach (QString ls_Key, mk_FileListsRemoveButtons.keys())
			mk_FileListsRemoveButtons[ls_Key]->setEnabled(mk_FileLists[ls_Key]->selectedItems().count() != 0);
		mk_ResetAction_->setEnabled(mk_pScript);
		mk_ReloadScriptAction_->setEnabled(mk_pScript);
		mk_StartAction_->setEnabled(mk_pScript);
		mk_CheckTicketAction_->setEnabled(lb_RemoteScriptLoaded);
		mk_LoadParametersAction_->setEnabled(mk_pScript);
		mk_SaveParametersAction_->setEnabled(mk_pScript);
		if (mk_pInputFilesContainer)
			mk_pInputFilesContainer->setEnabled(mk_pScript);
	}

	if (mb_VersionChanged)
	{
		mk_AbortAction_->setEnabled(false);
		mk_LoadScriptButton_->setEnabled(false);
		mk_ProfilesAction_->setEnabled(false);
		mk_ResetAction_->setEnabled(false);
		mk_ReloadScriptAction_->setEnabled(false);
		mk_StartAction_->setEnabled(false);
		mk_CheckTicketAction_->setEnabled(false);
		mk_LoadParametersAction_->setEnabled(false);
		mk_SaveParametersAction_->setEnabled(mk_pScript);
		if (mk_pInputFilesContainer)
			mk_pInputFilesContainer->setEnabled(false);
	}
	if (mk_pScript && (!mk_VSplitter_->isVisible()))
	{
		mk_VSplitter_->show();
		mk_HSplitter_->setStretchFactor(0, 1);
		mk_HSplitter_->setStretchFactor(1, 1);
		mk_HSplitter_->setSizes(QList<int>() << mk_HSplitter_->width() / 2 << mk_HSplitter_->width() / 2);
	}
}


void k_ScriptHelper::remoteHubRequestFinished(int ai_SocketId, bool /*ab_Error*/, QString as_Response)
{
	if (mk_RemoteRequests.contains(ai_SocketId))
	{
		r_RemoteRequest lr_RemoteRequest = mk_RemoteRequests[ai_SocketId];
		mk_RemoteRequests.remove(ai_SocketId);

		if (lr_RemoteRequest.me_Type == r_RemoteRequestType::SubmitJob)
		{
			bool lb_Error = true;
			if (!as_Response.isEmpty() && as_Response.startsWith("command=submitJob\n"))
			{
				QRegExp lk_RegExp("\nticket=(.+)\n");
				if (lk_RegExp.indexIn(as_Response) > -1) 
				{
					QString ls_Ticket = lk_RegExp.cap(1);
					checkTicket(ls_Ticket);
					lb_Error = false;				
				}
			}
			
			if (lb_Error)
			{
				mk_Proteomatic.showMessageBox("Error", QString("There was an error while trying to submit your job.\n\n%1").arg(as_Response), ":/icons/dialog-warning.png");
			}
		}
	}
	toggleUi();
}


void k_ScriptHelper::remoteHubLineBatch(QStringList ak_Lines)
{
	foreach (QString ls_Line, ak_Lines)
	{
		QRegExp lk_RegExp("\\[---proteomaticProgress\\]start/(\\d+)");
		if (lk_RegExp.indexIn(ls_Line) > -1) 
		{
			QString ls_TotalSize = lk_RegExp.cap(1);
			bool lb_Ok;
			int li_TotalSize = QVariant(ls_TotalSize).toInt(&lb_Ok);
			if (lb_Ok)
			{
				mk_ProgressDialog_ = new QProgressDialog("Submitting files...", "Cancel", 0, li_TotalSize, this);
				mk_ProgressDialog_->setMinimumDuration(1000);
				mk_ProgressDialog_->setWindowModality(Qt::WindowModal);
				mk_ProgressDialog_->setValue(0);
				mk_ProgressDialog_->show();
			}
		}
		lk_RegExp = QRegExp("\\[---proteomaticProgress\\](\\d+)/(\\d+)");
		if (lk_RegExp.indexIn(ls_Line) > -1) 
		{
			QString ls_ProcessedSize = lk_RegExp.cap(1);
			bool lb_Ok;
			int li_ProcessedSize = QVariant(ls_ProcessedSize).toInt(&lb_Ok);
			if (lb_Ok && mk_ProgressDialog_)
				mk_ProgressDialog_->setValue(li_ProcessedSize);
		}
		if (ls_Line == "\\[---proteomaticProgress\\]finished" && mk_ProgressDialog_) 
			delete mk_ProgressDialog_;
	}
}


void k_ScriptHelper::checkTicket()
{
	RefPtr<QDialog> lk_pDialog(new QDialog(this));
	lk_pDialog->setWindowIcon(QIcon(":/icons/proteomatic.png"));
	lk_pDialog->setWindowTitle("Check ticket");
	QBoxLayout* lk_MainLayout_ = new QVBoxLayout(lk_pDialog.get_Pointer());
	QBoxLayout* lk_Layout_ = new QHBoxLayout();
	QBoxLayout* lk_SubLayout_ = new QVBoxLayout();
	QLabel* lk_Icon_ = new QLabel();
	lk_Icon_->setPixmap(QPixmap(":/icons/ticket.png"));
	lk_SubLayout_->addWidget(lk_Icon_);
	lk_SubLayout_->addStretch();
	lk_Layout_->addLayout(lk_SubLayout_);
	lk_SubLayout_ = new QHBoxLayout();
	lk_SubLayout_->addWidget(new QLabel("Ticket:", lk_pDialog.get_Pointer()));
	QLineEdit* lk_LineEdit_ = new QLineEdit(lk_pDialog.get_Pointer());
	lk_SubLayout_->addWidget(lk_LineEdit_);
	lk_Layout_->addLayout(lk_SubLayout_);
	lk_MainLayout_->addLayout(lk_Layout_);
	lk_Layout_ = new QHBoxLayout();
	lk_Layout_->addStretch();
	QPushButton* lk_CancelButton_ = new QPushButton(QIcon(":/icons/dialog-cancel.png"), "Cancel", lk_pDialog.get_Pointer());
	QPushButton* lk_CheckButton_ = new QPushButton(QIcon(":/icons/dialog-ok.png"), "Check", lk_pDialog.get_Pointer());
	lk_CheckButton_->setDefault(true);
	lk_CheckButton_->setAutoDefault(true);
	lk_Layout_->addWidget(lk_CancelButton_);
	lk_Layout_->addWidget(lk_CheckButton_);
	connect(lk_CancelButton_, SIGNAL(clicked()), lk_pDialog.get_Pointer(), SLOT(reject()));
	connect(lk_CheckButton_, SIGNAL(clicked()), lk_pDialog.get_Pointer(), SLOT(accept()));
	lk_MainLayout_->addLayout(lk_Layout_);
	lk_pDialog->resize(300, 10);
	if (lk_pDialog->exec() == QDialog::Accepted)
		checkTicket(lk_LineEdit_->text());
}


void k_ScriptHelper::checkTicket(QString as_Ticket)
{
	k_TicketWindow* lk_TicketWindow_ = new k_TicketWindow(mk_Proteomatic, mk_pScript->uri(), as_Ticket); 
	mk_TicketWindows[lk_TicketWindow_] = RefPtr<k_TicketWindow>(lk_TicketWindow_);
	connect(lk_TicketWindow_, SIGNAL(closed()), this, SLOT(ticketWindowClosed()));
}


void k_ScriptHelper::ticketWindowClosed()
{
	k_TicketWindow* lk_TicketWindow_ = dynamic_cast<k_TicketWindow*>(sender());
	if (mk_TicketWindows.contains(lk_TicketWindow_))
		mk_TicketWindows.remove(lk_TicketWindow_);
}


void k_ScriptHelper::abortScript()
{
	if (mk_pScript->status() != r_ScriptStatus::Running)
		return;

	if (mk_Proteomatic.showMessageBox("Abort script", "Are you sure you want to abort the current script?", ":/icons/dialog-warning.png", 
		QMessageBox::Yes | QMessageBox::No, QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
	{
		mk_pScript->kill();
		addOutput("\nScript aborted by user.");
	}
}


void k_ScriptHelper::resetDialog()
{
	QDialog* lk_Dialog_ = dynamic_cast<QDialog*>(sender());
	if (lk_Dialog_ == NULL)
		return;

	QListWidget* lk_ListWidget_ = lk_Dialog_->findChild<QListWidget*>();
	if (lk_ListWidget_ != NULL)
	{
		lk_ListWidget_->clearSelection();
		lk_ListWidget_->setFocus();
	}
}


void k_ScriptHelper::aboutDialog()
{
	QString ls_About = QString("<b>Proteomatic %1</b><br />Written in 2008 by Michael Specht.<br />Please contribute at <a href='http://proteomatic.origo.ethz.ch/'>http://proteomatic.origo.ethz.ch</a>.").arg(mk_Proteomatic.version());
	mk_Proteomatic.showMessageBox("About Proteomatic", ls_About, ":/icons/gpf-48.png", QMessageBox::Ok, QMessageBox::Ok, QMessageBox::Ok);
}


void k_ScriptHelper::scriptMenuScriptClicked(QAction* ak_Action_)
{
	setScript(ak_Action_->data().toString());
}


void k_ScriptHelper::scriptMenuChanged()
{
	mk_LoadScriptButton_->setMenu(mk_Proteomatic.proteomaticScriptsMenu());
	this->updateWindowTitle();
}


bool k_ScriptHelper::checkVersionChanged()
{
	if (mk_Proteomatic.versionChanged())
	{
		mb_VersionChanged = true;
		mk_Proteomatic.showMessageBox("Proteomatic Update", 
			"<b>Whoops!</b><p>The Proteomatic distribution you are running has been updated behind your back since you started Proteomatic.</p><p>As a result, you will not be able to run or load any scripts until you restart Proteomatic. Please consider saving your settings before exiting.</p>", 
			":/icons/face-monkey.png", 
			QMessageBox::Ok, QMessageBox::Ok, QMessageBox::Ok);
		toggleUi();
	}
	return mb_VersionChanged;
}


void k_ScriptHelper::updateWindowTitle()
{
	QString ls_ScriptsVersion = "(no scripts available)";
	if (mk_Proteomatic.scriptsVersion() != "")
	{
		ls_ScriptsVersion = "(using scripts " + mk_Proteomatic.scriptsVersion() + ")";
	}
	ms_WindowTitle = "Proteomatic " + mk_Proteomatic.version() + " " + ls_ScriptsVersion;
	if (mk_pScript)
		ms_WindowTitle = mk_pScript->title() + " - " + ms_WindowTitle;
	setWindowTitle(ms_WindowTitle);
}


void k_ScriptHelper::showProfileManager()
{
	mk_pProfileManager->reset();
	if (mk_pProfileManager->exec())
		mk_pScript->setConfiguration(mk_pProfileManager->getGoodProfileMix());
}


void k_ScriptHelper::proposePrefix()
{
	if (!mk_pScript)
		return;

	if (mk_pScript->status() == r_ScriptStatus::Running)
		return;

	QStringList lk_Files;

	foreach (QString ls_GroupKey, mk_FileLists.keys())
		lk_Files += mk_FileLists[ls_GroupKey]->files();
	
	QString ls_Result = mk_pScript->proposePrefix(lk_Files);
	if (!ls_Result.isEmpty())
		mk_pScript->setOutputFilePrefix(ls_Result);
	else
		mk_Proteomatic.showMessageBox("Propose prefix", 
			"<p>Sorry, but Proteomatic was unable to propose a prefix.</p>", 
			":/icons/emblem-important.png", QMessageBox::Ok, QMessageBox::Ok, QMessageBox::Ok);
}
