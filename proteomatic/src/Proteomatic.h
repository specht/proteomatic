#pragma once

#include <QtGui>
#include <QtNetwork>
#include "RefPtr.h"


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
	k_Proteomatic(QString as_ApplicationPath);
	virtual ~k_Proteomatic();

	QStringList availableScripts();
	QHash<QString, QString> scriptInfo(QString as_ScriptPath);
	QString scriptInfo(QString as_ScriptPath, QString as_Key);
	QMenu* proteomaticScriptsMenu() const;
	QString syncRuby(QStringList ak_Arguments);
	QString syncShowRuby(QStringList ak_Arguments, QString as_Title = "Ruby script");
	QString rubyPath();
	QString version();
	bool versionChanged() const;
	bool supressCache() const;
	int showMessageBox(QString as_Title, QString as_Text, QString as_Icon = "", 
		QMessageBox::StandardButtons ae_Buttons = QMessageBox::Ok, 
		QMessageBox::StandardButton ae_DefaultButton = QMessageBox::Ok, 
		QMessageBox::StandardButton ae_EscapeButton = QMessageBox::Ok);
	void setMessageBoxParent(QWidget* ak_Widget_);
	QWidget* messageBoxParent() const;
	int queryRemoteHub(QString as_Uri, QStringList ak_Arguments);
	QFont& consoleFont();
	QString scriptPath() const;
	
signals:
	void scriptMenuScriptClicked(QAction* ak_Action_);
	void remoteHubReady();
	void remoteHubLineBatch(QStringList ak_Lines);
	void remoteHubRequestFinished(int ai_SocketId, bool ab_Error, QString as_Response);

protected slots:
	void remoteHubReadyReadSlot();
	void scriptMenuScriptClickedInternal();
	void addRemoteScriptDialog();
	void remoteHubRequestFinishedSlot(int ai_SocketId, bool ab_Error);
	void rebuildRemoteScriptsMenu();

protected:
	void collectScriptInfo();
	void createProteomaticScriptsMenu();
	void rememberScriptUri(QString as_Uri);
	QStringList rememberedScriptUris();
	
	QString ms_RubyPath;
	QString ms_Version;
	
	// uri / path => uri, title, group, description, optional: parameters
	QHash<QString, QHash<QString, QString> > mk_ScriptInfo;
	bool mb_SupressCache;
	QWidget* mk_MessageBoxParent_;
	RefPtr<QMenu> mk_pProteomaticScriptsMenu;
	QMenu* mk_RemoteMenu_;
	RefPtr<QProcess> mk_pRemoteHubProcess;
	QString ms_RemoteHubStdout;
	QHttp* mk_RemoteHubHttp_;
	QFont mk_ConsoleFont;
	QString ms_ScriptPath;
	
	// title => uri
	QMap<QString, QString> mk_RemoteScripts;
	
	// socket id => request
	QHash<int, r_RemoteRequest> mk_RemoteRequests;
	
	QString ms_RemoteHubPortion;
};
