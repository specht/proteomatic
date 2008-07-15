#include <QtGui>
#include "ClickableLabel.h"
#include "ConsoleString.h"
#include "FileList.h"
#include "Proteomatic.h"
#include "RefPtr.h"
#include "Script.h"
#include "TicketWindow.h"


class k_ScriptHelper: public QMainWindow
{
	Q_OBJECT
public:
	k_ScriptHelper(QWidget* ak_Parent_, k_Proteomatic& ak_Proteomatic);
	~k_ScriptHelper();

protected:
	virtual void dragEnterEvent(QDragEnterEvent* ak_Event_);
	virtual void dropEvent(QDropEvent* ak_Event_);

protected slots:
	void reset();
	void start();
	void processStarted();
	void processFinished(int ai_ExitCode, QProcess::ExitStatus ak_ExitStatus);
	void processReadyRead();
	void parameterLabelClicked(const QString& as_Id);
	void loadFilesButtonClicked();
	void resetParameters();
	void setOutputDirectoryButtonClicked();
	void abortScript();
	void saveParameters();
	void loadParameters();
	void resetDialog();
	void aboutDialog();
	void scriptMenuScriptClicked(QAction* ak_Action_);
	void toggleUi();
	void remoteHubLineBatch(QStringList ak_Lines);
	void remoteHubRequestFinished(int ai_SocketId, bool ab_Error, QString as_Response);
	void checkTicket();
	void checkTicket(QString as_Ticket);
	void ticketWindowClosed();

protected:
	void setScript(QString as_Filename);
	void activateScript();
	void addOutput(QString as_Text);
	void addInputFile(QString as_Path);
	bool checkVersionChanged();

	QString ms_WindowTitle;
	QLineEdit mk_OutputDirectory;
	k_FileList mk_FileList;
	QToolButton mk_RemoveInputFileButton;
	QPushButton mk_ResetButton;
	QPushButton mk_ResetParametersButton;
	QPushButton mk_LoadParametersButton;
	QPushButton mk_SaveParametersButton;
	QToolButton mk_AddFilesButton;
	QToolButton mk_SetOutputDirectoryButton;
	QTextEdit mk_Output;
	k_ConsoleString ms_Output;
	bool mb_VersionChanged;

	QVBoxLayout mk_MainLayout;
	QVBoxLayout* mk_UpperLayout_;
	QVBoxLayout* mk_LowerLayout_;
	QHBoxLayout* mk_TopLevelLayout_;
	QMenu* mk_ScriptMenu_;
	QSplitter* mk_VSplitter_;
	QSplitter* mk_HSplitter_;
	QScrollArea* mk_ScrollArea_;
	
	// should be a RefPtr, but not possible with VC on win32... (sigh!)
	k_Script* mk_Script_;
	
	k_Proteomatic& mk_Proteomatic;
	
	QAction* mk_StartAction_;
	QAction* mk_AbortAction_;
	QAction* mk_LoadParametersAction_;
	QAction* mk_SaveParametersAction_;
	QAction* mk_ResetAction_;
	QAction* mk_ReloadScriptAction_;
	QAction* mk_CheckTicketAction_;
	QToolButton* mk_LoadScriptButton_;
	QHash<int, r_RemoteRequest> mk_RemoteRequests;
	QHash<k_TicketWindow*, RefPtr<k_TicketWindow> > mk_TicketWindows;
	QProgressDialog* mk_ProgressDialog_;
};
