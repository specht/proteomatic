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
#include "FoldedHeader.h"
#include "PipelineTabWidget.h"


class k_PipelineMainWindow: public QMainWindow
{
    Q_OBJECT
public:
    k_PipelineMainWindow(QWidget* ak_Parent_, k_Proteomatic& ak_Proteomatic, QApplication& ak_Application);
    virtual ~k_PipelineMainWindow();
    
    void initialize();
    
    //QString outputPrefix();
    virtual void setCurrentScriptBox(IScriptBox* ak_ScriptBox_);
    virtual bool panMode() const;
    virtual QMenu* createPopupMenu();
    virtual bool askForSaveIfNecessary();
    
public slots:
    void toggleUi();
    void togglePaneFloat();
    void newPipeline();
    void loadPipeline();
    void savePipeline();
    void savePipelineAs();
    void searchFieldPopup(const QString& as_String);
    void restartProteomatic();
    void previewFile(const QString& as_Path);
    
signals:
    //void outputPrefixChanged(const QString& as_Prefix);
    void forceRefresh();
    
protected:
    virtual void closeEvent(QCloseEvent* event);
    virtual void keyPressEvent(QKeyEvent* ak_Event_);

protected slots:
    void quit();
    void addScript(QAction* ak_Action_);
    void start();
    void startUntracked();
    void abort();
    void addFileListBox();
    void addSnippetBox();
    void resetParameters();
    void showProfileManager();
    void showAll();
    void updateStatusBar();
    void scriptMenuChanged();
    void updateWindowTitle();
    void previewTabCloseRequested(int ai_Index);

protected:
    QApplication& mk_Application;
    k_Desktop* mk_Desktop_;
    QToolButton* mk_ProteomaticButton_;
    QToolButton* mk_AddScriptAction_;
    QAction* mk_NewPipelineAction_;
    QAction* mk_LoadPipelineAction_;
    QAction* mk_SavePipelineAction_;
    QAction* mk_SavePipelineAsAction_;
    QAction* mk_QuitAction_;
    QToolButton* mk_AddScriptButton_;
    QToolButton* mk_AddFilesButton_;
    QAction* mk_AddFileListAction_;
    QAction* mk_AddSnippetAction_;
    QAction* mk_AbortAction_;
    QAction* mk_RefreshAction_;
    QAction* mk_ProfileManagerAction_;
    QAction* mk_ResetParametersAction_;
    QAction* mk_ShowConfigurationAction_;
    QToolButton* mk_PanButton_;
    QToolButton* mk_LassoButton_;
    //QLineEdit* mk_OutputPrefix_;
    //QAction* mk_ClearPrefixForAllScriptsAction_;
    QAction* mk_ProposePrefixForAllScriptsAction_;
    k_Proteomatic& mk_Proteomatic;
    IScriptBox* mk_CurrentScriptBox_;
    QWidget* mk_PaneLayoutWidget_;
    QBoxLayout* mk_PaneLayout_;
    QString ms_PipelineFilename;
    QSplitter* mk_HSplitter_;
    QLabel* mk_StatusBarMessage_;
    QObject* mk_WatchedBoxObject_;
    QToolBar* mk_AddToolBar_;
    QDockWidget* mk_PaneDockWidget_;
    QWidget* mk_FauxTitleBarWidget_;
    QLabel* mk_FauxTitleBarWidgetFloatingLabel_;
    QWidget* mk_FauxTitleBarWidgetFloating_;
    bool mb_JustStarted;
    QRegExp mk_WordSplitter;
    QSharedPointer<QListWidget> mk_pSearchPopup;
    QString ms_CurrentStyleSheet;
    k_PipelineTabWidget* mk_PreviewTabWidget_;
    QHash<QString, QSharedPointer<QWidget> > mk_Previews;
};
