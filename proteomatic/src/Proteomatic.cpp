#include "Proteomatic.h"
#include "RefPtr.h"
#include "RubyWindow.h"


k_Proteomatic::k_Proteomatic(QString as_ApplicationPath)
	: ms_RubyPath("ruby")
	// TODO cache maybe? currently disabled
	, mb_SupressCache(true)
	, mk_MessageBoxParent_(NULL)
	, mk_RemoteMenu_(NULL)
	, ms_RemoteHubStdout("")
	, mk_RemoteHubHttp_(NULL)
	, ms_ScriptPath(as_ApplicationPath + "/scripts")
{
	QDir::setCurrent(as_ApplicationPath);

	if (QFile::exists("no.cache"))
		mb_SupressCache = true;

	QString ls_Version = syncRuby(QStringList() << "-v");
	// TODO: better version comparison (1.8.6 and up)
	if (!ls_Version.startsWith("ruby 1.8.6"))
	{
		if (QFile::exists("ruby.fallback"))
		{
			QFile lk_File("ruby.fallback");
			lk_File.open(QIODevice::ReadOnly);
			ms_RubyPath = QString(lk_File.readLine().trimmed());
			lk_File.close();
		}
	}
	
	// start remote hub
	QFileInfo lk_FileInfo("scripts/remote.rb");
	mk_pRemoteHubProcess = RefPtr<QProcess>(new QProcess());
	connect(mk_pRemoteHubProcess.get_Pointer(), SIGNAL(readyRead()), this, SLOT(remoteHubReadyReadSlot()));
	mk_pRemoteHubProcess->setWorkingDirectory(lk_FileInfo.absolutePath());
	mk_pRemoteHubProcess->setProcessChannelMode(QProcess::MergedChannels);
	mk_pRemoteHubProcess->start(ms_RubyPath, QStringList() << "remote.rb" << "--hub", QIODevice::ReadOnly | QIODevice::Unbuffered);
	
	QFontDatabase lk_FontDatabase;
	QStringList lk_Fonts = QStringList() << "Consolas" << "Bitstream Vera Sans Mono" << "Lucida Console" << "Courier New" << "Courier";
	while (!lk_Fonts.empty())
	{
		QString ls_Font = lk_Fonts.takeFirst();
		if (lk_FontDatabase.families().contains(ls_Font))
		{
			mk_ConsoleFont = QFont(ls_Font, 8);
			break;
		}
	}
	
	collectScriptInfo();
	createProteomaticScriptsMenu();
}


k_Proteomatic::~k_Proteomatic()
{
	mk_pRemoteHubProcess->kill();
}


QStringList k_Proteomatic::availableScripts()
{
	return mk_ScriptInfo.keys();
}


QHash<QString, QString> k_Proteomatic::scriptInfo(QString as_ScriptPath)
{
	return mk_ScriptInfo[as_ScriptPath];
}


QString k_Proteomatic::scriptInfo(QString as_ScriptPath, QString as_Key)
{
	return mk_ScriptInfo[as_ScriptPath][as_Key];
}


QMenu* k_Proteomatic::proteomaticScriptsMenu() const
{
	return mk_pProteomaticScriptsMenu.get_Pointer();
}


QString k_Proteomatic::syncRuby(QStringList ak_Arguments)
{
	QProcess lk_QueryProcess;

	QFileInfo lk_FileInfo(ak_Arguments.first());
	lk_QueryProcess.setWorkingDirectory(lk_FileInfo.absolutePath());
	lk_QueryProcess.setProcessChannelMode(QProcess::MergedChannels);
	lk_QueryProcess.start(ms_RubyPath, ak_Arguments, QIODevice::ReadOnly | QIODevice::Unbuffered);
	if (lk_QueryProcess.waitForFinished())
		return lk_QueryProcess.readAll();
	else
		return QString();
}


QString k_Proteomatic::rubyPath()
{
	return ms_RubyPath;
}


QString k_Proteomatic::version()
{
	return ms_Version;
}


bool k_Proteomatic::versionChanged() const
{
	QFile lk_VersionFile("scripts/include/version.rb");
	lk_VersionFile.open(QIODevice::ReadOnly);
	QString ls_Version = QString(lk_VersionFile.readAll().trimmed());
	lk_VersionFile.close();
	return ls_Version != ms_Version;
}


bool k_Proteomatic::supressCache() const
{
	return mb_SupressCache;
}


void k_Proteomatic::collectScriptInfo()
{
	QDir lk_Dir("scripts/");
	QStringList lk_Scripts = lk_Dir.entryList(QStringList() << "*.rb", QDir::Files);
	foreach (QString ls_Path, lk_Scripts)
	{
		if (ls_Path.contains(".defunct."))
			continue;
		ls_Path = lk_Dir.cleanPath(lk_Dir.absoluteFilePath(ls_Path));
		QString ls_Title = "";
		QString ls_Group = "";
		QString ls_Description = "";

		QFile lk_File(ls_Path);
		lk_File.open(QIODevice::ReadOnly | QIODevice::Unbuffered);
		if (lk_File.size() >= 29)
		{
			QString ls_Marker = QString(lk_File.read(29));
			if (ls_Marker == "require 'include/proteomatic'")
			{
				if (!mb_SupressCache && !QFile::exists("cache"))
				{
					QDir lk_Dir;
					lk_Dir.mkdir("cache");
				}

				QProcess lk_QueryProcess;

				QFileInfo lk_FileInfo(ls_Path);
				lk_QueryProcess.setWorkingDirectory(lk_FileInfo.absolutePath());
				QStringList lk_Arguments;
				lk_Arguments << ls_Path << "---info";
				lk_QueryProcess.setProcessChannelMode(QProcess::MergedChannels);
				lk_QueryProcess.start(ms_RubyPath, lk_Arguments, QIODevice::ReadOnly | QIODevice::Unbuffered);
				QString ls_Response;
				QString ls_CacheFilename = QString("cache/%1.info").arg(lk_FileInfo.baseName());
				if (QFile::exists(ls_CacheFilename) && !mb_SupressCache)
				{
					QFile lk_File(ls_CacheFilename);
					if (lk_File.open(QIODevice::ReadOnly))
					{
						ls_Response = lk_File.readAll();
						lk_File.close();
					}
				}
				else
				{
					if (lk_QueryProcess.waitForFinished())
					{
						ls_Response = lk_QueryProcess.readAll();
						if (!mb_SupressCache && !QFile::exists(ls_CacheFilename))
						{
							QFile lk_File(ls_CacheFilename);
							if (lk_File.open(QIODevice::WriteOnly))
							{
								QTextStream lk_Stream(&lk_File);
								lk_Stream << ls_Response;
								lk_Stream.flush();
								lk_File.close();
							}
						}
					}
				}
				if (ls_Response.length() > 0)
				{
					QStringList lk_Response = ls_Response.split(QChar('\n'));
					if (!lk_Response.empty() && lk_Response.takeFirst().trimmed() == "---info")
					{
						ls_Title = lk_Response.takeFirst().trimmed();
						ls_Group = lk_Response.takeFirst().trimmed();
						while (!lk_Response.empty())
						{
							if (!ls_Description.isEmpty())
								ls_Description += "\n";
							ls_Description += lk_Response.takeFirst().trimmed();
						}
					}
				}
			}
		}
		lk_File.close();

		if (ls_Title.length() > 0)
		{
			QHash<QString, QString> lk_Script;
			lk_Script["title"] = ls_Title;
			lk_Script["group"] = ls_Group;
			lk_Script["description"] = ls_Description;
			lk_Script["uri"] = ls_Path;
			mk_ScriptInfo.insert(ls_Path, lk_Script);
		}
	}

	QFile lk_VersionFile("scripts/include/version.rb");
	lk_VersionFile.open(QIODevice::ReadOnly);
	ms_Version = QString(lk_VersionFile.readAll().trimmed());
	lk_VersionFile.close();
}


void k_Proteomatic::createProteomaticScriptsMenu()
{
	QMenu* lk_Menu_ = new QMenu(NULL);
	QHash<QString, QMenu* > lk_GroupMenus;
	lk_GroupMenus[""] = lk_Menu_;

	QMap<QString, QString> lk_ScriptOrder; // title => script path
	QSet<QString> lk_Groups;
	QStringList lk_Scripts = availableScripts();
	foreach (QString ls_Path, lk_Scripts)
	{
		QHash<QString, QString> lk_ScriptInfo = scriptInfo(ls_Path);
		lk_ScriptOrder.insert(lk_ScriptInfo["title"], ls_Path);
		lk_Groups.insert(lk_ScriptInfo["group"]);
	}
	QList<QString> lk_GroupKeys = lk_Groups.toList();
	qSort(lk_GroupKeys);

	// create sub menus
	foreach (QString ls_Group, lk_GroupKeys)
	{
		QStringList lk_GroupElements = ls_Group.split("/");
		QString ls_IncPath = "";
		QMenu* lk_ParentMenu_ = lk_Menu_;
		foreach (QString ls_GroupElement, lk_GroupElements)
		{
			if (!ls_IncPath.isEmpty())
				ls_IncPath += "/";
			ls_IncPath += ls_GroupElement;
			if (!lk_GroupMenus.contains(ls_IncPath))
			{
				QMenu* lk_SubMenu_ = new QMenu(ls_GroupElement, lk_ParentMenu_);
				lk_SubMenu_->setIcon(QIcon(":/icons/folder.png"));
				lk_ParentMenu_->addMenu(lk_SubMenu_);
				lk_GroupMenus[ls_IncPath] = lk_SubMenu_;
			}
			lk_ParentMenu_ = lk_GroupMenus[ls_IncPath];
		}
	}
	
	mk_RemoteMenu_ = new QMenu("Remote", lk_Menu_);
	mk_RemoteMenu_->setIcon(QIcon(":/icons/applications-internet.png"));
	mk_RemoteMenu_->setEnabled(false);
	
	rebuildRemoteScriptsMenu();

	// insert menu entries
	foreach (QString ls_Title, lk_ScriptOrder.keys())
	{
		QString ls_Path = lk_ScriptOrder[ls_Title];
		QHash<QString, QString> lk_ScriptInfo = scriptInfo(ls_Path);
		QString ls_Group = lk_ScriptInfo["group"];
		QMenu* lk_TargetMenu_ = lk_GroupMenus[ls_Group];
		QAction* lk_Action_ = new QAction(QIcon(":/icons/proteomatic.png"), ls_Title, lk_TargetMenu_);
		lk_Action_->setData(lk_ScriptInfo["uri"]);
		lk_TargetMenu_->addAction(lk_Action_);
		connect(lk_Action_, SIGNAL(triggered()), this, SLOT(scriptMenuScriptClickedInternal()));
	}
	/* TODO: revive remote stuff */
    lk_Menu_->addSeparator();
   	lk_Menu_->addMenu(mk_RemoteMenu_);

	mk_pProteomaticScriptsMenu = RefPtr<QMenu>(lk_Menu_);
}


void k_Proteomatic::rememberScriptUri(QString as_Uri)
{
	QStringList lk_Uris = rememberedScriptUris();
	if (!lk_Uris.contains(as_Uri))
	{
		lk_Uris << as_Uri;
		QFile lk_File(QDir::homePath() + "/proteomatic.conf");
		if (lk_File.open(QIODevice::WriteOnly))
		{
			lk_File.write(lk_Uris.join("\n").toAscii());
			lk_File.close();
		}
	}
}


QStringList k_Proteomatic::rememberedScriptUris()
{
	QFile lk_File(QDir::homePath() + "/proteomatic.conf");
	if (lk_File.open(QIODevice::ReadOnly))
	{
		QStringList lk_Uris = QString(lk_File.readAll()).split("\n");
		lk_File.close();
		QStringList lk_Result;
		foreach (QString ls_Uri, lk_Uris)
			lk_Result << ls_Uri.trimmed();
		return lk_Result;
	}
	else 
		return QStringList();
}


int k_Proteomatic::queryRemoteHub(QString as_Uri, QStringList ak_Arguments)
{
	if (mk_RemoteHubHttp_ == NULL)
		return -1;
		
	QString ls_Arguments = QString("%1\r\n").arg(as_Uri);
	foreach (QString ls_Argument, ak_Arguments)
		ls_Arguments += QString("%1\r\n").arg(ls_Argument);
	
	return mk_RemoteHubHttp_->post("/", ls_Arguments.toAscii());
}


QFont& k_Proteomatic::consoleFont()
{
	return mk_ConsoleFont;
}


QString k_Proteomatic::scriptPath() const
{
	return ms_ScriptPath;
}


int k_Proteomatic::showMessageBox(QString as_Title, QString as_Text, QString as_Icon, 
								  QMessageBox::StandardButtons ae_Buttons, QMessageBox::StandardButton ae_DefaultButton, 
								  QMessageBox::StandardButton ae_EscapeButton)
{
	QMessageBox lk_MessageBox(mk_MessageBoxParent_);
	if (as_Icon != "")
		lk_MessageBox.setIconPixmap(QPixmap(as_Icon).scaledToWidth(48, Qt::SmoothTransformation));
	lk_MessageBox.setWindowIcon(QIcon(":/icons/proteomatic.png"));
	lk_MessageBox.setWindowTitle(as_Title);
	lk_MessageBox.setText(as_Text);
	lk_MessageBox.setStandardButtons(ae_Buttons);
	lk_MessageBox.setEscapeButton(ae_EscapeButton);
	lk_MessageBox.setDefaultButton(ae_DefaultButton);
	return lk_MessageBox.exec();
}


void k_Proteomatic::setMessageBoxParent(QWidget* ak_Widget_)
{
	mk_MessageBoxParent_ = ak_Widget_;
}


QWidget* k_Proteomatic::messageBoxParent() const
{
	return mk_MessageBoxParent_;
}


void k_Proteomatic::remoteHubReadyReadSlot()
{
	QString ls_Result = QString(mk_pRemoteHubProcess->readAll());
	
	ms_RemoteHubPortion += ls_Result;
	if (ms_RemoteHubPortion.contains(QChar('\n')))
	{
		QStringList lk_Lines = ms_RemoteHubPortion.split(QChar('\n'));
		ms_RemoteHubPortion = lk_Lines.takeLast();
		emit remoteHubLineBatch(lk_Lines);
	}
	
	// don't care about stdout if we already know our rubylicious remote hub
	if (mk_RemoteHubHttp_ != NULL)
		return;
		
	ms_RemoteHubStdout += ls_Result;
	QRegExp lk_RegExp("(REMOTE-HUB-PORT-START)(\\d+)(REMOTE-HUB-PORT-END)");
	if (lk_RegExp.indexIn(ms_RemoteHubStdout) > -1) 
	{
		QString ls_Port = lk_RegExp.cap(2);
		bool lb_Ok;
		int li_HubPort = QVariant(ls_Port).toInt(&lb_Ok);
		mk_RemoteHubHttp_ = new QHttp("127.0.0.1", li_HubPort);
		connect(mk_RemoteHubHttp_, SIGNAL(requestFinished(int, bool)), this, SLOT(remoteHubRequestFinishedSlot(int, bool)));
		mk_RemoteMenu_->setEnabled(true);
		mk_RemoteMenu_->setTitle("Remote");
		QStringList lk_Uris = rememberedScriptUris();
		foreach (QString ls_Uri, lk_Uris)
	 		mk_RemoteRequests[queryRemoteHub(ls_Uri, QStringList() << "---getInfoAndParameters")]
 				= r_RemoteRequest(r_RemoteRequestType::GetInfoAndParameters, ls_Uri);
 				
		emit remoteHubReady();
	}
}


void k_Proteomatic::scriptMenuScriptClickedInternal()
{
	QAction* lk_Action_ = dynamic_cast<QAction*>(sender());
	if (lk_Action_ != NULL)
		emit scriptMenuScriptClicked(lk_Action_);
}


void k_Proteomatic::addRemoteScriptDialog()
{
	RefPtr<QDialog> lk_pDialog(new QDialog(mk_MessageBoxParent_));
	lk_pDialog->setWindowIcon(QIcon(":/icons/proteomatic.png"));
	lk_pDialog->setWindowTitle("Add remote script");
	QBoxLayout* lk_MainLayout_ = new QVBoxLayout(lk_pDialog.get_Pointer());
	QBoxLayout* lk_Layout_ = new QHBoxLayout(lk_pDialog.get_Pointer());
	QBoxLayout* lk_SubLayout_ = new QVBoxLayout(lk_pDialog.get_Pointer());
	QLabel* lk_Icon_ = new QLabel();
	lk_Icon_->setPixmap(QPixmap(":/icons/applications-internet.png"));
	lk_SubLayout_->addWidget(lk_Icon_);
	lk_SubLayout_->addStretch();
	lk_Layout_->addLayout(lk_SubLayout_);
	lk_SubLayout_ = new QVBoxLayout(lk_pDialog.get_Pointer());
	lk_SubLayout_->addWidget(new QLabel("Remote script URI:", lk_pDialog.get_Pointer()));
	QLineEdit* lk_LineEdit_ = new QLineEdit(lk_pDialog.get_Pointer());
	QCheckBox* lk_Remember_ = new QCheckBox("Remember this remote script", lk_pDialog.get_Pointer());
	lk_Remember_->setCheckState(Qt::Checked);
	lk_SubLayout_->addWidget(lk_LineEdit_);
	lk_SubLayout_->addWidget(lk_Remember_);
	lk_SubLayout_->addStretch();
	lk_Layout_->addLayout(lk_SubLayout_);
	lk_MainLayout_->addLayout(lk_Layout_);
	lk_Layout_ = new QHBoxLayout(lk_pDialog.get_Pointer());
	lk_Layout_->addStretch();
	QPushButton* lk_CancelButton_ = new QPushButton(QIcon(":/icons/dialog-cancel.png"), "Cancel", lk_pDialog.get_Pointer());
	QPushButton* lk_AddButton_ = new QPushButton(QIcon(":/icons/list-add.png"), "Add", lk_pDialog.get_Pointer());
	lk_AddButton_->setDefault(true);
	lk_AddButton_->setAutoDefault(true);
	lk_Layout_->addWidget(lk_CancelButton_);
	lk_Layout_->addWidget(lk_AddButton_);
	connect(lk_CancelButton_, SIGNAL(clicked()), lk_pDialog.get_Pointer(), SLOT(reject()));
	connect(lk_AddButton_, SIGNAL(clicked()), lk_pDialog.get_Pointer(), SLOT(accept()));
	lk_MainLayout_->addLayout(lk_Layout_);
	lk_pDialog->setLayout(lk_MainLayout_);
	lk_pDialog->resize(300, 10);
	if (lk_pDialog->exec() == QDialog::Accepted)
	{
		QString ls_Uri = lk_LineEdit_->text();
		
		QStringList lk_AdditionalInfo;
		lk_AdditionalInfo << "feedback";
		if (lk_Remember_->checkState() == Qt::Checked)
			lk_AdditionalInfo << "remember";
			
 		mk_RemoteRequests[queryRemoteHub(ls_Uri, QStringList() << "---getInfoAndParameters")]
 			= r_RemoteRequest(r_RemoteRequestType::GetInfoAndParameters, ls_Uri, lk_AdditionalInfo);
 	}
}


void k_Proteomatic::remoteHubRequestFinishedSlot(int ai_SocketId, bool ab_Error)
{
	QString ls_Response = QString(mk_RemoteHubHttp_->readAll());
	
	if (mk_RemoteRequests.contains(ai_SocketId))
	{
		r_RemoteRequest lr_RemoteRequest = mk_RemoteRequests[ai_SocketId];
		mk_RemoteRequests.remove(ai_SocketId);
		
		if (ls_Response.startsWith("error"))
		{
			if (lr_RemoteRequest.mk_AdditionalInfo.contains("feedback"))
				showMessageBox("Remote hub response", ls_Response, ":/icons/dialog-warning.png", QMessageBox::Ok, QMessageBox::Ok, QMessageBox::Ok);
		}
		else
		{
			QString ls_Uri = lr_RemoteRequest.ms_Info;
			QString ls_Splitter = "---getParameters";
			QStringList lk_InfoAndParameters = ls_Response.split(ls_Splitter);
			QString ls_Info = lk_InfoAndParameters[0];
			QString ls_Parameters = ls_Splitter + lk_InfoAndParameters[1];
			QStringList lk_Response = ls_Info.split("\n");
			if (lk_Response.takeFirst().trimmed() == "---info")
			{
				QString ls_Title = "";
				QString ls_Group = "";
				QString ls_Description = "";
				ls_Title = lk_Response.takeFirst().trimmed();
				ls_Group = lk_Response.takeFirst().trimmed();
				while (!lk_Response.empty())
				{
					if (!ls_Description.isEmpty())
						ls_Description += "\n";
					ls_Description += lk_Response.takeFirst().trimmed();
				}
				
				QString ls_Host = ls_Uri;
				ls_Host = ls_Host.replace("druby://", "").split(":").first();
				
				if (!mk_ScriptInfo.contains(ls_Uri))
				{
					QHash<QString, QString> lk_Script;
					lk_Script["title"] = ls_Title + " (" + ls_Host + ")";
					lk_Script["group"] = ls_Group;
					lk_Script["description"] = ls_Description;
					lk_Script["uri"] = ls_Uri;
					lk_Script["parameters"] = ls_Parameters;
					mk_ScriptInfo.insert(ls_Uri, lk_Script);
				}
					
				QString ls_Caption = ls_Title + " (" + ls_Host + ")";
				mk_RemoteScripts[ls_Caption] = ls_Uri;
				rebuildRemoteScriptsMenu();
				if (lr_RemoteRequest.mk_AdditionalInfo.contains("remember"))
					rememberScriptUri(ls_Uri);
				if (lr_RemoteRequest.mk_AdditionalInfo.contains("feedback"))
					showMessageBox("Remote script added", "Successfully added " + ls_Caption + ".", ":/icons/applications-internet.png");
			}
		}
	}
	else
		emit remoteHubRequestFinished(ai_SocketId, ab_Error, ls_Response);
}


void k_Proteomatic::rebuildRemoteScriptsMenu()
{
	if (mk_RemoteMenu_ == NULL)
		return;
	
	mk_RemoteMenu_->clear();
	
	foreach (QString ls_Title, mk_RemoteScripts.keys())
	{
		QAction* lk_Action_ = new QAction(QIcon(":/icons/proteomatic.png"), ls_Title, mk_RemoteMenu_);
		lk_Action_->setData(mk_RemoteScripts[ls_Title]);
		connect(lk_Action_, SIGNAL(triggered()), this, SLOT(scriptMenuScriptClickedInternal()));
		mk_RemoteMenu_->addAction(lk_Action_);
	}
	
	mk_RemoteMenu_->addSeparator();
	QAction* lk_Action_ = new QAction(QIcon(":/icons/list-add.png"), "Add remote script...", mk_RemoteMenu_);
	connect(lk_Action_, SIGNAL(triggered()), this, SLOT(addRemoteScriptDialog()));
	lk_Action_->setData("druby-add-remote-script");
	mk_RemoteMenu_->addAction(lk_Action_);
}
