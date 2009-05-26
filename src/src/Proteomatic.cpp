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

#include "Proteomatic.h"
#include "RefPtr.h"
#include "RubyWindow.h"
#include "Yaml.h"
#include "version.h"


k_Proteomatic::k_Proteomatic(QString as_ApplicationPath)
	: mk_MessageBoxParent_(NULL)
	, mk_RemoteMenu_(NULL)
	, ms_RemoteHubStdout("")
	, ms_ScriptPath(as_ApplicationPath + "/scripts")
	, ms_ProgramConfigurationPath(as_ApplicationPath + "/proteomatic.conf.yaml")
	, ms_UserConfigurationPath(QDir::homePath() + "/proteomatic.conf.yaml")
{
	QDir::setCurrent(as_ApplicationPath);
	this->loadConfiguration();

	this->checkRuby();
	
	QFontDatabase lk_FontDatabase;
	QStringList lk_Fonts = QStringList() << "Consolas" << "Bitstream Vera Sans Mono" << "Lucida Console" << "Courier New" << "Courier";
	while (!lk_Fonts.empty())
	{
		QString ls_Font = lk_Fonts.takeFirst();
		if (lk_FontDatabase.families().contains(ls_Font))
		{
			mk_ConsoleFont = QFont(ls_Font);
			mk_ConsoleFont.setPointSizeF(mk_ConsoleFont.pointSizeF() * 0.8);
			break;
		}
	}
	
	// create scripts subdirectory if it doesn't exist
	if (!QFile::exists(ms_ScriptPath))
		QDir().mkdir(ms_ScriptPath);
		
	// determine currently used script package
	ms_ScriptPackage = findCurrentScriptPackage();
		
	collectScriptInfo();
	createProteomaticScriptsMenu();
}


k_Proteomatic::~k_Proteomatic()
{
	/*
	if (mk_RemoteMenu_)
	{
		delete mk_RemoteMenu_;
		mk_RemoteMenu_ = NULL;
	}
	*/
		
	// save configuration
	this->saveConfiguration();
	if (mk_pRemoteHubHttp.get_Pointer())
	{
		mk_pRemoteHubHttp->abort();
		mk_pRemoteHubHttp = RefPtr<QHttp>(NULL);
	}
	if (mk_pRemoteHubProcess.get_Pointer())
	{
		mk_pRemoteHubProcess->kill();
		mk_pRemoteHubProcess = RefPtr<QProcess>(NULL);
	}
}


void k_Proteomatic::checkForUpdates()
{
	if (!mk_Configuration[CONFIG_SCRIPTS_URL].toString().isEmpty())
	{
		QString ls_Result = this->syncRuby(QStringList() << QDir::currentPath() + "/helper/check-for-updates.rb" << mk_Configuration[CONFIG_SCRIPTS_URL].toString() << "--dryrun");
		if (ls_Result.startsWith("CURRENT-VERSION:"))
		{
			ls_Result.replace("CURRENT-VERSION:", "");
			QString ls_LatestVersion = ls_Result.replace(".tar.bz2", "").trimmed();
			QString ls_Version = ls_Result.replace(".tar.bz2", "").replace("proteomatic-scripts-", "").trimmed();
			QStringList lk_AvailableVersions = QDir(ms_ScriptPath).entryList(QDir::NoDotAndDotDot | QDir::AllDirs);
			QString ls_InstalledVersion = ms_ScriptPackage;
			ls_InstalledVersion.replace("proteomatic-scripts-", "").trimmed();
			if (ls_Version != ls_InstalledVersion)
			{
				if (this->showMessageBox("Online update", 
					QString("A new version of Proteomatic scripts is available.<br /> ") + 
					"Latest version: " + ls_Version + ", installed: " + ls_InstalledVersion + "<br />Do you want to update to the latest version?",
					":/icons/software-update-available.png", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
				{
					QStringList lk_Arguments;
					lk_Arguments = QStringList() << QDir::currentPath() + "/helper/check-for-updates.rb" << mk_Configuration[CONFIG_SCRIPTS_URL].toString() << "--outpath" << ms_ScriptPath;
					if (!ms_ScriptPackage.isEmpty())
						lk_Arguments << "--oldpath" << ms_ScriptPath + "/" + ms_ScriptPackage;
					k_RubyWindow lk_RubyWindow(*this, lk_Arguments, "Online update", ":/icons/software-update-available.png");
					lk_RubyWindow.exec();
					
					ms_ScriptPackage = ls_LatestVersion;
					QFile lk_File(ms_ScriptPath + "/which.txt");
					lk_File.open(QIODevice::WriteOnly);
					QTextStream lk_Stream(&lk_File);
					lk_Stream << ms_ScriptPackage;
					lk_File.close();
					
					// purge cache
					QStringList lk_CacheFiles = QDir("cache").entryList(QDir::Files);
					foreach (QString ls_Path, lk_CacheFiles)
						QFile(ls_Path).remove();

					this->collectScriptInfo();
					this->createProteomaticScriptsMenu();
				}
			}
		}
	}
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
	lk_QueryProcess.start(mk_Configuration[CONFIG_PATH_TO_RUBY].toString(), ak_Arguments, QIODevice::ReadOnly | QIODevice::Unbuffered);
	if (lk_QueryProcess.waitForFinished())
		return lk_QueryProcess.readAll();
	else
		return QString();
}


QString k_Proteomatic::version()
{
	return gs_ProteomaticVersion;
}


bool k_Proteomatic::versionChanged() const
{
	return false;
	/*
	QFile lk_VersionFile("scripts/include/version.rb");
	lk_VersionFile.open(QIODevice::ReadOnly);
	QString ls_Version = QString(lk_VersionFile.readAll().trimmed());
	lk_VersionFile.close();
	return ls_Version != ms_Version;
	*/
}


void k_Proteomatic::loadConfiguration()
{
	// can install program configuration as user configuration?
	if (QFile(ms_ProgramConfigurationPath).exists() && !QFile(ms_UserConfigurationPath).exists())
	{
		QFile lk_File(ms_ProgramConfigurationPath);
		lk_File.copy(ms_UserConfigurationPath);
	}
		
	// update program configuration with user configuration
	if (QFile(ms_UserConfigurationPath).exists())
		mk_Configuration = k_Yaml::parseFromFile(ms_UserConfigurationPath).toMap();
		
	// insert default values
	bool lb_InsertedDefaultValue = false;
	if (!mk_Configuration.contains(CONFIG_PATH_TO_RUBY) || mk_Configuration[CONFIG_PATH_TO_RUBY].type() != QVariant::String)
	{
		mk_Configuration[CONFIG_PATH_TO_RUBY] = "ruby";
		lb_InsertedDefaultValue = true;
	}
	if (!mk_Configuration.contains(CONFIG_REMOTE_SCRIPTS) || mk_Configuration[CONFIG_REMOTE_SCRIPTS].type() != QVariant::List)
	{
		mk_Configuration[CONFIG_REMOTE_SCRIPTS] = QList<QVariant>();
		lb_InsertedDefaultValue = true;
	}
	if (!mk_Configuration.contains(CONFIG_PROFILES) || mk_Configuration[CONFIG_PROFILES].type() != QVariant::Map)
	{
		mk_Configuration[CONFIG_PROFILES] = QMap<QString, QVariant>();
		lb_InsertedDefaultValue = true;
	}
	if (!mk_Configuration.contains(CONFIG_REMEMBER_PROFILE_PATH) || mk_Configuration[CONFIG_REMEMBER_PROFILE_PATH].type() != QVariant::String)
	{
		mk_Configuration[CONFIG_REMEMBER_PROFILE_PATH] = QDir::homePath();
		lb_InsertedDefaultValue = true;
	}
	if (!mk_Configuration.contains(CONFIG_REMEMBER_INPUT_FILES_PATH) || mk_Configuration[CONFIG_REMEMBER_INPUT_FILES_PATH].type() != QVariant::String)
	{
		mk_Configuration[CONFIG_REMEMBER_INPUT_FILES_PATH] = QDir::homePath();
		lb_InsertedDefaultValue = true;
	}
	if (!mk_Configuration.contains(CONFIG_REMEMBER_OUTPUT_PATH) || mk_Configuration[CONFIG_REMEMBER_OUTPUT_PATH].type() != QVariant::String)
	{
		mk_Configuration[CONFIG_REMEMBER_OUTPUT_PATH] = QDir::homePath();
		lb_InsertedDefaultValue = true;
	}
	if (!mk_Configuration.contains(CONFIG_SCRIPTS_URL) || mk_Configuration[CONFIG_SCRIPTS_URL].type() != QVariant::String)
	{
		mk_Configuration[CONFIG_SCRIPTS_URL] = "ftp://gpf.uni-muenster.de/download/proteomatic-scripts";
		lb_InsertedDefaultValue = true;
	}
	if (!mk_Configuration.contains(CONFIG_AUTO_CHECK_FOR_UPDATES) || mk_Configuration[CONFIG_AUTO_CHECK_FOR_UPDATES].type() != QVariant::String)
	{
		mk_Configuration[CONFIG_AUTO_CHECK_FOR_UPDATES] = true;
		lb_InsertedDefaultValue = true;
	}
	if (!mk_Configuration.contains(CONFIG_WARN_ABOUT_MIXED_PROFILES) || mk_Configuration[CONFIG_WARN_ABOUT_MIXED_PROFILES].type() != QVariant::String)
	{
		mk_Configuration[CONFIG_WARN_ABOUT_MIXED_PROFILES] = true;
		lb_InsertedDefaultValue = true;
	}
	if (!mk_Configuration.contains(CONFIG_CACHE_SCRIPT_INFO) || mk_Configuration[CONFIG_CACHE_SCRIPT_INFO].type() != QVariant::String)
	{
		mk_Configuration[CONFIG_CACHE_SCRIPT_INFO] = true;
		lb_InsertedDefaultValue = true;
	}
		
	// write user configuration if it doesn't already exist
	if (lb_InsertedDefaultValue)
		this->saveConfiguration();
}


void k_Proteomatic::collectScriptInfo()
{
	mk_ScriptInfo.clear();
	QDir lk_Dir(ms_ScriptPath + "/" + ms_ScriptPackage);
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
		lk_File.open(QIODevice::ReadOnly);
		QString ls_Marker;
		QTextStream lk_Stream(&lk_File);
		while (!lk_Stream.atEnd() && (ls_Marker.isEmpty() || ls_Marker.startsWith("#")))
			ls_Marker = lk_Stream.readLine().trimmed();
		lk_File.close();
			
		if (ls_Marker == "require 'include/proteomatic'" || ls_Marker == "require \"include/proteomatic\"")
		{
			if (getConfiguration(CONFIG_CACHE_SCRIPT_INFO).toBool() && (!QFile::exists("cache")))
			{
				QDir lk_Dir;
				lk_Dir.mkdir("cache");
			}

			QFileInfo lk_FileInfo(ls_Path);
			QString ls_Response;
			QString ls_CacheFilename = QString("cache/%1.info").arg(lk_FileInfo.baseName());
			bool lb_UseCache = getConfiguration(CONFIG_CACHE_SCRIPT_INFO).toBool() && fileUpToDate(ls_CacheFilename, QStringList() << ls_Path);
			
			// disable cache if we're trunk!
			if (gs_ProteomaticVersion == "trunk")
			  lb_UseCache = false;
			  
			if (lb_UseCache)
			{
				// see if cached info showed no errors or unresolved dependencies
				QFile lk_File(ls_CacheFilename);
				lk_File.open(QIODevice::ReadOnly);
				QTextStream lk_Stream(&lk_File);
				QString ls_Line = lk_Stream.readLine().trimmed();
				if (ls_Line != "---info")
					lb_UseCache = false;
			}
			if (lb_UseCache)
			{
				// re-use cached information
				QFile lk_File(ls_CacheFilename);
				if (lk_File.open(QIODevice::ReadOnly))
				{
					ls_Response = lk_File.readAll();
					lk_File.close();
				}
			}
			else
			{
				// retrieve information from script
				QProcess lk_QueryProcess;
				lk_QueryProcess.setWorkingDirectory(lk_FileInfo.absolutePath());
				QStringList lk_Arguments;
				lk_Arguments << ls_Path << "---info";
				lk_QueryProcess.setProcessChannelMode(QProcess::MergedChannels);
				lk_QueryProcess.start(mk_Configuration[CONFIG_PATH_TO_RUBY].toString(), lk_Arguments, QIODevice::ReadOnly | QIODevice::Unbuffered);
				if (lk_QueryProcess.waitForFinished())
				{
					ls_Response = lk_QueryProcess.readAll();
					if (getConfiguration(CONFIG_CACHE_SCRIPT_INFO).toBool())
					{
						// update cached information
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
}


void k_Proteomatic::createProteomaticScriptsMenu()
{
	mk_pProteomaticScriptsMenu = RefPtr<QMenu>(NULL);
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
    
    lk_Menu_->addSeparator();
   	lk_Menu_->addMenu(mk_RemoteMenu_);

	mk_pProteomaticScriptsMenu = RefPtr<QMenu>(lk_Menu_);
	emit scriptMenuChanged();

	ms_RemoteHubStdout = "";
	mk_pRemoteHubHttp = RefPtr<QHttp>(NULL);

	// (re-)start remote hub
	QFileInfo lk_FileInfo(ms_ScriptPath + "/" + ms_ScriptPackage + "/remote.rb");
	mk_pRemoteHubProcess = RefPtr<QProcess>(new QProcess());
	connect(mk_pRemoteHubProcess.get_Pointer(), SIGNAL(readyRead()), this, SLOT(remoteHubReadyReadSlot()));
	mk_pRemoteHubProcess->setWorkingDirectory(lk_FileInfo.absolutePath());
	mk_pRemoteHubProcess->setProcessChannelMode(QProcess::MergedChannels);
	mk_pRemoteHubProcess->start(mk_Configuration[CONFIG_PATH_TO_RUBY].toString(), QStringList() << "remote.rb" << "--hub", QIODevice::ReadOnly | QIODevice::Unbuffered);
}


int k_Proteomatic::queryRemoteHub(QString as_Uri, QStringList ak_Arguments)
{
	if (mk_pRemoteHubHttp.get_Pointer() == NULL)
		return -1;
		
	QString ls_Arguments = QString("%1\r\n").arg(as_Uri);
	foreach (QString ls_Argument, ak_Arguments)
		ls_Arguments += QString("%1\r\n").arg(ls_Argument);
	
	return mk_pRemoteHubHttp->post("/", ls_Arguments.toAscii());
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
	if (mk_pRemoteHubHttp.get_Pointer() != NULL)
		return;
		
	ms_RemoteHubStdout += ls_Result;
	QRegExp lk_RegExp("(REMOTE-HUB-PORT-START)(\\d+)(REMOTE-HUB-PORT-END)");
	if (lk_RegExp.indexIn(ms_RemoteHubStdout) > -1) 
	{
		QString ls_Port = lk_RegExp.cap(2);
		bool lb_Ok;
		int li_HubPort = QVariant(ls_Port).toInt(&lb_Ok);
		mk_pRemoteHubHttp = RefPtr<QHttp>(new QHttp("127.0.0.1", li_HubPort));
		connect(mk_pRemoteHubHttp.get_Pointer(), SIGNAL(requestFinished(int, bool)), this, SLOT(remoteHubRequestFinishedSlot(int, bool)));
		mk_RemoteMenu_->setEnabled(true);
		mk_RemoteMenu_->setTitle("Remote");
		QList<QVariant> lk_Uris = mk_Configuration[CONFIG_REMOTE_SCRIPTS].toList();
		foreach (QVariant lk_Uri, lk_Uris)
		{
			QString ls_Uri = lk_Uri.toString();
	 		mk_RemoteRequests[queryRemoteHub(ls_Uri, QStringList() << "---getInfoAndParameters")]
 				= r_RemoteRequest(r_RemoteRequestType::GetInfoAndParameters, ls_Uri);
 		}
 				
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
	QBoxLayout* lk_Layout_ = new QHBoxLayout();
	QBoxLayout* lk_SubLayout_ = new QVBoxLayout();
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
	QString ls_Response = QString(mk_pRemoteHubHttp->readAll());
	
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
				{
					QVariant lk_Uri = QVariant(ls_Uri);
					QList<QVariant> lk_RemoteScriptList = mk_Configuration[CONFIG_REMOTE_SCRIPTS].toList();
					if (!lk_RemoteScriptList.contains(lk_Uri))
					{
						lk_RemoteScriptList.push_back(lk_Uri);
						mk_Configuration[CONFIG_REMOTE_SCRIPTS] = lk_RemoteScriptList;
						this->saveConfiguration();
					}
				}
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


QVariant k_Proteomatic::getConfiguration(QString as_Key)
{
	return mk_Configuration[as_Key];
}


tk_YamlMap& k_Proteomatic::getConfigurationRoot()
{
	return mk_Configuration;
}


void k_Proteomatic::saveConfiguration()
{
	k_Yaml::emitToFile(mk_Configuration, ms_UserConfigurationPath);
}


QString k_Proteomatic::scriptsVersion()
{
	QString ls_Version = ms_ScriptPackage;
	return ls_Version.replace("proteomatic-scripts-", "").trimmed();
}


void k_Proteomatic::checkRuby()
{
	mk_CheckRubyDialog.setMaximumWidth(300);
	QBoxLayout* lk_VLayout_ = new QVBoxLayout(&mk_CheckRubyDialog);
	
	QBoxLayout* lk_HLayout_ = new QHBoxLayout();
	QLabel* lk_IconLabel_ = new QLabel(&mk_CheckRubyDialog);
	lk_IconLabel_ ->setPixmap(QPixmap(":/icons/dialog-warning.png"));
	lk_HLayout_->addWidget(lk_IconLabel_);
	QLabel* lk_ErrorLabel_ = new QLabel(&mk_CheckRubyDialog);
	lk_HLayout_->addWidget(lk_ErrorLabel_);
	lk_VLayout_->addLayout(lk_HLayout_);
	
	lk_HLayout_ = new QHBoxLayout();
	lk_HLayout_->addWidget(new QLabel("Path to Ruby:", &mk_CheckRubyDialog));
	mk_CheckRubyLocation_ = new QLineEdit(&mk_CheckRubyDialog);
	mk_CheckRubyLocation_->setText(mk_Configuration[CONFIG_PATH_TO_RUBY].toString());
	lk_HLayout_->addWidget(mk_CheckRubyLocation_);
	QToolButton* lk_FindRubyButton_ = new QToolButton(&mk_CheckRubyDialog);
	lk_FindRubyButton_->setIcon(QIcon(":/icons/system-search.png"));
	lk_HLayout_->addWidget(lk_FindRubyButton_);
	lk_VLayout_->addLayout(lk_HLayout_);
	
	lk_HLayout_ = new QHBoxLayout();
	lk_HLayout_->addStretch();
	QPushButton* lk_QuitButton_ = new QPushButton(QIcon(":/icons/system-log-out.png"), "Quit", &mk_CheckRubyDialog);
	lk_HLayout_->addWidget(lk_QuitButton_);
	mk_CheckRubyRetryButton_ = new QPushButton(QIcon(":/icons/view-refresh.png"), "Retry", &mk_CheckRubyDialog);
	lk_HLayout_->addWidget(mk_CheckRubyRetryButton_);
	lk_VLayout_->addLayout(lk_HLayout_);
	
	mk_CheckRubyRetryButton_->setEnabled(false);
	
	connect(mk_CheckRubyRetryButton_, SIGNAL(clicked()), &mk_CheckRubyDialog, SLOT(accept()));
	connect(lk_QuitButton_, SIGNAL(clicked()), &mk_CheckRubyDialog, SLOT(reject()));
	connect(mk_CheckRubyLocation_, SIGNAL(textChanged(const QString&)), this, SLOT(checkRubyTextChanged(const QString&)));
	connect(lk_FindRubyButton_, SIGNAL(clicked()), this, SLOT(checkRubySearchDialog()));
	
	mk_CheckRubyDialog.setLayout(lk_VLayout_);
	
	// see whether there's a local Ruby installed and prefer that
	// if there is a local Ruby then overwrite the configuration
	QString ls_OldRubyPath = mk_Configuration[CONFIG_PATH_TO_RUBY].toString();
	mk_Configuration[CONFIG_PATH_TO_RUBY] = "ruby";
	QString ls_Version = syncRuby(QStringList() << "-v");
	if (ls_Version.startsWith("ruby"))
	{
		/*
		ls_Version.replace("ruby", "");
		ls_Version = ls_Version.trimmed();
		QStringList lk_Tokens = ls_Version.split(" ");
		ls_Version = lk_Tokens.first();
		if (ls_Version == "1.8.6")
		{
			*/
			// we have found a local Ruby, hooray!
			this->saveConfiguration();
			return;
			/*
		}
		*/
	}
	mk_Configuration[CONFIG_PATH_TO_RUBY] = ls_OldRubyPath;
	
	while (true)
	{
		QString ls_Version = syncRuby(QStringList() << "-v");
		QString ls_Error;
		if (!ls_Version.startsWith("ruby"))
			ls_Error = "Proteomatic cannot find Ruby, which is required in order to run the scripts.";
		else
		{
			/*
			ls_Version.replace("ruby", "");
			ls_Version = ls_Version.trimmed();
			QStringList lk_Tokens = ls_Version.split(" ");
			ls_Version = lk_Tokens.first();
			if (ls_Version != "1.8.6")
				ls_Error = QString("The Ruby version on this computer is %1, but Proteomatic needs Ruby 1.8.6.").arg(ls_Version);
			else
				ls_Error = "";
			*/
			// if we're here, we have found a Ruby! Now we save the configuration so that the dialog won't pop up the next time.
			this->saveConfiguration();
		}
		if (ls_Error != "")
		{
			ls_Error += "<br />You can download Ruby at <a href='http://www.ruby-lang.org/en/downloads/'>http://www.ruby-lang.org/en/downloads/</a>.";
			ls_Error += "<br />If you already have Ruby, please specify the path to the Ruby interpreter below:";
			lk_ErrorLabel_->setText(ls_Error);
			mk_CheckRubyLocation_->setText(mk_Configuration[CONFIG_PATH_TO_RUBY].toString());
			if (mk_CheckRubyDialog.exec() == QDialog::Rejected)
				exit(0);
		}
		else
			break;
	}
}

void k_Proteomatic::checkRubyTextChanged(const QString& as_Text)
{
	mk_CheckRubyRetryButton_->setEnabled(!as_Text.isEmpty());
}


void k_Proteomatic::checkRubySearchDialog()
{
	QFileDialog lk_FileDialog(&mk_CheckRubyDialog, "Locate ruby executable", "", "");
	lk_FileDialog.setFileMode(QFileDialog::ExistingFile);
	if (lk_FileDialog.exec())
	{
		mk_Configuration[CONFIG_PATH_TO_RUBY] = QVariant(lk_FileDialog.selectedFiles().first());
		mk_CheckRubyLocation_->setText(mk_Configuration[CONFIG_PATH_TO_RUBY].toString());
		mk_CheckRubyRetryButton_->setEnabled(true);
	}
}


QString k_Proteomatic::findCurrentScriptPackage()
{
	QStringList lk_AvailableVersions = QDir(ms_ScriptPath).entryList(QDir::NoDotAndDotDot | QDir::AllDirs);
	if (lk_AvailableVersions.empty())
		return "";
	else
	{
		QString ls_Current;
		QFile lk_File(ms_ScriptPath + "/which.txt");
		if (lk_File.open(QIODevice::ReadOnly))
		{
			QTextStream lk_Stream(&lk_File);
			lk_Stream >> ls_Current;
			lk_File.close();
		}
		foreach (QString ls_Path, lk_AvailableVersions)
		{
			if (QFileInfo(ls_Path).fileName() == ls_Current)
				return ls_Path;
		}
		return QFileInfo(lk_AvailableVersions.first()).fileName();
	}
}


bool k_Proteomatic::fileUpToDate(QString as_Path, QStringList ak_Dependencies)
{
	QFileInfo lk_MainInfo(as_Path);
	if (!lk_MainInfo.exists())
		return false;
		
	foreach (QString ls_Path, ak_Dependencies)
	{
		QFileInfo lk_FileInfo(ls_Path);
		if (lk_FileInfo.exists())
			if (lk_MainInfo.lastModified() < lk_FileInfo.lastModified())
				return false;
	}
	return true;
}
