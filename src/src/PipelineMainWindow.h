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
#include "Desktop.h"
#include "Proteomatic.h"
#include "ConsoleString.h"


class k_PipelineMainWindow: public QMainWindow
{
	Q_OBJECT
public:
	k_PipelineMainWindow(QWidget* ak_Parent_, k_Proteomatic& ak_Proteomatic);
	virtual ~k_PipelineMainWindow();
	QString outputPrefix();
	virtual void addOutput(QString as_String);
	virtual void clearOutput();
	virtual void setCurrentScriptBox(IScriptBox* ak_ScriptBox_);
	
public slots:
	void toggleUi();
	
signals:
	void outputPrefixChanged(const QString& as_Prefix);
	void forceRefresh();
	
protected:
	virtual void closeEvent(QCloseEvent* event);
	virtual bool askForSaveIfNecessary();

protected slots:
	void newPipeline();
	void loadPipeline();
	void savePipeline();
	void savePipelineAs();
	void quit();
	void addScript(QAction* ak_Action_);
	void start();
	void abort();
	void addFileListBox();
	void resetParameters();
	void showProfileManager();
	void showAll();
	void updateStatusBar();

protected:
	k_Desktop* mk_Desktop_;
	QToolButton* mk_AddScriptAction_;
	QAction* mk_NewPipelineAction_;
	QAction* mk_LoadPipelineAction_;
	QAction* mk_SavePipelineAction_;
	QAction* mk_SavePipelineAsAction_;
	QAction* mk_QuitAction_;
	QAction* mk_AddFileListAction_;
	QAction* mk_StartAction_;
	QAction* mk_AbortAction_;
	QAction* mk_RefreshAction_;
	QAction* mk_ProfileManagerAction_;
	QAction* mk_ResetParametersAction_;
	QLineEdit* mk_OutputPrefix_;
	QAction* mk_ClearPrefixForAllScriptsAction_;
	QAction* mk_ProposePrefixForAllScriptsAction_;
	k_Proteomatic& mk_Proteomatic;
	QFileSystemWatcher mk_FileSystemWatcher;
	QTextEdit* mk_Log_;
	k_ConsoleString ms_Log;
	IScriptBox* mk_CurrentScriptBox_;
	QWidget* mk_PaneLayoutWidget_;
	QBoxLayout* mk_PaneLayout_;
	QString ms_PipelineFilename;
	QSplitter* mk_HSplitter_;
	QLabel* mk_StatusBarMessage_;
	QObject* mk_WatchedBoxObject_;
};
