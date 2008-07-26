#include "ScriptHelper.h"
#include "ScriptFactory.h"
#include "ClickableLabel.h"
#include "CiListWidgetItem.h"
#include "Proteomatic.h"
#include "Script.h"
#include "LocalScript.h"
#include "RemoteScript.h"
#include "TicketWindow.h"
#include <limits.h>
#include <float.h>


k_ScriptHelper::k_ScriptHelper(QWidget* ak_Parent_, k_Proteomatic& ak_Proteomatic)
	: QMainWindow(ak_Parent_)
	, mk_FileList(NULL, true)
	, mk_MainLayout(this)
	, mb_VersionChanged(false)
	, mk_Proteomatic(ak_Proteomatic)
	, mk_Script_(NULL)
	, mk_ProfileManager_(NULL)
	, ms_WindowTitle("Proteomatic")
	, mk_ProgressDialog_(NULL)
{
	mk_Proteomatic.setMessageBoxParent(this);
	connect(&mk_Proteomatic, SIGNAL(remoteHubLineBatch(QStringList)), this, SLOT(remoteHubLineBatch(QStringList)));
	connect(&mk_Proteomatic, SIGNAL(remoteHubRequestFinished(int, bool, QString)), this, SLOT(remoteHubRequestFinished(int, bool, QString)));
	setWindowIcon(QIcon(":/icons/proteomatic.png"));
	ms_WindowTitle = "Proteomatic " + mk_Proteomatic.version();
	setWindowTitle(ms_WindowTitle);
	
	QWidget* lk_UpperLayoutWidget_ = new QWidget(this);
	QWidget* lk_LowerLayoutWidget_ = new QWidget(this);
	mk_UpperLayout_ = new QVBoxLayout(lk_UpperLayoutWidget_);
	mk_LowerLayout_ = new QVBoxLayout(lk_LowerLayoutWidget_);
	mk_UpperLayout_->setContentsMargins(0, 0, 0, 8);
	mk_LowerLayout_->setContentsMargins(0, 0, 0, 0);

	mk_VSplitter_ = new QSplitter(this);
	mk_VSplitter_->setStyle(new QPlastiqueStyle());
	mk_VSplitter_->setOrientation(Qt::Vertical);
	mk_VSplitter_->setHandleWidth(4);
	mk_HSplitter_ = new QSplitter(this);
	mk_HSplitter_->setStyle(new QPlastiqueStyle());
	mk_HSplitter_->setOrientation(Qt::Horizontal);
	mk_HSplitter_->setHandleWidth(4);

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
	QFrame* lk_Frame_;
	QWidget* lk_Container_;
	QBoxLayout* lk_GroupBoxLayout_;
	

	mk_ScriptMenu_ = mk_Proteomatic.proteomaticScriptsMenu();
	
	QToolBar* lk_ToolBar_ = new QToolBar(this);
	lk_ToolBar_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

	mk_LoadScriptButton_ = new QToolButton(lk_ToolBar_);
	mk_LoadScriptButton_->setIcon(QIcon(":/icons/document-open.png"));
	mk_LoadScriptButton_->setText("Load script");
	mk_LoadScriptButton_->setMenu(mk_ScriptMenu_);
	mk_LoadScriptButton_->setPopupMode(QToolButton::InstantPopup);
	mk_LoadScriptButton_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	lk_ToolBar_->addWidget(mk_LoadScriptButton_);
	
	mk_ProfilesAction_ = lk_ToolBar_->addAction(QIcon(":/icons/preferences-system.png"), "Profiles");
	
	//connect(mk_LoadScriptButton_, SIGNAL(clicked()), this, SLOT(showScriptMenu()));
	connect(&mk_Proteomatic, SIGNAL(scriptMenuScriptClicked(QAction*)), this, SLOT(scriptMenuScriptClicked(QAction*)));
	mk_ReloadScriptAction_ = lk_ToolBar_->addAction(QIcon(":/icons/edit-clear.png"), "Reset", this, SLOT(resetParameters()));
	mk_ReloadScriptAction_->setToolTip("Reset all manual settings.");

	
	lk_ToolBar_->addSeparator();
	
	mk_StartAction_ = lk_ToolBar_->addAction(QIcon(":/icons/dialog-ok.png"), "Start", this, SLOT(start()));
	mk_AbortAction_ = lk_ToolBar_->addAction(QIcon(":/icons/dialog-cancel.png"), "Abort", this, SLOT(abortScript()));
	
	// TODO: revive remote stuff
	//mk_CheckTicketAction_ = new QAction(this);
	mk_CheckTicketAction_ = lk_ToolBar_->addAction(QIcon(":/icons/ticket.png"), "&Check ticket");
	connect(mk_CheckTicketAction_, SIGNAL(triggered()), this, SLOT(checkTicket()));
	mk_CheckTicketAction_->setEnabled(false);
	
	addToolBar(Qt::TopToolBarArea, lk_ToolBar_);

	//connect(&mk_LoadScriptButton, SIGNAL(clicked()), this, SLOT(showScriptMenu()));

	QLabel* lk_Label_ = new QLabel("<b>Input files</b>", this);
	mk_UpperLayout_->addWidget(lk_Label_);

	lk_Container_ = new QWidget(this);
	lk_Label_->setBuddy(lk_Container_);
	lk_Container_->setContentsMargins(0, 0, 0, 0);
	lk_GroupBoxLayout_ = new QHBoxLayout(lk_Container_);
	lk_GroupBoxLayout_->setMargin(0);
	lk_GroupBoxLayout_->addWidget(&mk_FileList);
	mk_AddFilesButton.setIcon(QIcon(":/icons/document-open.png"));
	connect(&mk_AddFilesButton, SIGNAL(clicked()), this, SLOT(loadFilesButtonClicked()));
	mk_RemoveInputFileButton.setIcon(QIcon(":/icons/list-remove.png"));
	connect(&mk_RemoveInputFileButton, SIGNAL(clicked()), &mk_FileList, SLOT(removeSelection()));
	connect(&mk_FileList, SIGNAL(itemSelectionChanged()), this, SLOT(toggleUi()));
	QBoxLayout* lk_SubLayout_ = new QVBoxLayout(this);
	lk_SubLayout_->addWidget(&mk_AddFilesButton);
	lk_SubLayout_->addWidget(&mk_RemoveInputFileButton);
	lk_SubLayout_->addStretch();
	lk_GroupBoxLayout_->addLayout(lk_SubLayout_);
	lk_Container_->setLayout(lk_GroupBoxLayout_);
	mk_UpperLayout_->addWidget(lk_Container_);

	lk_UpperLayoutWidget_->setLayout(mk_UpperLayout_);
	mk_VSplitter_->addWidget(lk_UpperLayoutWidget_);
	
	lk_Label_ = new QLabel("<b>Output</b>", this);
	mk_LowerLayout_->addWidget(lk_Label_);

	lk_Container_ = new QWidget(this);
	lk_Label_->setBuddy(lk_Container_);
	lk_Container_->setContentsMargins(0, 0, 0, 0);
	lk_GroupBoxLayout_ = new QVBoxLayout(lk_Container_);
	lk_GroupBoxLayout_->setMargin(0);
	lk_GroupBoxLayout_->addWidget(&mk_Output);
	lk_Container_->setLayout(lk_GroupBoxLayout_);
	mk_LowerLayout_->addWidget(lk_Container_);

	lk_LowerLayoutWidget_->setLayout(mk_LowerLayout_);
	mk_VSplitter_->addWidget(lk_LowerLayoutWidget_);

	mk_VSplitter_->setChildrenCollapsible(false);
	mk_VSplitter_->setStretchFactor(0, 1);
	mk_VSplitter_->setStretchFactor(1, 3);
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
	mk_ParameterLayout_ = new QVBoxLayout(this);
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
	if (mk_ProfileManager_)
		delete mk_ProfileManager_;
	mk_ProfileManager_ = NULL;
	
	if (mk_Script_)
		delete mk_Script_;
	mk_Script_ = NULL;
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
		if (ls_Path != "" && mk_Script_)
		{
			QFileInfo lk_FileInfo(ls_Path);
			if (!lk_FileInfo.isDir())
				addInputFile(ls_Path);
		}
	}
}


void k_ScriptHelper::setScript(QString as_Filename)
{
	reset();
	resetDialog();
	if (checkVersionChanged())
		return;

	if (mk_ProfileManager_)
		delete mk_ProfileManager_;
	mk_ProfileManager_ = NULL;
	
	if (mk_Script_)
		delete mk_Script_;
	mk_Script_ = NULL;
	
	mk_Script_ = k_ScriptFactory::makeScript(as_Filename, mk_Proteomatic, true);
	mk_ProfileManager_ = new k_ProfileManager(mk_Proteomatic.messageBoxParent());
		
	activateScript();
	
	toggleUi();
}


void k_ScriptHelper::activateScript()
{
	if (mk_Script_->isGood())
	{
		QString ls_Text = "<b>" + mk_Script_->title() + "</b>";
		if (mk_Script_->description().length() > 0)
			ls_Text += "<br /><br />" + mk_Script_->description();
		mk_Script_->parameterWidget()->layout()->setContentsMargins(0, 0, 0, 0);
		connect(mk_Script_->parameterWidget(), SIGNAL(widgetResized()), this, SLOT(parameterWidgetResized()));
		connect(mk_ProfilesAction_, SIGNAL(triggered()), mk_ProfileManager_, SLOT(exec()));
		//mk_UpperLayout_->insertWidget(0, mk_Script_->parameterWidget());
		//mk_HSplitter_->insertWidget(0, mk_Script_->parameterWidget());
		mk_ParameterLayout_->addWidget(mk_Script_->parameterWidget());
		mk_ScrollArea_->setVisible(true);
		mk_ScrollArea_->resize(300, 10);
		mk_HSplitter_->setStretchFactor(0, 1);
		mk_HSplitter_->setStretchFactor(1, 1);
	
		if (mk_Script_->type() == r_ScriptType::Local)
		{
			k_LocalScript* lk_LocalScript_ = dynamic_cast<k_LocalScript*>(mk_Script_);
			connect(lk_LocalScript_, SIGNAL(started()), this, SLOT(processStarted()));
			connect(lk_LocalScript_, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(processFinished(int, QProcess::ExitStatus)));
			connect(lk_LocalScript_, SIGNAL(readyRead()), this, SLOT(processReadyRead()));
		}
	
		setAcceptDrops(true);
	}
	else
	{
		if (mk_Script_)
			delete mk_Script_;
		mk_Script_ = NULL;
	}
	
	toggleUi();
}


void k_ScriptHelper::start()
{
	if (!mk_Script_)
		return;

	if (mk_Script_->running())
		return;

	if (checkVersionChanged())
		return;

	ms_Output.clear();
	mk_Output.clear();

	QStringList lk_Arguments;

	for (int i = 0; i < mk_FileList.count(); ++i)
		lk_Arguments.push_back(mk_FileList.item(i)->text());

	if (mk_Script_->type() == r_ScriptType::Local)
		mk_Script_->start(lk_Arguments);
	else
		mk_RemoteRequests[mk_Proteomatic.queryRemoteHub(mk_Script_->uri(), (QStringList() << "---gui") + mk_Script_->commandLineArguments() + lk_Arguments)] = r_RemoteRequest(r_RemoteRequestType::SubmitJob);
}


void k_ScriptHelper::processStarted()
{
	addOutput(mk_Script_->title() + ":\n");
	processReadyRead();
	toggleUi();
}


void k_ScriptHelper::processFinished(int ai_ExitCode, QProcess::ExitStatus ak_ExitStatus)
{
	processReadyRead();
	toggleUi();
	if (ai_ExitCode != 0)
	{
		addOutput("\n-----------------------------------\n");
		addOutput(QString("Process failed with exit code %1\n").arg(ai_ExitCode));
	}
}


void k_ScriptHelper::processReadyRead()
{
	addOutput(mk_Script_->readAll());
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
	if (mk_Script_)
		delete mk_Script_;
	mk_Script_ = NULL;
	//mk_Program.setText("(no script loaded)");
	mk_FileList.clear();
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
	QStringList lk_Files = QFileDialog::getOpenFileNames(this, tr("Add files"), QDir::homePath(), tr("All files (*.*)"));
	foreach (QString ls_Path, lk_Files)
		addInputFile(ls_Path);
}


void k_ScriptHelper::resetParameters()
{
	if (mk_Script_)
		mk_Script_->reset();

	mk_FileList.clear();
	ms_Output.clear();
	mk_Output.clear();
	toggleUi();
}


void k_ScriptHelper::toggleUi()
{
	mk_ScrollArea_->setVisible(mk_Script_);
	this->setEnabled(mk_RemoteRequests.empty());
		
	bool lb_ProcessRunning = mk_Script_ && mk_Script_->running();
	bool lb_RemoteScriptLoaded = mk_Script_ && mk_Script_->type() == r_ScriptType::Remote;
	
	if (mk_Script_)
		setWindowTitle(mk_Script_->title() + " - " + ms_WindowTitle);
	else
		setWindowTitle(ms_WindowTitle);
	
	//mk_CheckTicketAction_->setEnabled(mk_Proteomatic.remoteHub().isReady());

	mk_AbortAction_->setEnabled(lb_ProcessRunning);
	if (mk_Script_)
		mk_Script_->parameterWidget()->setEnabled(!lb_ProcessRunning);
	mk_LoadScriptButton_->setEnabled(!lb_ProcessRunning);
	if (lb_ProcessRunning)
	{
		mk_RemoveInputFileButton.setEnabled(false);
		mk_ResetAction_->setEnabled(false);
		mk_ProfilesAction_->setEnabled(false);
		mk_ReloadScriptAction_->setEnabled(false);
		mk_StartAction_->setEnabled(false);
		mk_CheckTicketAction_->setEnabled(false);
		mk_LoadParametersAction_->setEnabled(false);
		mk_SaveParametersAction_->setEnabled(false);
		mk_AddFilesButton.setEnabled(false);
	}
	else
	{
		mk_RemoveInputFileButton.setEnabled(mk_FileList.selectedItems().count() != 0);
		mk_ProfilesAction_->setEnabled(mk_Script_ && mk_Script_->hasParameters());
		mk_ResetAction_->setEnabled(mk_Script_);
		mk_ReloadScriptAction_->setEnabled(mk_Script_);
		mk_StartAction_->setEnabled(mk_Script_);
		mk_CheckTicketAction_->setEnabled(lb_RemoteScriptLoaded);
		mk_LoadParametersAction_->setEnabled(mk_Script_);
		mk_SaveParametersAction_->setEnabled(mk_Script_);
		mk_AddFilesButton.setEnabled(mk_Script_);
	}

	if (mb_VersionChanged)
	{
		mk_RemoveInputFileButton.setEnabled(false);
		mk_AbortAction_->setEnabled(false);
		mk_LoadScriptButton_->setEnabled(false);
		mk_ProfilesAction_->setEnabled(false);
		mk_ResetAction_->setEnabled(false);
		mk_ReloadScriptAction_->setEnabled(false);
		mk_StartAction_->setEnabled(false);
		mk_CheckTicketAction_->setEnabled(false);
		mk_LoadParametersAction_->setEnabled(false);
		mk_SaveParametersAction_->setEnabled(mk_Script_);
		mk_AddFilesButton.setEnabled(false);
	}
}


void k_ScriptHelper::remoteHubRequestFinished(int ai_SocketId, bool ab_Error, QString as_Response)
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
	QBoxLayout* lk_Layout_ = new QHBoxLayout(lk_pDialog.get_Pointer());
	QBoxLayout* lk_SubLayout_ = new QVBoxLayout(lk_pDialog.get_Pointer());
	QLabel* lk_Icon_ = new QLabel();
	lk_Icon_->setPixmap(QPixmap(":/icons/ticket.png"));
	lk_SubLayout_->addWidget(lk_Icon_);
	lk_SubLayout_->addStretch();
	lk_Layout_->addLayout(lk_SubLayout_);
	lk_SubLayout_ = new QHBoxLayout(lk_pDialog.get_Pointer());
	lk_SubLayout_->addWidget(new QLabel("Ticket:", lk_pDialog.get_Pointer()));
	QLineEdit* lk_LineEdit_ = new QLineEdit(lk_pDialog.get_Pointer());
	lk_SubLayout_->addWidget(lk_LineEdit_);
	lk_Layout_->addLayout(lk_SubLayout_);
	lk_MainLayout_->addLayout(lk_Layout_);
	lk_Layout_ = new QHBoxLayout(lk_pDialog.get_Pointer());
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
	lk_pDialog->setLayout(lk_MainLayout_);
	lk_pDialog->resize(300, 10);
	if (lk_pDialog->exec() == QDialog::Accepted)
		checkTicket(lk_LineEdit_->text());
}


void k_ScriptHelper::checkTicket(QString as_Ticket)
{
	k_TicketWindow* lk_TicketWindow_ = new k_TicketWindow(mk_Proteomatic, mk_Script_->uri(), as_Ticket); 
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
	if (!mk_Script_->running())
		return;

	if (mk_Proteomatic.showMessageBox("Abort script", "Are you sure you want to abort the current script?", ":/icons/dialog-warning.png", 
		QMessageBox::Yes | QMessageBox::No, QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
	{
		mk_Script_->kill();
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


void k_ScriptHelper::addInputFile(QString as_Path)
{
	mk_FileList.addItem(as_Path);
	return;

/*
	if (mk_Program.text().isEmpty())
		return;
		*/

/*
	QProcess lk_QueryProcess;
	QFileInfo lk_FileInfo(mk_Program.text());
	lk_QueryProcess.setWorkingDirectory(lk_FileInfo.absolutePath());
	QStringList lk_Arguments;
	lk_Arguments << mk_Program.text() << "---detectInput" << as_Path;
	lk_QueryProcess.setProcessChannelMode(QProcess::MergedChannels);
	lk_QueryProcess.start(mk_Proteomatic.rubyPath(), lk_Arguments, QIODevice::ReadOnly | QIODevice::Unbuffered);
	if (lk_QueryProcess.waitForFinished())
	{
		QString ls_Response = lk_QueryProcess.readAll();
		QStringList lk_Response = ls_Response.split(QChar('\n'));
		if (!lk_Response.empty() && lk_Response.takeFirst().trimmed() == "---detectInput")
		{
			if (!lk_Response.empty() && !lk_Response.takeFirst().trimmed().isEmpty())
			{
				mk_FileList.addItem(as_Path);
			}
		}
	}
	*/
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


void k_ScriptHelper::parameterWidgetResized()
{
	/*
	int li_Width = mk_Script_->parameterWidget()->width();
	int li_Difference = mk_ScrollArea_->width() - li_Width;
	if (mk_ScrollArea_->verticalScrollBar())
		li_Difference -= mk_ScrollArea_->verticalScrollBar()->width();
	if (li_Difference < 0)
		mk_ScrollArea_->resize(mk_ScrollArea_->width() - li_Difference, mk_ScrollArea_->height());
		*/
		/*
	mk_Proteomatic.showMessageBox("Info", QString("width: %1, size hint: %2x%3")
		.arg(mk_Script_->parameterWidget()->width())
		.arg(mk_ScrollArea_->sizeHint().width())
		.arg(mk_ScrollArea_->sizeHint().height()));
		*/
	/*
	int li_Width = mk_Script_->parameterWidget()->width() + mk_ScrollArea_->verticalScrollBar()->width();
	if (mk_ScrollArea_->width() < li_Width)
		mk_ScrollArea_->setMinimumWidth(li_Width);
	*/
	/*
	if (mk_ScrollArea_->horizontalScrollBar()->isVisible())
	{
		int li_Width = mk_Script_->parameterWidget()->width() + mk_ScrollArea_->verticalScrollBar()->width();
		mk_ScrollArea_->setMinimumWidth(li_Width + 10);
	}
	*/
}


void k_ScriptHelper::createProfile()
{
	k_Script* lk_Script_ = k_ScriptFactory::makeScript(mk_Script_->uri(), mk_Proteomatic, false, true);
	QDialog lk_Dialog(this);
	lk_Dialog.setModal(true);
	QBoxLayout* lk_Layout_ = new QVBoxLayout(&lk_Dialog);
	QScrollArea* lk_ScrollArea_ = new QScrollArea(&lk_Dialog);
	lk_ScrollArea_->setWidgetResizable(true);
	lk_ScrollArea_->setFrameStyle(QFrame::NoFrame);
	lk_ScrollArea_->setWidget(lk_Script_->parameterWidget());
	lk_Layout_->addWidget(lk_ScrollArea_);
	lk_Dialog.setLayout(lk_Layout_);
	lk_Script_->parameterWidget()->resize(550, 10);
	lk_Dialog.exec();
}
