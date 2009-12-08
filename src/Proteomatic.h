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

#pragma once

#include <QtGui>
#include <QtNetwork>
#include "RefPtr.h"
#include "Yaml.h"

#define CONFIG_PATH_TO_RUBY "pathToRuby"
#define CONFIG_PATH_TO_PYTHON "pathToPython"
#define CONFIG_PATH_TO_PHP "pathToPhp"
#define CONFIG_REMOTE_SCRIPTS "remoteScripts"
#define CONFIG_PROFILES "profiles"
#define CONFIG_REMEMBER_PROFILE_PATH "rememberProfilePath"
#define CONFIG_REMEMBER_PIPELINE_PATH "rememberPipelinePath"
#define CONFIG_REMEMBER_INPUT_FILES_PATH "rememberInputFilesPath"
#define CONFIG_REMEMBER_OUTPUT_PATH "rememberOutputPath"
#define CONFIG_SCRIPTS_URL "scriptsUrl"
#define CONFIG_SCRIPTS_PATH "scriptsPath"
#define CONFIG_AUTO_CHECK_FOR_UPDATES "autoCheckForUpdates"
#define CONFIG_WARN_ABOUT_MIXED_PROFILES "warnAboutMixedProfiles"
#define CONFIG_CACHE_SCRIPT_INFO "cacheScriptInfo"
#define CONFIG_FILETRACKER_URL "fileTrackerUrl"
#define CONFIG_FOLLOW_NEW_BOXES "followNewBoxes"
#define CONFIG_ANIMATION "animation"
#define CONFIG_APPEARANCE_SIZE "appearanceSize"


struct r_RemoteRequestType
{
	enum Enumeration
	{
		Unknown = 0,
		GetInfo,
		GetParameters,
		GetInfoAndParameters,
		SubmitJob,
		QueryTicket,
		GetStandardOutput,
		GetOutputFiles
	};
};


struct r_RemoteRequest
{
public:
	r_RemoteRequest()
		: me_Type(r_RemoteRequestType::Unknown)
		, ms_Info("")
		, mk_AdditionalInfo(QStringList())
		, ms_Response("")
	{
	}
	
	r_RemoteRequest(r_RemoteRequestType::Enumeration ae_Type, QString as_Info = "", QStringList ak_AdditionalInfo = QStringList())
		: me_Type(ae_Type)
		, ms_Info(as_Info)
		, mk_AdditionalInfo(ak_AdditionalInfo)
		, ms_Response("")
	{
	}
	
	virtual ~r_RemoteRequest()
	{
	}
	
	r_RemoteRequestType::Enumeration me_Type;
	QString ms_Info;
	QStringList mk_AdditionalInfo;
	QString ms_Response;
};


class k_Proteomatic: public QObject
{
	Q_OBJECT
public:
	k_Proteomatic(QCoreApplication& ak_Application, bool ab_NeedScripts = true);
	virtual ~k_Proteomatic();
	
	void checkForUpdates();

	QString interpreterForScript(QString as_Path);
    QString interpreterKeyForScript(QString as_Path);
	QStringList availableScripts();
	QHash<QString, QString> scriptInfo(QString as_ScriptPath);
	QString scriptInfo(QString as_ScriptPath, QString as_Key);
	bool hasScriptInfo(QString as_ScriptPath);
	QMenu* proteomaticScriptsMenu() const;
	QString syncRuby(QStringList ak_Arguments);
	QString syncScript(QStringList ak_Arguments);
    QString syncScriptNoFile(QStringList ak_Arguments, QString as_Language);
	QString syncShowRuby(QStringList ak_Arguments, QString as_Title = "Ruby script");
	QString rubyPath();
	QString version();
	bool versionChanged() const;
	int showMessageBox(QString as_Title, QString as_Text, QString as_Icon = "", 
		QMessageBox::StandardButtons ae_Buttons = QMessageBox::Ok, 
		QMessageBox::StandardButton ae_DefaultButton = QMessageBox::Ok, 
		QMessageBox::StandardButton ae_EscapeButton = QMessageBox::Ok,
		QString as_InformativeText = QString(), QString as_DetailedText = QString());
	void setMessageBoxParent(QWidget* ak_Widget_);
	QWidget* messageBoxParent() const;
	int queryRemoteHub(QString as_Uri, QStringList ak_Arguments);
	QFont& consoleFont();
	QStringList scriptPaths() const;
	QString scriptPathAndPackage() const;
	QVariant getConfiguration(QString as_Key);
	tk_YamlMap& getConfigurationRoot();
	void saveConfiguration();
	QString scriptsVersion();
	bool fileUpToDate(QString as_Path, QStringList ak_Dependencies);
	static void openFileLink(QString as_Path);
	QString md5ForFile(QString as_Path);
	void reloadScripts();
	QToolButton* startButton();
	QAction* startUntrackedAction();
	QLabel* fileTrackerIconLabel();
	QLabel* fileTrackerLabel();
    QString scriptInterpreter(const QString& as_Language);
    bool stringToBool(const QString& as_String);
    
    QHash<QString, QStringList> mk_ScriptKeywords;
	
signals:
	void scriptMenuScriptClicked(QAction* ak_Action_);
	void scriptMenuChanged();
	void remoteHubReady();
	void remoteHubLineBatch(QStringList ak_Lines);
	void remoteHubRequestFinished(int ai_SocketId, bool ab_Error, QString as_Response);
	void configurationChanged();

public slots:
	void showConfigurationDialog();
	
protected slots:
	void remoteHubReadyReadSlot();
	void scriptMenuScriptClickedInternal();
	void addRemoteScriptDialog();
	void remoteHubRequestFinishedSlot(int ai_SocketId, bool ab_Error);
	void rebuildRemoteScriptsMenu();
	void checkRubyTextChanged(const QString& as_Text);
	void checkRubySearchDialog();

protected:
	void loadConfiguration();
	void collectScriptInfo(bool ab_ShowImmediately = false);
	void createProteomaticScriptsMenu();
	void checkRuby();
	QString findCurrentScriptPackage();
	void updateConfigDependentStuff();
	
	QCoreApplication& mk_Application;
	// uri / path => uri, title, group, description, optional: parameters
	QHash<QString, QHash<QString, QString> > mk_ScriptInfo;
	QWidget* mk_MessageBoxParent_;
	RefPtr<QMenu> mk_pProteomaticScriptsMenu;
	QMenu* mk_RemoteMenu_;
// 	RefPtr<QProcess> mk_pRemoteHubProcess;
	QString ms_RemoteHubStdout;
// 	RefPtr<QHttp> mk_pRemoteHubHttp;
	QFont mk_ConsoleFont;
	QStringList mk_ScriptPaths;
	QString ms_ConfigurationPath;
	QString ms_ScriptPackage;
	
	// title => uri
	QMap<QString, QString> mk_RemoteScripts;
	
	// socket id => request
	QHash<int, r_RemoteRequest> mk_RemoteRequests;
	
	QString ms_RemoteHubPortion;
	tk_YamlMap mk_Configuration;
	
	// check ruby stuff
	QDialog mk_CheckRubyDialog;
	QPushButton* mk_CheckRubyRetryButton_;
	QLineEdit* mk_CheckRubyLocation_;
	
	// mk_pStartButton starts with file tracking, if configured
	QToolButton* mk_StartButton_;
	QLabel* mk_FileTrackerIconLabel_;
	QLabel* mk_FileTrackerLabel_;
	QMenu mk_StartButtonMenu;
	// mk_StartUntrackedAction_ starts without file tracking
	QAction* mk_StartUntrackedAction_;
    QHash<QString, QString> mk_ScriptInterpreter;
    QHash<QString, bool> mk_ScriptInterpreterWorking;
};
