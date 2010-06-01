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
#include "PipelineMainWindow.h"
#include "Desktop.h"
#include "HintLineEdit.h"
#include "IFileBox.h"
#include "IScriptBox.h"
#include "ProfileManager.h"
#include "Proteomatic.h"
#include "Tango.h"
#include "UnclickableLabel.h"
#include "version.h"
#include "Yaml.h"


k_PipelineMainWindow::k_PipelineMainWindow(QWidget* ak_Parent_, k_Proteomatic& ak_Proteomatic, QApplication& ak_Application)
    : QMainWindow(ak_Parent_)
    , mk_Application(ak_Application)
    , mk_Desktop_(new k_Desktop(this, ak_Proteomatic, *this))
    , mk_Proteomatic(ak_Proteomatic)
    //, mk_OutputPrefix_(new QLineEdit(this))
    , mk_CurrentScriptBox_(NULL)
    , ms_PipelineFilename(QString())
    , mk_WatchedBoxObject_(NULL)
    , mb_JustStarted(true)
    , mk_WordSplitter("\\W+")

{
}


void k_PipelineMainWindow::initialize()
{
    mk_Proteomatic.setMessageBoxParent(this);
    mk_Proteomatic.setDesktop(mk_Desktop_);
    
    connect(mk_Desktop_, SIGNAL(selectionChanged()), this, SLOT(updateStatusBar()));

    setWindowIcon(QIcon(":/icons/proteomatic-pipeline.png"));
    updateWindowTitle();
    resize(1000, 600);
    
    mk_HSplitter_ = new QSplitter(this);
    mk_HSplitter_->setOrientation(Qt::Horizontal);
    mk_PaneLayoutWidget_ = new QWidget(this);
    mk_PaneLayout_ = new QVBoxLayout(mk_PaneLayoutWidget_);
    mk_PaneLayout_->setContentsMargins(0, 0, 0, 0);
    
    setCentralWidget(mk_HSplitter_);

    // lk_FauxTitleBarWidget_ is an empty widget so that
    // the pane widget will have no title bar
    mk_FauxTitleBarWidget_ = new QWidget(this);
    //mk_FauxTitleBarWidget_ = new QLabel("<b>Script parameters</b>", this);
    mk_FauxTitleBarWidgetFloating_ = new QWidget(this);
    mk_FauxTitleBarWidgetFloating_->setWindowIcon(QIcon(":/icons/proteomatic.png"));
    mk_FauxTitleBarWidgetFloating_->setWindowTitle("Current script");
    QVBoxLayout* lk_FauxTitleVLayout_ = new QVBoxLayout(mk_FauxTitleBarWidgetFloating_);
    QHBoxLayout* lk_FauxTitleLayout_ = new QHBoxLayout(NULL);
    lk_FauxTitleLayout_->setContentsMargins(0, 4, 0, 0);
    lk_FauxTitleVLayout_->setContentsMargins(0, 0, 0, 4);
    QLabel* lk_FauxHeaderIconLabel_ = new k_UnclickableLabel(mk_FauxTitleBarWidgetFloating_);
    lk_FauxHeaderIconLabel_->setPixmap(QPixmap(":/icons/proteomatic.png").scaledToHeight(16, Qt::SmoothTransformation));
    lk_FauxTitleLayout_->addSpacing(4);
    lk_FauxTitleLayout_->addWidget(lk_FauxHeaderIconLabel_);
    mk_FauxTitleBarWidgetFloatingLabel_ = new k_UnclickableLabel("Current script", mk_FauxTitleBarWidgetFloating_);
    lk_FauxTitleLayout_->addWidget(mk_FauxTitleBarWidgetFloatingLabel_);
    lk_FauxTitleLayout_->addStretch();
    lk_FauxTitleVLayout_->addLayout(lk_FauxTitleLayout_);
    QFrame* lk_Frame_ = new QFrame(this);
    lk_Frame_->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    lk_FauxTitleVLayout_->addWidget(lk_Frame_);
    
    mk_FauxTitleBarWidget_->hide();
    mk_FauxTitleBarWidgetFloating_->hide();
    
    mk_PaneDockWidget_ = new QDockWidget("Script parameters", this);
    mk_PaneDockWidget_->setTitleBarWidget(mk_FauxTitleBarWidgetFloating_);
    mk_PaneDockWidget_->setWidget(mk_PaneLayoutWidget_);
    mk_PaneDockWidget_->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    mk_PaneDockWidget_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    mk_PaneDockWidget_->hide();
    mk_PaneDockWidget_->setMinimumSize(QSize(440, 1));

    mk_HSplitter_->addWidget(mk_Desktop_);
//  mk_HSplitter_->addWidget(mk_PaneLayoutWidget_);
    mk_HSplitter_->setChildrenCollapsible(false);

    statusBar()->show();
    mk_StatusBarMessage_ = new QLabel("", this);
    statusBar()->addWidget(mk_StatusBarMessage_);
    statusBar()->addPermanentWidget(mk_Proteomatic.fileTrackerIconLabel());
    statusBar()->addPermanentWidget(mk_Proteomatic.fileTrackerLabel());
    
    mk_AddToolBar_ = new QToolBar(this);
    
    mk_AddToolBar_->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);


    QToolButton* lk_ProteomaticButton_ = new QToolButton(mk_AddToolBar_);
    lk_ProteomaticButton_->setIcon(QIcon(":/icons/proteomatic-pipeline.png"));
    lk_ProteomaticButton_->setText("Pipeline");
    QMenu* lk_ProteomaticMenu_ = new QMenu(this);
    mk_NewPipelineAction_ = lk_ProteomaticMenu_->addAction(QIcon(":icons/document-new.png"), "New pipeline", this, SLOT(newPipeline()), QKeySequence("Ctrl+N"));
    mk_LoadPipelineAction_ = lk_ProteomaticMenu_->addAction(QIcon(":icons/document-open.png"), "Open pipeline...", this, SLOT(loadPipeline()), QKeySequence("Ctrl+O"));
    mk_SavePipelineAction_ = lk_ProteomaticMenu_->addAction(QIcon(":icons/document-save.png"), "Save pipeline", this, SLOT(savePipeline()), QKeySequence("Ctrl+S"));
    mk_SavePipelineAsAction_ = lk_ProteomaticMenu_->addAction(QIcon(":icons/document-save-as.png"), "Save pipeline as...", this, SLOT(savePipelineAs()));
    lk_ProteomaticMenu_->addSeparator();
    mk_QuitAction_ = lk_ProteomaticMenu_->addAction(QIcon(":icons/system-shutdown.png"), "Quit", this, SLOT(quit()));
    lk_ProteomaticButton_->setMenu(lk_ProteomaticMenu_);
    lk_ProteomaticButton_->setPopupMode(QToolButton::InstantPopup);
    lk_ProteomaticButton_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    mk_AddToolBar_->addWidget(lk_ProteomaticButton_);

    mk_AddScriptButton_ = new QToolButton(mk_AddToolBar_);
    mk_AddScriptButton_->setIcon(QIcon(":/icons/proteomatic.png"));
    mk_AddScriptButton_->setText("Add script");
    mk_AddScriptButton_->setPopupMode(QToolButton::InstantPopup);
    mk_AddScriptButton_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    mk_AddToolBar_->addWidget(mk_AddScriptButton_);
    mk_AddScriptButton_->setMenu(mk_Proteomatic.proteomaticScriptsMenu());
    connect(mk_Proteomatic.proteomaticScriptsMenu(), SIGNAL(triggered(QAction*)), this, SLOT(addScript(QAction*)));
    mk_AddScriptAction_ = mk_AddScriptButton_;
    connect(&mk_Proteomatic, SIGNAL(scriptMenuChanged()), this, SLOT(scriptMenuChanged()));

    // currently disabled search function
    /*
    k_HintLineEdit* lk_SearchField_ = new k_HintLineEdit(this);
    lk_SearchField_->setHint("Search");
    connect(lk_SearchField_, SIGNAL(textEdited(const QString&)), this, SLOT(searchFieldPopup(const QString&)));
    mk_AddToolBar_->addWidget(lk_SearchField_);
    */
    
    mk_AddFileListAction_ = mk_AddToolBar_->addAction(QIcon(":/icons/document-open.png"), "Add file list", this, SLOT(addFileListBox()));

    mk_AddToolBar_->addSeparator();
    
    mk_PanButton_ = new QToolButton(mk_AddToolBar_);
    mk_PanButton_->setIcon(QIcon(":/icons/cursor-openhand.png"));
    mk_PanButton_->setText("Pan");
    mk_PanButton_->setCheckable(true);
    mk_PanButton_->setAutoExclusive(true);
    mk_PanButton_->setChecked(true);
    connect(mk_PanButton_, SIGNAL(toggled(bool)), mk_Desktop_, SLOT(updatePanMode()));
    mk_AddToolBar_->addWidget(mk_PanButton_);

    mk_LassoButton_ = new QToolButton(mk_AddToolBar_);
    mk_LassoButton_->setIcon(QIcon(":/icons/select-lasso.png"));
    mk_LassoButton_->setText("Select");
    mk_LassoButton_->setCheckable(true);
    mk_LassoButton_->setAutoExclusive(true);
    connect(mk_LassoButton_, SIGNAL(toggled(bool)), mk_Desktop_, SLOT(updatePanMode()));
    mk_AddToolBar_->addWidget(mk_LassoButton_);
    
    mk_AddToolBar_->addAction(QIcon(":icons/system-search.png"), "Show all", this, SLOT(showAll()));

    mk_AddToolBar_->addSeparator();
    
    mk_ProposePrefixForAllScriptsAction_ = mk_AddToolBar_->addAction(QIcon(":icons/select-continuous-area.png"), "Propose prefixes", mk_Desktop_, SLOT(proposePrefixForAllScripts()));
    mk_RefreshAction_ = mk_AddToolBar_->addAction(QIcon(":icons/view-refresh.png"), "Refresh", this, SIGNAL(forceRefresh()));
    mk_AddToolBar_->addWidget(mk_Proteomatic.startButton());
    connect(mk_Proteomatic.startButton(), SIGNAL(clicked()), this, SLOT(start()));
    
    connect(mk_Proteomatic.startUntrackedAction(), SIGNAL(triggered()), this, SLOT(startUntracked()));
    mk_AbortAction_ = mk_AddToolBar_->addAction(QIcon(":icons/dialog-cancel.png"), "Abort", this, SLOT(abort()));

    mk_AddToolBar_->addSeparator();
    
    mk_ProfileManagerAction_ = mk_AddToolBar_->addAction(QIcon(":icons/document-properties.png"), "Profiles", this, SLOT(showProfileManager()));
    mk_ResetParametersAction_ = mk_AddToolBar_->addAction(QIcon(":icons/edit-clear.png"), "Reset", this, SLOT(resetParameters()));
    mk_AddToolBar_->addSeparator();
    mk_ShowConfigurationAction_ = new QAction(QIcon(":icons/preferences-system.png"), "Preferences", this);
    mk_ShowConfigurationAction_->setShortcut(QKeySequence("Ctrl+P"));
    connect(mk_ShowConfigurationAction_, SIGNAL(triggered()), &mk_Proteomatic, SLOT(showConfigurationDialog()));
     mk_AddToolBar_->addAction(mk_ShowConfigurationAction_);
    
/*  QToolBar* lk_OtherToolBar_ = new QToolBar(this);
    lk_OtherToolBar_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    
    lk_OtherToolBar_->addWidget(new QLabel("Global output prefix: ", this));
    
    lk_OtherToolBar_->addWidget(mk_OutputPrefix_);
    connect(mk_OutputPrefix_, SIGNAL(textChanged(const QString&)), this, SIGNAL(outputPrefixChanged(const QString&)));
    
    mk_ClearPrefixForAllScriptsAction_ = lk_OtherToolBar_->addAction(QIcon(":icons/dialog-cancel.png"), "", mk_Desktop_, SLOT(clearPrefixForAllScripts()));
    mk_ProposePrefixForAllScriptsAction_ = lk_OtherToolBar_->addAction(QIcon(":icons/select-continuous-area.png"), "", mk_Desktop_, SLOT(proposePrefixForAllScripts()));
    mk_ProposePrefixForAllScriptsAction_->setCheckable(true);*/
    
    addToolBar(Qt::TopToolBarArea, mk_AddToolBar_);
    addDockWidget(Qt::RightDockWidgetArea, mk_PaneDockWidget_);
/*  addToolBarBreak(Qt::TopToolBarArea);
    addToolBar(Qt::TopToolBarArea, lk_OtherToolBar_);*/
    
//  lk_OtherToolBar_->setIconSize(QSize(24, 24));
    
    //setDockOptions(dockOptions() | QMainWindow::VerticalTabs);
    //setDockOptions(dockOptions() | QMainWindow::ForceTabbedDocks);
    //setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::North);
    
    connect(&mk_Proteomatic, SIGNAL(configurationChanged()), this, SLOT(toggleUi()));
    
    updateStatusBar();
    toggleUi();
}


k_PipelineMainWindow::~k_PipelineMainWindow()
{
    delete mk_Desktop_;
}


/*QString k_PipelineMainWindow::outputPrefix()
{
    return mk_OutputPrefix_->text();
}
*/

void k_PipelineMainWindow::closeEvent(QCloseEvent* event)
{
    if (askForSaveIfNecessary())
        event->accept();
    else
        event->ignore();
}


void k_PipelineMainWindow::keyPressEvent(QKeyEvent* ak_Event_)
{
    if ((ak_Event_->key() == Qt::Key_R) && ((ak_Event_->modifiers() & Qt::ControlModifier) != 0))
    {
        mk_Proteomatic.reloadScripts();
        return;
    }
    QMainWindow::keyPressEvent(ak_Event_);
}


void k_PipelineMainWindow::newPipeline()
{
    if (!askForSaveIfNecessary())
        return;
    mk_Desktop_->clearAll();
    mk_Desktop_->setHasUnsavedChanges(false);
    ms_PipelineFilename = QString();
    toggleUi();
}


void k_PipelineMainWindow::loadPipeline()
{
    if (!askForSaveIfNecessary())
        return;
    QString ls_Path;
    
    QString ls_StartingPath = mk_Proteomatic.getConfiguration(CONFIG_REMEMBER_PIPELINE_PATH).toString();
    if (!QFileInfo(ls_StartingPath).isDir())
        ls_StartingPath = QDir::homePath();

    ls_Path = QFileDialog::getOpenFileName(this, "Load pipeline", ls_StartingPath, "Proteomatic Pipeline (*.pipeline)");
    if (!ls_Path.isEmpty())
    {
        tk_YamlMap lk_Description = k_Yaml::parseFromFile(ls_Path).toMap();
        ms_PipelineFilename = ls_Path;
        mk_Desktop_->applyPipelineDescription(lk_Description);
        mk_Desktop_->setHasUnsavedChanges(false);
    }
}


void k_PipelineMainWindow::savePipeline()
{
    if (ms_PipelineFilename.isEmpty())
        savePipelineAs();
    else
    {
        k_Yaml::emitToFile(mk_Desktop_->pipelineDescription(), ms_PipelineFilename);
        mk_Desktop_->setHasUnsavedChanges(false);
    }
}


void k_PipelineMainWindow::savePipelineAs()
{
    QString ls_StartingPath = mk_Proteomatic.getConfiguration(CONFIG_REMEMBER_PIPELINE_PATH).toString();
    if (!QFileInfo(ls_StartingPath).isDir())
        ls_StartingPath = QDir::homePath();

    ms_PipelineFilename = QFileDialog::getSaveFileName(this, "Save pipeline as...", ls_StartingPath + "/" + (ms_PipelineFilename.isEmpty() ? "Unnamed" : QFileInfo(ms_PipelineFilename).completeBaseName()) + ".pipeline", "Proteomatic Pipeline (*.pipeline)");
    if (ms_PipelineFilename != "")
    {
        mk_Proteomatic.getConfigurationRoot()[CONFIG_REMEMBER_PIPELINE_PATH] = QFileInfo(ms_PipelineFilename).absolutePath();
        savePipeline();
    }
}


void k_PipelineMainWindow::quit()
{
    close();
}


void k_PipelineMainWindow::addScript(QAction* ak_Action_)
{
    if (!mk_Desktop_)
        return;
    
    QString ls_ScriptUri = ak_Action_->data().toString();
    mk_Desktop_->addScriptBox(ls_ScriptUri);
}


void k_PipelineMainWindow::start()
{
    if (!mk_Desktop_)
        return;
    
    mk_Desktop_->start();
}


void k_PipelineMainWindow::startUntracked()
{
    if (!mk_Desktop_)
        return;
    
    mk_Desktop_->start(false);
}


void k_PipelineMainWindow::abort()
{
    if (!mk_Desktop_)
        return;
    
    if (mk_Proteomatic.showMessageBox("Abort pipeline", "Are you sure you want to abort the pipeline?", ":/icons/dialog-warning.png", 
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
        mk_Desktop_->abort();
}


void k_PipelineMainWindow::addFileListBox()
{
    if (!mk_Desktop_)
        return;
    
    mk_Desktop_->addInputFileListBox();
}


void k_PipelineMainWindow::resetParameters()
{
    if (!mk_CurrentScriptBox_)
        return;
    mk_CurrentScriptBox_->script()->reset();
}


void k_PipelineMainWindow::showProfileManager()
{
    IScript* lk_Script_ = NULL;
    if (mk_CurrentScriptBox_)
        lk_Script_ = mk_CurrentScriptBox_->script();
    
    QSharedPointer<k_ProfileManager> lk_pProfileManager(new k_ProfileManager(mk_Proteomatic, lk_Script_, this));
    lk_pProfileManager->reset();
    if (lk_pProfileManager->exec())
    {
        if (lk_Script_)
            lk_Script_->setConfiguration(lk_pProfileManager->getGoodProfileMix());
    }
}


void k_PipelineMainWindow::showAll()
{
    if (!mk_Desktop_)
        return;
    
    mk_Desktop_->showAll();
}


void k_PipelineMainWindow::updateStatusBar()
{
    QSet<IDesktopBox*> lk_SelectedBoxes = mk_Desktop_->selectedBoxes();
    mk_StatusBarMessage_->setText("");
/*  QObject* lk_Object_ = dynamic_cast<QObject*>(mk_WatchedBoxObject_);
    if (lk_Object_ != NULL)
        disconnect(lk_Object_, NULL, this, SLOT(updateStatusBar()));*/
    mk_WatchedBoxObject_ = NULL;
    if (lk_SelectedBoxes.size() == 1)
    {
        IDesktopBox* lk_Box_ = lk_SelectedBoxes.toList().first();
        IScriptBox* lk_ScriptBox_ = dynamic_cast<IScriptBox*>(lk_Box_);
        IFileBox* lk_FileBox_ = dynamic_cast<IFileBox*>(lk_Box_);
        if (lk_ScriptBox_)
        {
            QString ls_OutputDirectory = lk_ScriptBox_->scriptOutputDirectory();
            if (ls_OutputDirectory.isEmpty())
                ls_OutputDirectory = "<i>undefined</i>";
            mk_StatusBarMessage_->setText("<b>Output directory:</b> " + QDir::toNativeSeparators(ls_OutputDirectory));
            QObject* lk_BoxObject_ = dynamic_cast<QObject*>(lk_ScriptBox_);
            //connect(lk_BoxObject_, SIGNAL(outputDirectoryChanged()), this, SLOT(updateStatusBar()));
            mk_WatchedBoxObject_ = lk_BoxObject_;
        }
        else if (lk_FileBox_)
        {
            QStringList lk_Filenames = lk_FileBox_->filenames(); 
            if (lk_Filenames.size() == 1)
            {
                QString ls_Path = lk_Filenames.first();
                mk_StatusBarMessage_->setText(QDir::toNativeSeparators(ls_Path));
                QObject* lk_BoxObject_ = dynamic_cast<QObject*>(lk_FileBox_);
                //connect(lk_BoxObject_, SIGNAL(filenamesChanged()), this, SLOT(updateStatusBar()));
                mk_WatchedBoxObject_ = lk_BoxObject_;
            }
        }
    }
}


void k_PipelineMainWindow::scriptMenuChanged()
{
    mk_AddScriptButton_->setMenu(mk_Proteomatic.proteomaticScriptsMenu());
    connect(mk_Proteomatic.proteomaticScriptsMenu(), SIGNAL(triggered(QAction*)), this, SLOT(addScript(QAction*)));
    updateWindowTitle();
}


void k_PipelineMainWindow::updateWindowTitle()
{
    QString ls_ScriptsVersion = "(using no scripts";
    if (mk_Proteomatic.scriptsVersion() != "")
        ls_ScriptsVersion = "(using scripts " + mk_Proteomatic.scriptsVersion();
    ls_ScriptsVersion += ")";
    QString ls_WindowTitle = QString("%1[*] - Proteomatic %3 %4")
        .arg(ms_PipelineFilename.isEmpty()? "Unnamed" : QFileInfo(ms_PipelineFilename).completeBaseName())
        //.arg(mk_Desktop_->hasUnsavedChanges() ? " [modified]" : "")
        .arg(mk_Proteomatic.version())
        .arg(ls_ScriptsVersion);
    setWindowTitle(ls_WindowTitle);
    setWindowModified(mk_Desktop_->hasUnsavedChanges());
}


void k_PipelineMainWindow::searchFieldPopup(const QString& as_String)
{
    QString ls_String = as_String.toLower();
    QStringList lk_Tags = ls_String.split(mk_WordSplitter, QString::SkipEmptyParts);
    QHash<QString, QStringList>& lk_Keywords = mk_Proteomatic.mk_ScriptKeywords;
    QHash<QString, int> lk_Targets;
    foreach (QString ls_Tag, lk_Tags)
    {
        foreach (QString ls_Keyword, lk_Keywords.keys())
        {
            if (ls_Keyword.startsWith(ls_Tag))
            {
                foreach (QString ls_Target, lk_Keywords[ls_Keyword])
                {
                    if (ls_Target.startsWith("script/"))
                    {
                        ls_Target.replace("script/", "");
                        QString ls_Class = ls_Target.left(ls_Target.indexOf("/"));
                        int li_ClassScore = 0;
                        if (ls_Class == "title")
                            li_ClassScore = 100;
                        else if (ls_Class == "group")
                            li_ClassScore = 10;
                        else if (ls_Class == "description")
                            li_ClassScore = 1;
                        ls_Target.replace(ls_Class + "/", "");
                        if (!lk_Targets.contains(ls_Target))
                            lk_Targets[ls_Target] = 0;
                        lk_Targets[ls_Target] += li_ClassScore;
                    }
                }
            }
        }
    }
    QMultiMap<int, QString> lk_TargetsSorted;
    foreach (QString ls_Target, lk_Targets.keys())
        lk_TargetsSorted.insert(lk_Targets[ls_Target], ls_Target);
    
    mk_pSearchPopup = QSharedPointer<QListWidget>(new QListWidget(this));
    
    if (lk_TargetsSorted.empty())
        return;
    
    QMultiMap<int, QString>::const_iterator lk_Iter = lk_TargetsSorted.constEnd();
    do
    {
        --lk_Iter;
        
        QString ls_ScriptPath = lk_Iter.value();
        QString ls_Title = mk_Proteomatic.scriptInfo(ls_ScriptPath)["title"];
        if (!ls_Title.isEmpty())
            new QListWidgetItem(QIcon("src/icons/proteomatic.png"), ls_Title, mk_pSearchPopup.data());
    } while (lk_Iter != lk_TargetsSorted.constBegin());
    
    //mk_pSearchPopup->setWindowModality(Qt::NonModal);
    QWidget* lk_Sender_ = dynamic_cast<QWidget*>(sender());
    if (lk_Sender_)
    {
        mk_pSearchPopup->move(mapFromGlobal(lk_Sender_->mapToGlobal(QPoint(0, lk_Sender_->height()))));
        mk_pSearchPopup->resize(250, 100);
        //mk_pSearchPopup->setIconSize(QSize(16, 16));
        mk_pSearchPopup->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        mk_pSearchPopup->show();
        //connect(lk_Sender_, SIGNAL(focusOut()), mk_pSearchPopup.data(), SLOT(hide()));
        //lk_Sender_->setFocus(Qt::MouseFocusReason);
    }
}


void k_PipelineMainWindow::restartProteomatic()
{
    if (askForSaveIfNecessary())
        mk_Application.exit(0x1414);
}


void k_PipelineMainWindow::toggleUi()
{
    updateWindowTitle();
    mk_NewPipelineAction_->setEnabled(mk_Desktop_ && !mk_Desktop_->running());
    mk_LoadPipelineAction_->setEnabled(mk_Desktop_ && !mk_Desktop_->running());
    mk_SavePipelineAction_->setEnabled(mk_Desktop_ && !mk_Desktop_->running() && mk_Desktop_->hasBoxes());
    mk_SavePipelineAsAction_->setEnabled(mk_Desktop_ && !mk_Desktop_->running() && mk_Desktop_->hasBoxes());
    mk_QuitAction_->setEnabled(mk_Desktop_ && !mk_Desktop_->running());
    mk_AddScriptAction_->setEnabled(mk_Desktop_ && !mk_Desktop_->running());
    mk_AddFileListAction_->setEnabled(mk_Desktop_ && !mk_Desktop_->running());
    mk_Proteomatic.startButton()->setEnabled(mk_Desktop_ && (!mk_Desktop_->running()) && (mk_Desktop_->hasBoxes()));
    mk_AbortAction_->setEnabled(mk_Desktop_ && mk_Desktop_->running());
//     mk_AbortAction_->setVisible(mk_Desktop_ && mk_Desktop_->running());
    mk_RefreshAction_->setEnabled(mk_Desktop_ && (!mk_Desktop_->running()) && (mk_Desktop_->hasBoxes()));
    mk_ProfileManagerAction_->setEnabled(mk_Desktop_ && (!mk_Desktop_->running()));
    mk_ShowConfigurationAction_->setEnabled(mk_Desktop_ && (!mk_Desktop_->running()));
    mk_ResetParametersAction_->setEnabled(mk_Desktop_ && (!mk_Desktop_->running()) && mk_CurrentScriptBox_);
    //mk_OutputPrefix_->setEnabled(mk_Desktop_ && !mk_Desktop_->running());
    //mk_ClearPrefixForAllScriptsAction_->setEnabled(mk_Desktop_ && !mk_Desktop_->running() && mk_Desktop_->hasBoxes());
    mk_ProposePrefixForAllScriptsAction_->setEnabled(mk_Desktop_ && !mk_Desktop_->running() && mk_Desktop_->hasBoxes());
    mk_LassoButton_->setEnabled(mk_Desktop_ && (!mk_Desktop_->running()));
    if (mk_Desktop_ && mk_Desktop_->running())
        mk_PanButton_->setChecked(true);
    
    mk_AddToolBar_->setToolButtonStyle(mk_Proteomatic.getConfiguration(CONFIG_APPEARANCE_SIZE).toInt() < 2 ? Qt::ToolButtonTextBesideIcon : Qt::ToolButtonIconOnly);
    QString ls_NewStyleSheet = QString("* { icon-size: %1px; }").arg((mk_Proteomatic.getConfiguration(CONFIG_APPEARANCE_SIZE).toInt() < 1) ? 20 : 16);
    if (ls_NewStyleSheet != ms_CurrentStyleSheet)
    {
        mk_Application.setStyleSheet(ls_NewStyleSheet);
        ms_CurrentStyleSheet = ls_NewStyleSheet;
    }
    if (!mk_Desktop_->hasScriptBoxes())
        mk_PaneDockWidget_->hide();
    else
        mk_PaneDockWidget_->show();
}


void k_PipelineMainWindow::togglePaneFloat()
{
    mk_PaneDockWidget_->setFloating(!mk_PaneDockWidget_->isFloating());
}


void k_PipelineMainWindow::setCurrentScriptBox(IScriptBox* ak_ScriptBox_)
{
    mk_CurrentScriptBox_ = ak_ScriptBox_;

    QLayoutItem* lk_Item_ = NULL;
    while ((lk_Item_ = mk_PaneLayout_->takeAt(0)) != NULL)
        lk_Item_->widget()->hide();
    
    if (ak_ScriptBox_)
    {
        mk_PaneLayout_->addWidget(ak_ScriptBox_->paneWidget());
        ak_ScriptBox_->paneWidget()->show();
        if (mb_JustStarted)
        {
            mb_JustStarted = false;
            mk_PaneDockWidget_->show();
/*          QList<int> lk_Sizes;
            lk_Sizes.push_back(width() - 450);
            lk_Sizes.push_back(450);
            mk_HSplitter_->setSizes(lk_Sizes);*/
            //ak_ScriptBox_->paneWidget()->resize(QSize(450, ak_ScriptBox_->paneWidget()->height()));
            //mk_PaneDockWidget_->resize(QSize(500, mk_PaneDockWidget_->height()));
            mk_Desktop_->showAll();
        }
        mk_FauxTitleBarWidgetFloatingLabel_->setText("<b>" + mk_CurrentScriptBox_->script()->title() + "</b>");
        mk_PaneDockWidget_->setWindowTitle(mk_CurrentScriptBox_->script()->title());
    }
    toggleUi();
}


bool k_PipelineMainWindow::panMode() const
{
    return mk_PanButton_->isChecked();
}


QMenu* k_PipelineMainWindow::createPopupMenu()
{
    return NULL;
}


bool k_PipelineMainWindow::askForSaveIfNecessary()
{
    if (mk_Desktop_->hasUnsavedChanges() && mk_Desktop_->hasBoxes())
    {
        // save discard cancel
        int li_Button = mk_Proteomatic.showMessageBox("Warning", "There are unsaved changes.", ":/icons/dialog-warning.png", QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save, QMessageBox::Cancel, "Do you want to save the current pipeline?");
        if (li_Button == QMessageBox::Cancel)
            return false;
        if (li_Button == QMessageBox::Save)
        {
            this->savePipelineAs();
            if (mk_Desktop_->hasUnsavedChanges())
                return false;
        }
    }
    return true;
}
