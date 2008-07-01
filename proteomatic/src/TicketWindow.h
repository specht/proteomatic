#pragma once

#include <QtCore>
#include <QtGui>
#include "Proteomatic.h"
#include "ConsoleString.h"


class k_TicketWindow: public QWidget
{
	Q_OBJECT
	
public:
	k_TicketWindow(k_Proteomatic& ak_Proteomatic, QString as_ScriptUri, QString as_Ticket);
	virtual ~k_TicketWindow();
	
signals:
	void closed();
		
protected slots:
	void queryTicket();
	void remoteRequestFinished(int ai_SocketId, bool ab_Error, QString as_Result);
	void saveOutputFiles();
	
protected:
	void closeEvent(QCloseEvent* ak_Event_);

	k_Proteomatic& mk_Proteomatic;
	QString ms_ScriptUri;
	QString ms_Ticket;
	QLabel* mk_StateLabel_;
	QLabel* mk_StateIcon_;
	QHash<int, r_RemoteRequest> mk_RemoteRequests;
	QTextEdit* mk_Output_;
	QToolButton* mk_ChooseOutputDirectory_;
	QLabel* mk_OutputDirectoryLabel_;
	QLineEdit* mk_OutputDirectory_;
	QLabel* mk_PrefixLabel_;
	QLineEdit* mk_Prefix_;
	QPushButton* mk_SaveButton_;
	k_ConsoleString mk_StandardOutput;
	bool mb_GotStandardOutput;
	int mi_Delay;
	QStringList mk_OutputFiles;
};
