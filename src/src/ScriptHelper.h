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
#include "ClickableLabel.h"
#include "ConsoleString.h"
#include "FileList.h"
#include "IScript.h"
#include "Proteomatic.h"
#include "RefPtr.h"
#include "TicketWindow.h"
#include "ProfileManager.h"


class k_ScriptHelper: public QMainWindow
{
	Q_OBJECT
public:
	k_ScriptHelper(QWidget* ak_Parent_, k_Proteomatic& ak_Proteomatic);
	~k_ScriptHelper();

protected:
	virtual void dragEnterEvent(QDragEnterEvent* ak_Event_);
	virtual void dragMoveEvent(QDragMoveEvent* ak_Event_);
	virtual void dropEvent(QDropEvent* ak_Event_);
	virtual void keyPressEvent(QKeyEvent* ak_Event_);

protected slots:
	void reset();
	void start(bool ab_UseFileTrackerIfAvailable = true);
	void startUntracked();
	void processStarted();
	void processFinished(int ai_ExitCode);
	void processReadyRead();
	void parameterLabelClicked(const QString& as_Id);
	void loadFilesButtonClicked();
	void resetParameters();
	void abortScript();
	void resetDialog();
	void aboutDialog();
	void scriptMenuScriptClicked(QAction* ak_Action_);
	void scriptMenuChanged();
	void toggleUi();
	void remoteHubLineBatch(QStringList ak_Lines);
	void remoteHubRequestFinished(int ai_SocketId, bool ab_Error, QString as_Response);
	void checkTicket();
	void checkTicket(QString as_Ticket);
	void ticketWindowClosed();
	void showProfileManager();
	void proposePrefix();

protected:
	void setScript(QString as_Filename);
	void activateScript();
	void addOutput(QString as_Text);
	bool checkVersionChanged();
	void updateWindowTitle();

	QString ms_WindowTitle;
	RefPtr<QWidget> mk_pInputFilesContainer;
	QHash<QString, RefPtr<k_FileList> > mk_FileLists;
	QHash<QString, QWidget*> mk_FileListsRemoveButtons;
	QPushButton mk_ResetButton;
	QPushButton mk_ResetParametersButton;
	QPushButton mk_LoadParametersButton;
	QPushButton mk_SaveParametersButton;
	QTextEdit mk_Output;
	k_ConsoleString ms_Output;

	QVBoxLayout mk_MainLayout;
	QVBoxLayout* mk_LowerLayout_;
	QSplitter* mk_VSplitter_;
	QSplitter* mk_HSplitter_;
	QBoxLayout* mk_ParameterLayout_;
	QWidget* mk_ParameterLayoutWidget_;
	QScrollArea* mk_ScrollArea_;
	
	bool mb_VersionChanged;

	k_Proteomatic& mk_Proteomatic;

	RefPtr<IScript> mk_pScript;
	RefPtr<k_ProfileManager> mk_pProfileManager;
	
	QAction* mk_ProfilesAction_;
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
