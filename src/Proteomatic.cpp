/*
Copyright (c) 2007-2010 Michael Specht

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
#include "Desktop.h"
#include "FoldedHeader.h"
#include "HintLineEdit.h"
#include "PipelineMainWindow.h"
#include "RubyWindow.h"
#include "SearchMenu.h"
#include "Yaml.h"
#include "version.h"
#include "ext/md5/md5.h"


#define DEFAULT_UPDATE_URI "http://www.proteomatic.org/update"


#ifdef Q_OS_WIN32
    #define FILE_URL_PREFIX "file:///"
#else
    #define FILE_URL_PREFIX "file://"
#endif


k_Proteomatic::k_Proteomatic(QCoreApplication& ak_Application)
    : mk_Application(ak_Application)
    , mk_Desktop_(NULL)
    , mk_PipelineMainWindow_(NULL)
    , mk_MessageBoxParent_(NULL)
    , mk_RemoteMenu_(NULL)
    , ms_RemoteHubStdout("")
    , ms_DataDirectory(".")
    , ms_ManagedScriptsPath("scripts")
    , ms_ConfigurationPath("proteomatic.conf.yaml")
    , mk_FolderEnabledIcon(":icons/folder.png")
    , mk_FolderDisabledIcon(":icons/folder-disabled.png")
    , mk_ScriptEnabledIcon(":icons/proteomatic.png")
    , mk_ScriptDisabledIcon(":icons/proteomatic-disabled.png")
{
    // data directory is home path by default
    ms_DataDirectory = QDir::homePath() + "/.proteomatic";
    
    // however, if we're a portable version, the data directory is
    // simply THIS directory... right?
    #ifdef PROTEOMATIC_PORTABLE
    ms_DataDirectory = ".";
    // Now check whether this portable data directory is also writable,
    // it should be! This way, we can make sure that Proteomatic is not
    // started from within the (write-protected because compressed)
    // disk image on Mac
    QFile lk_WriteTest(ms_DataDirectory + "/can_has_write.txt");
    if (!lk_WriteTest.open(QIODevice::WriteOnly))
    {
        QString ls_Message;
        #ifdef Q_OS_MAC
        ls_Message = "<b>Proteomatic has no permission to write files.</b><br /><br />Please drag the Proteomatic icon to the Applications shortcut or to your Desktop and try again.";
        #else
        ls_Message = "<p><b>Proteomatic has no permission to write files.</b></p><p>The directory from which Proteomatic was started is not writable. Because Proteomatic needs to download scripts and external programs, please move the Proteomatic folder to a writable location such as your Desktop and try again.</p>";
        #endif
        showMessageBox("Unable to start Proteomatic", ls_Message, ":icons/dialog-warning.png");
        exit(1);
    }
    else
        lk_WriteTest.remove();
    #endif
    
    // now turn the data directory into an absolute, clean path
    ms_DataDirectory = QFileInfo(ms_DataDirectory).absoluteFilePath();
    ms_DataDirectory = QDir::cleanPath(ms_DataDirectory);

    // create data directory if it doesn't exist 
    QDir lk_Dir;
    if (!lk_Dir.exists(ms_DataDirectory))
        lk_Dir.mkdir(ms_DataDirectory);
    
    // create ext tools directory if it doesn't exist 
    ms_ExternalToolsPath = QDir::cleanPath(ms_DataDirectory + "/ext");
    if (!lk_Dir.exists(ms_ExternalToolsPath))
        lk_Dir.mkdir(ms_ExternalToolsPath);
    ms_TempPath = QDir::cleanPath(ms_DataDirectory + "/temp");
    if (!lk_Dir.exists(ms_TempPath))
        lk_Dir.mkdir(ms_TempPath);
    ms_HelperPath = QDir::cleanPath(ms_DataDirectory + "/helper");
    if (!lk_Dir.exists(ms_HelperPath))
        lk_Dir.mkdir(ms_HelperPath);

    ms_ManagedScriptsPath = QDir::cleanPath(ms_DataDirectory + "/scripts");
    ms_ConfigurationPath = QDir::cleanPath(ms_DataDirectory + "/proteomatic.conf.yaml");
    
    mk_Languages.clear();
    mk_Languages << "ruby";
    mk_Languages << "python";
    mk_Languages << "php";
    mk_Languages << "perl";
    mk_Languages << "java";

    // write the update helper if it's not there already, or if an old version is there
    QString ls_PackagedUpdateMd5 = md5ForFile(":/update.rb");
    ms_UpdateHelperPath = QDir::cleanPath(ms_HelperPath + "/update-" + ls_PackagedUpdateMd5 + ".rb");
    QString ls_RealUpdateMd5 = md5ForFile(ms_UpdateHelperPath, false);
    if (ls_PackagedUpdateMd5 != ls_RealUpdateMd5)
    {
        QFile lk_File(ms_UpdateHelperPath);
        if (lk_File.open(QIODevice::WriteOnly))
        {
            QFile lk_Template(":/update.rb");
            if (lk_Template.open(QIODevice::ReadOnly))
            {
                lk_File.write(lk_Template.readAll());
                lk_Template.close();
            }
        }
        lk_File.close();
    }

    #ifdef Q_OS_WIN32
    // If we're on Windows, and there's no 7zip yet in the helper directory,
    // copy it over, it must be here.
    if (!QDir(ms_HelperPath + "/7zip").exists())
    {
        if (QDir(ms_HelperPath).mkdir("7zip"))
        {
            if (QDir(ms_HelperPath + "/7zip").mkdir("7za457"))
            {
                QString ls_OldPath = "./helper/7zip/7za457/";
                QString ls_NewPath = ms_HelperPath + "/7zip/7za457/";
                foreach (QString ls_Path, QDir(ls_OldPath).entryList(QDir::Files))
                    QFile::copy(ls_OldPath + ls_Path, ls_NewPath + ls_Path);
            }
        }
    }
    #endif

    // the script lock id looks like this: 4qn-l4ougm
    ms_ScriptLockId = QString("%1-%2").
        arg(QCoreApplication::applicationPid(), 0, 36).
        arg(QDateTime::currentDateTime().toTime_t(), 0, 36);
}


k_Proteomatic::~k_Proteomatic()
{
}


void k_Proteomatic::initialize()
{
    mk_StartButton_ = new QToolButton(NULL);
    mk_StartButton_->setIcon(QIcon(":icons/dialog-ok.png"));
    mk_StartButton_->setText("Start");
    mk_StartButton_->setPopupMode(QToolButton::DelayedPopup);
    mk_StartButton_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    
    mk_StartUntrackedAction_ = mk_StartButtonMenu.addAction(QIcon(":icons/dialog-ok.png"), "Start untracked");
    
    mk_FileTrackerIconLabel_ = new QLabel(NULL);
    mk_FileTrackerLabel_ = new QLabel(NULL);
    mk_FileTrackerIconLabel_->setPixmap(QPixmap(":icons/revelio.png").scaledToHeight(16, Qt::SmoothTransformation));
    
    QString ls_BasePath = mk_Application.applicationDirPath();
#ifdef PROTEOMATIC_UPDATES_ENABLED
    ls_BasePath += "/..";
#endif
    QDir::setCurrent(ls_BasePath);

    this->loadConfiguration();

    QFontDatabase lk_FontDatabase;
    QStringList lk_Fonts = QStringList() << "Consolas" << "Bitstream Vera Sans Mono" << "DejaVu Sans Mono" << "Lucida Console" << "Monaco" << "Liberation Mono" << "Courier New" << "Courier" << "Fixed" << "System";
    while (!lk_Fonts.empty())
    {
        QString ls_Font = lk_Fonts.takeFirst();
        if (lk_FontDatabase.families().contains(ls_Font))
        {
            mk_ConsoleFont = QFont(ls_Font);
            mk_ConsoleFont.setPointSizeF(mk_ConsoleFont.pointSizeF() * 0.8);
			#ifdef Q_OS_WIN32
            mk_ConsoleFont.setPointSizeF(mk_ConsoleFont.pointSizeF() * 0.9);
			#endif
			#ifdef Q_OS_MAC
            mk_ConsoleFont.setPointSizeF(mk_ConsoleFont.pointSizeF() * 1.2);
			#endif
			break;
        }
    }
    
    this->checkRuby();
    
    collectTextFileFormats(mk_OwnTextFileFormats);
    mk_OwnPlusScriptsTextFileFormats = mk_OwnTextFileFormats;
    
    collectScriptInfo();
    createProteomaticScriptsMenu();
    
    updateConfigDependentStuff();
}


void k_Proteomatic::setPipelineMainWindow(k_PipelineMainWindow* ak_PipelineMainWindow_)
{
    mk_PipelineMainWindow_ = ak_PipelineMainWindow_;
}


k_PipelineMainWindow* k_Proteomatic::pipelineMainWindow()
{
    return mk_PipelineMainWindow_;
}


void k_Proteomatic::setDesktop(k_Desktop* ak_Desktop_)
{
    mk_Desktop_ = ak_Desktop_;
}


void k_Proteomatic::checkForUpdates()
{
    if (!mk_Configuration[CONFIG_SCRIPTS_URL].toString().isEmpty())
    {
        QStringList lk_Arguments = QStringList() << "-W0" << ms_UpdateHelperPath << mk_Configuration[CONFIG_SCRIPTS_URL].toString() << "--dryrun";
        #ifndef PROTEOMATIC_PORTABLE
        lk_Arguments << "--scriptsPath" << QDir(ms_ManagedScriptsPath).absolutePath();
        #endif
        mk_pModalProcess = QSharedPointer<QProcess>(new QProcess());
        QFileInfo lk_FileInfo(lk_Arguments.first());
        mk_pModalProcess->setWorkingDirectory(lk_FileInfo.absolutePath());
        mk_pModalProcess->setProcessChannelMode(QProcess::MergedChannels);
        connect(mk_pModalProcess.data(), SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(checkForUpdatesScriptFinished()));
        
        // find parent dialog
        QWidget* lk_Parent_ = dynamic_cast<QWidget*>(sender());
        while (lk_Parent_ != NULL && dynamic_cast<QDialog*>(lk_Parent_) == NULL)
            lk_Parent_ = lk_Parent_->parentWidget();
        
        if (mk_pModalProgressDialog)
            delete mk_pModalProgressDialog;
        mk_pModalProgressDialog = QPointer<QProgressDialog>(new QProgressDialog("Checking for script updates...", "&Cancel", 0, 0, (lk_Parent_ != NULL) ? lk_Parent_ : mk_MessageBoxParent_));
        connect(mk_pModalProgressDialog.data(), SIGNAL(canceled()), this, SLOT(checkForUpdatesCanceled()));
        mk_pModalProgressDialog->setAutoClose(false);
        mk_pModalProgressDialog->setWindowTitle("Proteomatic");
        mk_pModalProgressDialog->setMinimumDuration(2000);
        mk_pModalProgressDialog->setWindowModality(Qt::ApplicationModal);
        QTimer* lk_Timer_ = new QTimer(mk_pModalProgressDialog.data());
        lk_Timer_->setSingleShot(true);
        connect(lk_Timer_, SIGNAL(timeout()), this, SLOT(checkForUpdatesProgress()));
        
        mb_ModalProcessUserRequested = lk_Parent_;

        mk_pModalProcess->start(mk_Configuration[CONFIG_PATH_TO_RUBY].toString(), lk_Arguments, QIODevice::ReadOnly | QIODevice::Unbuffered);
        mk_pModalProcess->waitForStarted();
        lk_Timer_->start(lk_Parent_ ? 0 : 2000);
    }
}


void k_Proteomatic::checkForUpdatesProgress()
{
    if (mk_pModalProcess && mk_pModalProcess->state() == QProcess::Running && mk_pModalProgressDialog)
        mk_pModalProgressDialog->setValue(1);
}


void k_Proteomatic::checkForUpdatesCanceled()
{
    if (mk_PipelineMainWindow_)
        mk_PipelineMainWindow_->setEnabled(true);
    mk_pModalProcess->kill();
    mk_pModalProcess->waitForFinished();
    if (mk_pModalProgressDialog)
        delete mk_pModalProgressDialog;
    mk_pModalProcess = QSharedPointer<QProcess>(NULL);
}


void k_Proteomatic::checkForUpdatesScriptFinished()
{
    if (mk_PipelineMainWindow_)
        mk_PipelineMainWindow_->setEnabled(true);
    if (!mk_pModalProcess)
        return;
    
    mk_pModalProgressDialog->accept();
    mk_pModalProgressDialog->hide();
    
    QString ls_Result = mk_pModalProcess->readAll().replace("\r\n", "\n");
    
    bool lb_SomethingNewAvailable = false;
    
    if (mk_pModalProcess->exitStatus() == QProcess::NormalExit && ls_Result.startsWith("CURRENT-VERSIONS\n"))
    {
        ls_Result.replace("CURRENT-VERSIONS\n", "");
        QHash<QString, QString> lk_CurrentVersions;
        foreach (QString ls_Line, ls_Result.split("\n"))
        {
            QStringList lk_Line = ls_Line.split(":");
            if (lk_Line.size() == 2)
            {
                QString ls_Package = lk_Line[0].trimmed();
                QString ls_Version = lk_Line[1].trimmed();
                lk_CurrentVersions[ls_Package] = ls_Version;
            }
        }
#ifdef PROTEOMATIC_UPDATES_ENABLED        
        if (lk_CurrentVersions.contains("proteomatic"))
        {
            QString ls_LatestVersion = lk_CurrentVersions["proteomatic"];
            QString ls_InstalledVersion = gs_ProteomaticVersion;
            if (ls_LatestVersion != ls_InstalledVersion)
            {
                lb_SomethingNewAvailable = true;
                if (this->showMessageBox("Online update", 
                    QString("A new version of Proteomatic is available.<br /> ") + 
                    "Latest version: " + ls_LatestVersion + ", installed: " + (ls_InstalledVersion.isEmpty() ? "none" : ls_InstalledVersion) + "<br />Do you want to update to the latest version?",
                    ":/icons/software-update-available.png", QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes, QMessageBox::Cancel, QString(), QString(), "Update") == QMessageBox::Yes)
                {
                    if (mk_PipelineMainWindow_ && mk_Desktop_)
                    {
                        mk_PipelineMainWindow_->newPipeline();
                        if (!mk_Desktop_->hasUnsavedChanges())
                        {
                            QStringList lk_Arguments;
                            lk_Arguments = QStringList() << "-W0" << ms_UpdateHelperPath << mk_Configuration[CONFIG_SCRIPTS_URL].toString() << "proteomatic";
                            k_RubyWindow lk_RubyWindow(*this, lk_Arguments, "Online update", ":/icons/software-update-available.png");
                            if (lk_RubyWindow.exec())
                            {
                                purgeCacheAndTempFiles();
                                
                                this->showMessageBox("Online update", 
                                    "Press OK to restart Proteomatic",
                                    ":/icons/proteomatic-pipeline.png", 
                                    QMessageBox::Ok, QMessageBox::Ok, QMessageBox::Ok);
                                
                                mk_PipelineMainWindow_->restartProteomatic();
                            }
                        }
                    }
                }
            }
        }
#endif
/*
        if (lk_CurrentVersions.contains("proteomatic"))
        {
            QString ls_LatestVersion = lk_CurrentVersions["proteomatic"];
            QString ls_InstalledVersion = gs_ProteomaticVersion;
            if (ls_LatestVersion != ls_InstalledVersion)
            {
                lb_SomethingNewAvailable = true;
                this->showMessageBox("Online update", 
                    QString("A new version of Proteomatic is available.<br /> ") + 
                    "Latest version: " + ls_LatestVersion + ", installed: " + (ls_InstalledVersion.isEmpty() ? "none" : ls_InstalledVersion) + "<br />Please go to <a href='http://www.proteomatic.org/'>www.proteomatic.org</a>.",
                    ":/icons/software-update-available.png", QMessageBox::Ok, QMessageBox::Ok, QMessageBox::Ok);
            }
        }
        */
        if (lk_CurrentVersions.contains("scripts"))
        {
            QString ls_LatestVersion = lk_CurrentVersions["scripts"];
            QString ls_InstalledVersion = findMostRecentManagedScriptPackage().replace("proteomatic-scripts-", "");
            if (ls_LatestVersion != ls_InstalledVersion)
            {
                lb_SomethingNewAvailable = true;
                QString ls_Message = QString("A new version of Proteomatic scripts is available.<br /> ") + "Latest version: " + ls_LatestVersion + ", installed: " + (ls_InstalledVersion.isEmpty() ? "none" : ls_InstalledVersion) + "<br />Do you want to update to the latest version?";
                if (ls_InstalledVersion.isEmpty())
                        ls_Message = QString("<p><b>Welcome to Proteomatic!</b></p><p>Because Proteomatic has been started for the first time, there are no scripts available yet. Do you want to download the latest scripts package now?</p>");
                if (this->showMessageBox("Online update", 
                    ls_Message,
                    ":/icons/software-update-available.png", QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes, QMessageBox::Cancel, QString(), QString(), "Update") == QMessageBox::Yes)
                {
                    if (mk_PipelineMainWindow_ && mk_Desktop_)
                    {
                        mk_PipelineMainWindow_->newPipeline();
                        if (!mk_Desktop_->hasUnsavedChanges())
                        {
                            // create scripts path if it doesn't exist
                            if (!QDir(ms_ManagedScriptsPath).exists())
                                QDir().mkpath(ms_ManagedScriptsPath);
                            
                            // remove the old lock file, if there is any
                            mk_pLockFile = QSharedPointer<k_LockFile>(NULL);
                            
                            QStringList lk_Arguments;
                            lk_Arguments = QStringList() << "-W0" << ms_UpdateHelperPath << mk_Configuration[CONFIG_SCRIPTS_URL].toString() << "scripts";
                            #ifndef PROTEOMATIC_PORTABLE
                            lk_Arguments << "--scriptsPath" << QDir(ms_ManagedScriptsPath).absolutePath();
                            #endif
                            k_RubyWindow lk_RubyWindow(*this, lk_Arguments, "Online update", ":/icons/software-update-available.png");
                            lk_RubyWindow.exec();
                            
                            purgeCacheAndTempFiles();

                            this->collectScriptInfo();
                            this->createProteomaticScriptsMenu();
                        }
                    }
                }
            }
        }
        if (!lb_SomethingNewAvailable)
        {
            if (mb_ModalProcessUserRequested)
                this->showMessageBox("Online update", 
                    "Proteomatic is up-to-date.",
                    ":/icons/dialog-ok.png", 
                    QMessageBox::Ok, QMessageBox::Ok, QMessageBox::Ok);
        }
    }
    else
    {
        if (mb_ModalProcessUserRequested)
            this->showMessageBox("Online update", 
                "Unable to fetch update information.",
                ":/icons/dialog-warning.png", 
                QMessageBox::Ok, QMessageBox::Ok, QMessageBox::Ok);
    }
    
    if (mk_pModalProgressDialog)
        delete mk_pModalProgressDialog;
    mk_pModalProcess = QSharedPointer<QProcess>(NULL);
}


void k_Proteomatic::touchScriptsLockFile()
{
    if (mk_pLockFile)
        mk_pLockFile->touch();
}


QString k_Proteomatic::interpreterForScript(QString as_Path)
{
    QString ls_Interpreter = interpreterKeyForScript(as_Path);
    if (ls_Interpreter.isEmpty())
        return "";
    
    QString ls_ConfigKey = configKeyForScriptingLanguage(ls_Interpreter);
    if (ls_ConfigKey.isEmpty())
        return "";
    
    return mk_Configuration[ls_ConfigKey].toString();
}


QString k_Proteomatic::interpreterKeyForScript(QString as_Path)
{
    QString ls_Suffix = QFileInfo(as_Path).suffix().toLower();
    if (ls_Suffix == "rb")
        return "ruby";
    else if (ls_Suffix == "py")
        return "python";
    else if (ls_Suffix.startsWith("php"))
        return "php";
    else if (ls_Suffix == "pl")
        return "perl";
    else if (ls_Suffix == "class")
        return "java";
    
    return QString();
}


bool k_Proteomatic::scriptInterpreterWorking(QString as_Language)
{
    return mk_ScriptInterpreterWorking[as_Language];
}


QStringList k_Proteomatic::availableScripts()
{
    return mk_ScriptInfo.keys();
}


QHash<QString, QVariant> k_Proteomatic::scriptInfo(QString as_ScriptPath)
{
    return mk_ScriptInfo[as_ScriptPath];
}


bool k_Proteomatic::hasScriptInfo(QString as_ScriptPath)
{
    return mk_ScriptInfo.contains(as_ScriptPath);
}


QVariant k_Proteomatic::scriptInfo(QString as_ScriptPath, QString as_Key)
{
    return mk_ScriptInfo[as_ScriptPath][as_Key];
}


QMenu* k_Proteomatic::proteomaticScriptsMenu() const
{
    return mk_pProteomaticScriptsMenu.data();
}


QString k_Proteomatic::syncRuby(QStringList ak_Arguments)
{
    QProcess lk_QueryProcess;

    QFileInfo lk_FileInfo(ak_Arguments.first());
    lk_QueryProcess.setWorkingDirectory(lk_FileInfo.absolutePath());
    QString ls_Script = QFileInfo(ak_Arguments.takeFirst()).fileName();
    ak_Arguments.insert(0, ls_Script);
    lk_QueryProcess.setProcessChannelMode(QProcess::MergedChannels);
    lk_QueryProcess.start(mk_Configuration[CONFIG_PATH_TO_RUBY].toString(), ak_Arguments, QIODevice::ReadOnly | QIODevice::Unbuffered);
    if (lk_QueryProcess.waitForFinished())
        return lk_QueryProcess.readAll();
    else
        return QString();
}


QString k_Proteomatic::syncScript(QStringList ak_Arguments)
{
    QProcess lk_QueryProcess;

    QFileInfo lk_FileInfo(ak_Arguments.first());
    lk_QueryProcess.setWorkingDirectory(lk_FileInfo.absolutePath());
    QString ls_Script = QFileInfo(ak_Arguments.takeFirst()).fileName();
    ak_Arguments.insert(0, ls_Script);
    QString ls_InterpreterKey = interpreterKeyForScript(ak_Arguments.first());
    if (ls_InterpreterKey != "ruby")
    {
        ak_Arguments.insert(1, "--pathToRuby");
        ak_Arguments.insert(2, scriptInterpreterAbsoluteNativePath("ruby"));
    }
    lk_QueryProcess.setProcessChannelMode(QProcess::MergedChannels);
    lk_QueryProcess.start(interpreterForScript(ak_Arguments.first()), ak_Arguments, QIODevice::ReadOnly | QIODevice::Unbuffered);
    if (lk_QueryProcess.waitForFinished())
        return lk_QueryProcess.readAll();
    else
        return QString();
}


QString k_Proteomatic::syncScriptNoFile(QStringList ak_Arguments, QString as_Language, bool ab_AddPathToRuby)
{
    QProcess lk_QueryProcess;

    // :UGLY: specify proper pre-filename arguments
    int li_Index = 0;
    if (ak_Arguments[0] == "-W0")
        ++li_Index;
    
    QFileInfo lk_FileInfo(ak_Arguments[li_Index]);
    lk_QueryProcess.setWorkingDirectory(lk_FileInfo.absolutePath());
    lk_QueryProcess.setProcessChannelMode(QProcess::MergedChannels);
    
    if (ab_AddPathToRuby && (as_Language != "ruby"))
    {
        ak_Arguments.insert(1, "--pathToRuby");
        ak_Arguments.insert(2, scriptInterpreterAbsoluteNativePath("ruby"));
    }
    lk_QueryProcess.start(scriptInterpreter(as_Language), ak_Arguments, QIODevice::ReadOnly | QIODevice::Unbuffered);
    if (lk_QueryProcess.waitForFinished())
        return lk_QueryProcess.readAll();
    else
        return QString();
}


bool k_Proteomatic::syncShowRuby(QStringList ak_Arguments, QString as_Title)
{
    QSharedPointer<k_RubyWindow> lk_pRubyWindow(new k_RubyWindow(*this, ak_Arguments, as_Title));
    return lk_pRubyWindow->exec();
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
    if (QFile(ms_ConfigurationPath).exists())
        mk_Configuration = k_Yaml::parseFromFile(ms_ConfigurationPath).toMap();
        
    // insert default values
    bool lb_InsertedDefaultValue = false;
    if (!mk_Configuration.contains(CONFIG_PATH_TO_RUBY) || mk_Configuration[CONFIG_PATH_TO_RUBY].type() != QVariant::String)
    {
        mk_Configuration[CONFIG_PATH_TO_RUBY] = "ruby";
        lb_InsertedDefaultValue = true;
    }
    if (!mk_Configuration.contains(CONFIG_PATH_TO_PYTHON) || mk_Configuration[CONFIG_PATH_TO_PYTHON].type() != QVariant::String)
    {
        mk_Configuration[CONFIG_PATH_TO_PYTHON] = "python";
        lb_InsertedDefaultValue = true;
    }
    if (!mk_Configuration.contains(CONFIG_PATH_TO_PHP) || mk_Configuration[CONFIG_PATH_TO_PHP].type() != QVariant::String)
    {
        mk_Configuration[CONFIG_PATH_TO_PHP] = "php";
        lb_InsertedDefaultValue = true;
    }
    if (!mk_Configuration.contains(CONFIG_PATH_TO_PERL) || mk_Configuration[CONFIG_PATH_TO_PERL].type() != QVariant::String)
    {
        mk_Configuration[CONFIG_PATH_TO_PERL] = "perl";
        lb_InsertedDefaultValue = true;
    }
    if (!mk_Configuration.contains(CONFIG_PATH_TO_JAVA) || mk_Configuration[CONFIG_PATH_TO_JAVA].type() != QVariant::String)
    {
        mk_Configuration[CONFIG_PATH_TO_JAVA] = "java";
        lb_InsertedDefaultValue = true;
    }
    if (!mk_Configuration.contains(CONFIG_FILETRACKER_URL) || mk_Configuration[CONFIG_FILETRACKER_URL].type() != QVariant::String)
    {
        mk_Configuration[CONFIG_FILETRACKER_URL] = "";
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
    if (!mk_Configuration.contains(CONFIG_REMEMBER_PROFILE_PATH) || mk_Configuration[CONFIG_REMEMBER_PROFILE_PATH].type() != QVariant::String || !QFileInfo(mk_Configuration[CONFIG_REMEMBER_PROFILE_PATH].toString()).exists())
    {
        mk_Configuration[CONFIG_REMEMBER_PROFILE_PATH] = QDir::homePath();
        lb_InsertedDefaultValue = true;
    }
    if (!mk_Configuration.contains(CONFIG_REMEMBER_PIPELINE_PATH) || mk_Configuration[CONFIG_REMEMBER_PIPELINE_PATH].type() != QVariant::String || !QFileInfo(mk_Configuration[CONFIG_REMEMBER_PIPELINE_PATH].toString()).exists())
    {
        mk_Configuration[CONFIG_REMEMBER_PIPELINE_PATH] = QDir::homePath();
        lb_InsertedDefaultValue = true;
    }
    if (!mk_Configuration.contains(CONFIG_REMEMBER_INPUT_FILES_PATH) || mk_Configuration[CONFIG_REMEMBER_INPUT_FILES_PATH].type() != QVariant::String || !QFileInfo(mk_Configuration[CONFIG_REMEMBER_INPUT_FILES_PATH].toString()).exists())
    {
        mk_Configuration[CONFIG_REMEMBER_INPUT_FILES_PATH] = QDir::homePath();
        lb_InsertedDefaultValue = true;
    }
    if (!mk_Configuration.contains(CONFIG_REMEMBER_OUTPUT_PATH) || mk_Configuration[CONFIG_REMEMBER_OUTPUT_PATH].type() != QVariant::String || !QFileInfo(mk_Configuration[CONFIG_REMEMBER_OUTPUT_PATH].toString()).exists())
    {
        mk_Configuration[CONFIG_REMEMBER_OUTPUT_PATH] = QDir::homePath();
        lb_InsertedDefaultValue = true;
    }
    if (!mk_Configuration.contains(CONFIG_SCRIPTS_URL) || mk_Configuration[CONFIG_SCRIPTS_URL].type() != QVariant::String)
    {
        mk_Configuration[CONFIG_SCRIPTS_URL] = DEFAULT_UPDATE_URI;
        lb_InsertedDefaultValue = true;
    }
    if (!mk_Configuration.contains(CONFIG_FOLLOW_NEW_BOXES) || mk_Configuration[CONFIG_FOLLOW_NEW_BOXES].type() != QVariant::String)
    {
        mk_Configuration[CONFIG_FOLLOW_NEW_BOXES] = "yes";
        lb_InsertedDefaultValue = true;
    }
    if (!mk_Configuration.contains(CONFIG_ANIMATION) || mk_Configuration[CONFIG_ANIMATION].type() != QVariant::String)
    {
        mk_Configuration[CONFIG_ANIMATION] = "yes";
        lb_InsertedDefaultValue = true;
    }
    if (!mk_Configuration.contains(CONFIG_APPEARANCE_SIZE) || mk_Configuration[CONFIG_APPEARANCE_SIZE].type() != QVariant::String)
    {
        mk_Configuration[CONFIG_APPEARANCE_SIZE] = "1";
        lb_InsertedDefaultValue = true;
    }
    if (mk_Configuration.contains(CONFIG_ADDITIONAL_SCRIPT_PATHS))
    {
        if (mk_Configuration[CONFIG_ADDITIONAL_SCRIPT_PATHS].type() != QVariant::List)
        {
            // if a single script path is defined as a string, upgrade to string array!
            tk_YamlSequence lk_Paths;
            if (mk_Configuration[CONFIG_ADDITIONAL_SCRIPT_PATHS].type() == QVariant::String)
                lk_Paths << mk_Configuration[CONFIG_ADDITIONAL_SCRIPT_PATHS].toString();
            mk_Configuration[CONFIG_ADDITIONAL_SCRIPT_PATHS] = lk_Paths;
            lb_InsertedDefaultValue = true;
        }
    }
    else
    {
        tk_YamlSequence lk_Paths;
        mk_Configuration[CONFIG_ADDITIONAL_SCRIPT_PATHS] = lk_Paths;
        lb_InsertedDefaultValue = true;
    }
    // RecentPipeInput
    if (mk_Configuration.contains(CONFIG_RECENT_PIPELINES))
    {
        if (mk_Configuration[CONFIG_RECENT_PIPELINES].type() != QVariant::List)
        {
            // if a single pipe path is defined as a string, upgrade to string array!
            tk_YamlSequence lk_Paths;
            if (mk_Configuration[CONFIG_RECENT_PIPELINES].type() == QVariant::String)
                lk_Paths << mk_Configuration[CONFIG_RECENT_PIPELINES].toString();
            mk_Configuration[CONFIG_RECENT_PIPELINES] = lk_Paths;
            lb_InsertedDefaultValue = true;
        }
    }
    else
    {
        tk_YamlSequence lk_Paths;
        mk_Configuration[CONFIG_RECENT_PIPELINES] = lk_Paths;
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
    
    if (!mk_Configuration[CONFIG_ADDITIONAL_SCRIPT_PATHS].toList().empty())
    {
        mk_AdditionalScriptPaths = QStringList();
        foreach (QVariant lk_Variant, mk_Configuration[CONFIG_ADDITIONAL_SCRIPT_PATHS].toList())
            mk_AdditionalScriptPaths << lk_Variant.toString();
    }
    
    // write user configuration if it doesn't already exist
    if (lb_InsertedDefaultValue)
    {
        this->saveConfiguration();
        updateConfigDependentStuff();
    }
}


void k_Proteomatic::collectTextFileFormats(QMap<QString, QPair<QString, QStringList> >& ak_Results, QString as_Path)
{
    QFile lk_File(as_Path + "/text-formats.txt");
    if (lk_File.open(QIODevice::ReadOnly))
    {
        QTextStream lk_Stream(&lk_File);
        while (!lk_Stream.atEnd())
        {
            QString ls_Key = lk_Stream.readLine().trimmed();
            if (ls_Key.isEmpty())
                continue;
            QFile lk_FormatFile(QString(as_Path + "/formats/%1.yaml").arg(ls_Key));
            if (lk_FormatFile.exists())
            {
                QVariant lk_Info = k_Yaml::parseFromFile(lk_FormatFile.fileName());
                /*
                extensions: ['.txt']
                description: Plain text
                */
                QStringList lk_Extensions;
                QString ls_Description;
                tk_YamlMap lk_Description = lk_Info.toMap();
                if (lk_Description.contains("extensions"))
                {
                    foreach (QVariant lk_Item, lk_Description["extensions"].toList())
                    {
                        QString ls_Extension = lk_Item.toString().trimmed();
                        if (ls_Extension.isEmpty())
                            continue;
                        if (!ls_Extension.startsWith("."))
                            ls_Extension = "." + ls_Extension;
                        lk_Extensions << ls_Extension;
                    }
                }
                if (lk_Description.contains("description"))
                    ls_Description = lk_Description["description"].toString();
                if ((!ls_Description.isEmpty()) && (!lk_Extensions.empty()))
                {
                    QString ls_LowerDescription = ls_Description.toLower();
                    if (!ak_Results.contains(ls_LowerDescription))
                        ak_Results[ls_LowerDescription] = QPair<QString, QStringList>(ls_Description, QStringList());
                    foreach (QString ls_Extension, lk_Extensions)
                    {
                        if (!ak_Results[ls_LowerDescription].second.contains(ls_Extension))
                            ak_Results[ls_LowerDescription].second << ls_Extension;
                    }
                }
            }
        }
    }
    foreach (QString ls_Key, ak_Results.keys())
        qSort(ak_Results[ls_Key].second.begin(), ak_Results[ls_Key].second.end());
}


void k_Proteomatic::collectScriptInfo(bool ab_ShowImmediately)
{
    mk_ScriptInfo.clear();
    QStringList lk_Scripts;
    QString ls_CurrentPackage = findMostRecentManagedScriptPackage();
    if (!ls_CurrentPackage.isEmpty())
    {
        QString ls_Path = ms_ManagedScriptsPath + "/" + ls_CurrentPackage;
        QDir lk_Dir(ls_Path);
        QStringList lk_Paths = lk_Dir.entryList(QStringList() << "*.rb" << "*.py" << "*.php" << "*.php5" << "*.php4" << ".pl" << ".class", QDir::Files);
        foreach (QString ls_Path, lk_Paths)
            lk_Scripts << lk_Dir.cleanPath(lk_Dir.absoluteFilePath(ls_Path));
        
        // write the scripts lock ... and don't forget to delete it later.
        // if the program crashes, and the lock stays there it only means
        // that the scripts package will not be deleted after updating to
        // a newer version until the lock is at least (variable, but maybe
        // one week) old.
        QDir lk_LockDir(ls_Path + "/.lock");
        if (!lk_LockDir.exists())
            QDir(ls_Path).mkdir(".lock");
        QString ls_LockFilePath = ls_Path + "/.lock/" + ms_ScriptLockId; 
        mk_pLockFile = QSharedPointer<k_LockFile>(new k_LockFile(ls_LockFilePath));
    }
    
    // collect scripts text file formats
    mk_OwnPlusScriptsTextFileFormats = mk_OwnTextFileFormats;
    // :TODO: maybe search for the cli-tools-directory
    collectTextFileFormats(mk_OwnPlusScriptsTextFileFormats, ms_ManagedScriptsPath + "/" + ls_CurrentPackage + "/include/cli-tools-atlas");

    foreach (QString ls_ScriptPath, mk_AdditionalScriptPaths)
    {
        QDir lk_Dir(ls_ScriptPath);
        QStringList lk_Paths = lk_Dir.entryList(QStringList() << "*.rb" << "*.py" << "*.php" << "*.php5" << "*.php4" << ".pl" << ".class", QDir::Files);
        foreach (QString ls_Path, lk_Paths)
            lk_Scripts << lk_Dir.cleanPath(lk_Dir.absoluteFilePath(ls_Path));
    }
    
    if (lk_Scripts.empty())
      return;
    
    QProgressDialog lk_ProgressDialog("Collecting scripts...", "", 0, lk_Scripts.size(), mk_MessageBoxParent_);
    lk_ProgressDialog.setCancelButton(0);
    lk_ProgressDialog.setWindowTitle("Proteomatic");
    lk_ProgressDialog.setWindowIcon(QIcon(":icons/proteomatic.png"));
    lk_ProgressDialog.setMinimumDuration(ab_ShowImmediately ? 0 : 2000);
    if (ab_ShowImmediately)
        lk_ProgressDialog.show();
    int li_Count = 0;
    mk_ScriptKeywords.clear();
    QRegExp lk_WordSplitter("\\W+");

    foreach (QString ls_Path, lk_Scripts)
    {
        QCoreApplication::processEvents();
        ++li_Count;
        lk_ProgressDialog.setValue(li_Count);
        if (ls_Path.contains(".defunct."))
            continue;
        QString ls_Title = "";
        QString ls_Group = "";
        QString ls_Description = "";

        QFile lk_File(ls_Path);
        lk_File.open(QIODevice::ReadOnly);
        QString ls_Marker;
        QByteArray lk_Head = lk_File.read(1024);
        bool lb_SeenProteomatic =  lk_Head.toLower().contains("proteomatic");
        lk_File.close();
            
        if (lb_SeenProteomatic)
        {
            if (getConfiguration(CONFIG_CACHE_SCRIPT_INFO).toBool() && (!QFile::exists(QDir::cleanPath(ms_DataDirectory + "/cache"))))
            {
                QDir lk_Dir;
                lk_Dir.mkdir(QDir::cleanPath(ms_DataDirectory + "/cache"));
            }

            QFileInfo lk_FileInfo(ls_Path);
            QString ls_Response;
            QString ls_CacheFilename = QDir::cleanPath(ms_DataDirectory + "/cache" + QString("/%1.%2.info").arg(lk_FileInfo.fileName()).arg(md5ForString(ls_Path)));
            bool lb_UseCache = getConfiguration(CONFIG_CACHE_SCRIPT_INFO).toBool() && fileUpToDate(ls_CacheFilename, QStringList() << ls_Path);
            
            if (lb_UseCache)
            {
                // see if cached info showed no errors or unresolved dependencies
                QFile lk_File(ls_CacheFilename);
                lk_File.open(QIODevice::ReadOnly);
                QTextStream lk_Stream(&lk_File);
                QString ls_Line = lk_Stream.readLine().trimmed();
                if (ls_Line != "---yamlInfo")
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
                // first check whether a cached yamlinfo is already in the scripts package
                QString ls_ScriptCachePath = QFileInfo(lk_FileInfo.absolutePath() + "/cache/" + lk_FileInfo.fileName() + ".short.yamlinfo").filePath();
                if (QFileInfo(ls_ScriptCachePath).exists())
                {
                    QFile lk_File(ls_ScriptCachePath);
                    lk_File.open(QIODevice::ReadOnly);
                    QTextStream lk_Stream(&lk_File);
                    ls_Response = lk_Stream.readAll();
                    lk_File.close();
                }
                else
                {
                    QProcess lk_QueryProcess;
                    lk_QueryProcess.setWorkingDirectory(lk_FileInfo.absolutePath());
                    QStringList lk_Arguments;
                    lk_Arguments << ls_Path << "---yamlInfo" << "--short";
                    // ignore Ruby warnings for ---yamlInfo
                    if (interpreterKeyForScript(ls_Path) == "ruby")
                        lk_Arguments.insert(0, "-W0");
                    else
                    {
                        lk_Arguments.insert(1, "--pathToRuby");
                        lk_Arguments.insert(2, scriptInterpreterAbsoluteNativePath("ruby"));
                    }
                    lk_QueryProcess.setProcessChannelMode(QProcess::MergedChannels);
                    lk_QueryProcess.start(interpreterForScript(ls_Path), lk_Arguments, QIODevice::ReadOnly | QIODevice::Unbuffered);
                    if (lk_QueryProcess.waitForFinished())
                        ls_Response = lk_QueryProcess.readAll();
                }
                
                if (!ls_Response.isEmpty())
                {
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
            ls_Response.replace("---yamlInfo\r\n", "---yamlInfo\n");
            if (ls_Response.startsWith("---yamlInfo\n"))
            {
                ls_Response = ls_Response.right(ls_Response.length() - QString("---yamlInfo\n").length());
                QVariant lk_Response = k_Yaml::parseFromString(ls_Response);
                if (lk_Response.canConvert<tk_YamlMap>())
                {
                    QHash<QString, QVariant> lk_Script;
                    
                    QString ls_Title = lk_Response.toMap()["title"].toString();
                    QString ls_Group = lk_Response.toMap()["group"].toString();
                    QString ls_Description = lk_Response.toMap()["description"].toString();
                    lk_Script["title"] = ls_Title;
                    lk_Script["group"] = ls_Group;
                    lk_Script["description"] = ls_Description;
                    lk_Script["input"] = lk_Response.toMap()["input"];
                    lk_Script["uri"] = ls_Path;
                    mk_ScriptInfo.insert(ls_Path, lk_Script);
                    
                    QStringList lk_Tags = ls_Title.split(lk_WordSplitter, QString::SkipEmptyParts);
                    foreach (QString ls_Tag, lk_Tags)
                    {
                        ls_Tag = ls_Tag.toLower();
                        if (!mk_ScriptKeywords.contains(ls_Tag))
                            mk_ScriptKeywords[ls_Tag] = QStringList();
                        mk_ScriptKeywords[ls_Tag] << "script/title/" + ls_Path;
                    }
                    lk_Tags = ls_Group.split(lk_WordSplitter, QString::SkipEmptyParts);
                    foreach (QString ls_Tag, lk_Tags)
                    {
                        ls_Tag = ls_Tag.toLower();
                        if (!mk_ScriptKeywords.contains(ls_Tag))
                            mk_ScriptKeywords[ls_Tag] = QStringList();
                        mk_ScriptKeywords[ls_Tag] << "script/group/" + ls_Path;
                    }
                    lk_Tags = ls_Description.split(lk_WordSplitter, QString::SkipEmptyParts);
                    foreach (QString ls_Tag, lk_Tags)
                    {
                        ls_Tag = ls_Tag.toLower();
                        if (!mk_ScriptKeywords.contains(ls_Tag))
                            mk_ScriptKeywords[ls_Tag] = QStringList();
                        mk_ScriptKeywords[ls_Tag] << "script/description/" + ls_Path;
                    }
                }
            }
        }
    }
    if (mk_PipelineMainWindow_)
        mk_PipelineMainWindow_->toggleUi();
}


void k_Proteomatic::createProteomaticScriptsMenu()
{
    k_SearchMenu* lk_Menu_ = new k_SearchMenu(*this, NULL);
    QHash<QString, QMenu* > lk_GroupMenus;
    lk_GroupMenus[""] = lk_Menu_;

    QMap<QString, QString> lk_ScriptOrder; // title => script path
    QSet<QString> lk_Groups;
    QStringList lk_Scripts = availableScripts();
    foreach (QString ls_Path, lk_Scripts)
    {
        QHash<QString, QVariant> lk_ScriptInfo = scriptInfo(ls_Path);
        lk_ScriptOrder.insert(lk_ScriptInfo["title"].toString(), ls_Path);
        lk_Groups.insert(lk_ScriptInfo["group"].toString());
    }
    QList<QString> lk_GroupKeys = lk_Groups.toList();
    qSort(lk_GroupKeys);
    
    mk_ExtensionsForScriptsMenuSubMenu.clear();
    mk_ExtensionsForScriptsMenuAction.clear();
    mk_ExtensionsForScriptPath.clear();

    // create sub menus
    foreach (QString ls_Group, lk_GroupKeys)
    {
        QStringList lk_GroupElements = ls_Group.split("/");
        QString ls_IncPath = "";
        QMenu* lk_ParentMenu_ = lk_Menu_;
        mk_ExtensionsForScriptsMenuSubMenu[lk_ParentMenu_] = QSet<QString>();
        foreach (QString ls_GroupElement, lk_GroupElements)
        {
            if (!ls_IncPath.isEmpty())
                ls_IncPath += "/";
            ls_IncPath += ls_GroupElement;
            if (!lk_GroupMenus.contains(ls_IncPath))
            {
                QMenu* lk_SubMenu_ = new QMenu(ls_GroupElement, lk_ParentMenu_);
                lk_SubMenu_->setIcon(mk_FolderEnabledIcon);
                lk_ParentMenu_->addMenu(lk_SubMenu_);
                lk_GroupMenus[ls_IncPath] = lk_SubMenu_;
                mk_ExtensionsForScriptsMenuSubMenu[lk_SubMenu_] = QSet<QString>();
            }
            lk_ParentMenu_ = lk_GroupMenus[ls_IncPath];
        }
    }
    
//     mk_RemoteMenu_ = new QMenu("Remote", lk_Menu_);
//     mk_RemoteMenu_->setIcon(QIcon(":/icons/applications-internet.png"));
//     mk_RemoteMenu_->setEnabled(false);
//     
//     rebuildRemoteScriptsMenu();

    // insert menu entries
    foreach (QString ls_Title, lk_ScriptOrder.keys())
    {
        QString ls_Path = lk_ScriptOrder[ls_Title];
        QHash<QString, QVariant> lk_ScriptInfo = scriptInfo(ls_Path);
        QString ls_Group = lk_ScriptInfo["group"].toString();
        QMenu* lk_TargetMenu_ = lk_GroupMenus[ls_Group];
        QAction* lk_Action_ = new QAction(mk_ScriptEnabledIcon, ls_Title, lk_TargetMenu_);
        QTextDocument doc;
        doc.setHtml(lk_ScriptInfo["description"].toString());
        doc.setHtml(doc.toPlainText());
        lk_Action_->setStatusTip(doc.toPlainText());
        lk_Action_->setData(lk_ScriptInfo["uri"].toString());
        mk_ExtensionsForScriptsMenuAction[lk_Action_] = QSet<QString>();
        mk_ExtensionsForScriptPath[ls_Path] = QSet<QString>();
        QSet<QString> lk_ThisExtensionsSet;
        foreach (QVariant lk_InputItem, lk_ScriptInfo["input"].toList())
        {
            tk_YamlMap lk_InputItemMap = lk_InputItem.toMap();
            lk_ThisExtensionsSet |= lk_InputItemMap["extensions"].toString().split("/").toSet();
        }
        mk_ExtensionsForScriptsMenuAction[lk_Action_] |= lk_ThisExtensionsSet;
        mk_ExtensionsForScriptPath[ls_Path] |= lk_ThisExtensionsSet;
        QStringList lk_Group = ls_Group.split("/");
        for (int i = 1; i <= lk_Group.size(); ++i)
        {
            QMenu* lk_TempMenu_ = lk_GroupMenus[QStringList(lk_Group.mid(0, i)).join("/")];
            mk_ExtensionsForScriptsMenuSubMenu[lk_TempMenu_] |= lk_ThisExtensionsSet;
        }
        lk_TargetMenu_->addAction(lk_Action_);
        connect(lk_Action_, SIGNAL(triggered()), this, SLOT(scriptMenuScriptClickedInternal()));
    }
    
    lk_Menu_->addSearchField();
//     lk_Menu_->addSeparator();
//     lk_Menu_->addAction(mk_pSearchWidgetAction.data());

// disable remote scripts as of now!
//     lk_Menu_->addSeparator();
//      lk_Menu_->addMenu(mk_RemoteMenu_);

    mk_pProteomaticScriptsMenu = QSharedPointer<k_SearchMenu>(lk_Menu_);
    emit scriptMenuChanged();
// 
//  ms_RemoteHubStdout = "";
//  mk_pRemoteHubHttp = QSharedPointer<QHttp>(NULL);

    // (re-)start remote hub
/*  QFileInfo lk_FileInfo(ms_ScriptPath + "/" + ms_ScriptPackage + "/remote.rb");
    mk_pRemoteHubProcess = QSharedPointer<QProcess>(new QProcess());
    connect(mk_pRemoteHubProcess.data(), SIGNAL(readyRead()), this, SLOT(remoteHubReadyReadSlot()));
    mk_pRemoteHubProcess->setWorkingDirectory(lk_FileInfo.absolutePath());
    mk_pRemoteHubProcess->setProcessChannelMode(QProcess::MergedChannels);
    mk_pRemoteHubProcess->start(mk_Configuration[CONFIG_PATH_TO_RUBY].toString(), QStringList() << "remote.rb" << "--hub", QIODevice::ReadOnly | QIODevice::Unbuffered);*/
}


int k_Proteomatic::queryRemoteHub(QString /*as_Uri*/, QStringList /*ak_Arguments*/)
{
/*  if (mk_pRemoteHubHttp.data() == NULL)
        return -1;
        
    QString ls_Arguments = QString("%1\r\n").arg(as_Uri);
    foreach (QString ls_Argument, ak_Arguments)
        ls_Arguments += QString("%1\r\n").arg(ls_Argument);
    
    return mk_pRemoteHubHttp->post("/", ls_Arguments.toAscii());*/
    return 0;
}


QFont& k_Proteomatic::consoleFont()
{
    return mk_ConsoleFont;
}


QString k_Proteomatic::tempPath() const
{
    return ms_TempPath;
}

void k_Proteomatic::checkScriptingLanguages(QString as_Language/* = QString()*/)
{
    QStringList lk_Languages = mk_Languages;
    if (!as_Language.isEmpty())
    {
        lk_Languages.clear();
        lk_Languages << as_Language;
    }
    foreach (QString ls_Language, lk_Languages)
    {
        QString ls_Key = configKeyForScriptingLanguage(ls_Language);
        QString ls_GetVersionSwitch = "--version";
        if (ls_Language == "java")
            ls_GetVersionSwitch = "-version";
        QString ls_Result = syncScriptNoFile(QStringList() << ls_GetVersionSwitch, ls_Language, false).toLower();
        if (ls_Language == "perl")
        {
            ls_Result.replace("this is", "");
            ls_Result = ls_Result.trimmed();
        }
        mk_ScriptInterpreterWorking[ls_Language] = ls_Result.startsWith(ls_Language);
    }
}


QString k_Proteomatic::dataDirectory() const
{
    return ms_DataDirectory;
}


/*
QStringList k_Proteomatic::scriptPaths() const
{
    return mk_ScriptPaths;
}


QString k_Proteomatic::scriptPathAndPackage() const
{
    return mk_ScriptPaths.first() + "/" + ms_ScriptPackage;
}
*/


int k_Proteomatic::showMessageBox(QString as_Title, QString as_Text, QString as_Icon, 
                                  QMessageBox::StandardButtons ae_Buttons, QMessageBox::StandardButton ae_DefaultButton, 
                                  QMessageBox::StandardButton ae_EscapeButton,
                                  QString as_InformativeText, QString as_DetailedText,
                                  QString as_OverrideYesText, QString as_OverrideNoText)

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
    if ((ae_Buttons & QMessageBox::Yes) && !as_OverrideYesText.isEmpty())
        lk_MessageBox.button(QMessageBox::Yes)->setText(as_OverrideYesText);
    if ((ae_Buttons & QMessageBox::No) && !as_OverrideNoText.isEmpty())
        lk_MessageBox.button(QMessageBox::No)->setText(as_OverrideNoText);
    if (!as_InformativeText.isEmpty())
        lk_MessageBox.setInformativeText(as_InformativeText);
    if (!as_DetailedText.isEmpty())
        lk_MessageBox.setDetailedText(as_DetailedText);
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


void k_Proteomatic::showConfigurationDialog()
{
    QDialog* lk_Dialog_ = new QDialog(mk_MessageBoxParent_);
    lk_Dialog_->setWindowTitle("Preferences");
    lk_Dialog_->setWindowIcon(QIcon(":icons/proteomatic.png"));

    QWidget* lk_LayoutWidget_ = new QWidget(lk_Dialog_);
    QBoxLayout* lk_VLayout_ = new QVBoxLayout(lk_LayoutWidget_);
    lk_LayoutWidget_->setLayout(lk_VLayout_);
    
    QScrollArea* lk_ScrollArea_ = new QScrollArea(lk_Dialog_);
    lk_ScrollArea_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    lk_ScrollArea_->setWidgetResizable(true);
    lk_ScrollArea_->setWidget(lk_LayoutWidget_);
    lk_ScrollArea_->setFrameStyle(QFrame::NoFrame);
    
    QVBoxLayout* lk_MainLayout_ = new QVBoxLayout(lk_Dialog_);
    lk_MainLayout_->setContentsMargins(0, 0, 0, 0);

    /*
    QListWidget* lk_ListWidget_ = new QListWidget(lk_Dialog_);
    lk_ListWidget_->setUniformItemSizes(true);
    lk_ListWidget_->setIconSize(QSize(32, 32));
    QListWidgetItem* lk_Item_ = NULL;
    lk_Item_ = new QListWidgetItem(QIcon(":/icons/proteomatic-pipeline.png"), "Proteomatic", lk_ListWidget_);
    lk_Item_ = new QListWidgetItem(QIcon(":/icons/proteomatic-pipeline.png"), "Interpreters", lk_ListWidget_);
    lk_Item_ = new QListWidgetItem(QIcon(":/icons/proteomatic-pipeline.png"), "User interface", lk_ListWidget_);
    lk_Item_ = new QListWidgetItem(QIcon(":/icons/proteomatic-pipeline.png"), "Persistence", lk_ListWidget_);
    lk_Item_ = new QListWidgetItem(QIcon(":/icons/proteomatic-pipeline.png"), "Miscellaneous", lk_ListWidget_);
    lk_MainLayout_->addWidget(lk_ListWidget_);
    */
    
    lk_MainLayout_->addWidget(lk_ScrollArea_);
    
    QBoxLayout* lk_HLayout_ = NULL;
    QFrame* lk_Frame_ = NULL;
    
    lk_VLayout_->addWidget(new QLabel("<b>Proteomatic</b>", lk_Dialog_));

    lk_HLayout_ = new QHBoxLayout(NULL);
    lk_HLayout_->addWidget(new QLabel("Update URI:", lk_Dialog_));
    QLineEdit* lk_ScriptsUrlLineEdit_ = new QLineEdit(lk_Dialog_);
    lk_ScriptsUrlLineEdit_->setText(getConfiguration(CONFIG_SCRIPTS_URL).toString());
    lk_ScriptsUrlLineEdit_->home(false);
    lk_HLayout_->addWidget(lk_ScriptsUrlLineEdit_);
    lk_VLayout_->addLayout(lk_HLayout_);
    
    lk_HLayout_ = new QHBoxLayout(NULL);
    QCheckBox* lk_AutoCheckForUpdates_ = new QCheckBox("Check for updates on startup", lk_Dialog_);
    lk_AutoCheckForUpdates_->setCheckState(getConfiguration(CONFIG_AUTO_CHECK_FOR_UPDATES).toBool() ? Qt::Checked : Qt::Unchecked);
    QPushButton* lk_CheckNowButton_ = new QPushButton("Check now", lk_Dialog_);
    connect(lk_CheckNowButton_, SIGNAL(clicked()), this, SLOT(checkForUpdates()));
    lk_HLayout_->addWidget(lk_AutoCheckForUpdates_);
    lk_HLayout_->addStretch();
    lk_HLayout_->addWidget(lk_CheckNowButton_);
    
    lk_VLayout_->addLayout(lk_HLayout_);
    
    lk_HLayout_ = new QHBoxLayout(NULL);
    lk_HLayout_->addWidget(new QLabel("Filetracker URL:", lk_Dialog_));
    QLineEdit* lk_FileTrackerUrlLineEdit_ = new QLineEdit(lk_Dialog_);
    lk_FileTrackerUrlLineEdit_->setText(getConfiguration(CONFIG_FILETRACKER_URL).toString());
    lk_FileTrackerUrlLineEdit_->home(false);
    lk_HLayout_->addWidget(lk_FileTrackerUrlLineEdit_);
    lk_VLayout_->addLayout(lk_HLayout_);

    lk_Frame_ = new QFrame(lk_Dialog_);
    lk_Frame_->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    lk_VLayout_->addWidget(lk_Frame_);
    lk_VLayout_->addWidget(new QLabel("<b>Interpreters</b>", lk_Dialog_));
    
    QHash<QString, QLineEdit*> lk_LanguagePathLineEdits;
    foreach (QString ls_Language, mk_Languages)
    {
        lk_HLayout_ = new QHBoxLayout(NULL);
        QString ls_Label;
        QString ls_Key = configKeyForScriptingLanguage(ls_Language);
        if (ls_Language == "ruby")
            ls_Label = "Ruby";
        else if (ls_Language == "python")
            ls_Label = "Python";
        else if (ls_Language == "php")
            ls_Label = "PHP";
        else if (ls_Language == "perl")
            ls_Label = "Perl";
        else if (ls_Language == "java")
            ls_Label = "Java";
        lk_HLayout_->addWidget(new QLabel(ls_Label + ":", lk_Dialog_));
        QLineEdit* lk_PathLineEdit_ = new QLineEdit(lk_Dialog_);
        lk_PathLineEdit_->setText(getConfiguration(ls_Key).toString());
        lk_PathLineEdit_->home(false);
        lk_HLayout_->addWidget(lk_PathLineEdit_);
        lk_VLayout_->addLayout(lk_HLayout_);
        if (mk_ScriptInterpreterWorking[ls_Language] != true)
        {
            QLabel* lk_PixmapLabel_ = new QLabel(lk_Dialog_);
            lk_PixmapLabel_->setPixmap(QPixmap(":icons/dialog-warning.png").scaledToHeight(16, Qt::SmoothTransformation));
            lk_HLayout_->addWidget(lk_PixmapLabel_);
            lk_VLayout_->addWidget(new QLabel("<b>Note:</b> " + ls_Label + " is not available.", lk_Dialog_));
        }
        lk_LanguagePathLineEdits[ls_Language] = lk_PathLineEdit_;
    }
    
    lk_Frame_ = new QFrame(lk_Dialog_);
    lk_Frame_->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    lk_VLayout_->addWidget(lk_Frame_);
    lk_VLayout_->addWidget(new QLabel("<b>User interface</b>", lk_Dialog_));
    
    lk_HLayout_ = new QHBoxLayout(NULL);
    lk_HLayout_->addWidget(new QLabel("Appearance:", lk_Dialog_));
    QComboBox* lk_AppearanceComboBox_ = new QComboBox(lk_Dialog_);
    lk_AppearanceComboBox_->addItem("normal", QVariant(0));
    lk_AppearanceComboBox_->addItem("small", QVariant(1));
    lk_AppearanceComboBox_->addItem("tiny", QVariant(2));
    lk_AppearanceComboBox_->setCurrentIndex(getConfiguration(CONFIG_APPEARANCE_SIZE).toInt());
    lk_HLayout_->addWidget(lk_AppearanceComboBox_);
    lk_VLayout_->addLayout(lk_HLayout_);
    
    QCheckBox* lk_Animation_ = new QCheckBox("Use animation", lk_Dialog_);
    lk_Animation_->setCheckState(stringToBool(getConfiguration(CONFIG_ANIMATION).toString()) ? Qt::Checked : Qt::Unchecked);
    lk_VLayout_->addWidget(lk_Animation_);
    
    QCheckBox* lk_FollowNewBoxes_ = new QCheckBox("Auto-follow new boxes", lk_Dialog_);
    lk_FollowNewBoxes_->setCheckState(stringToBool(getConfiguration(CONFIG_FOLLOW_NEW_BOXES).toString()) ? Qt::Checked : Qt::Unchecked);
    lk_VLayout_->addWidget(lk_FollowNewBoxes_);
    
    lk_Frame_ = new QFrame(lk_Dialog_);
    lk_Frame_->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    lk_VLayout_->addWidget(lk_Frame_);
    lk_VLayout_->addWidget(new QLabel("<b>Miscellaneous</b>", lk_Dialog_));
    QLabel* lk_DataDirLabel_ = new QLabel("Data directory: <a href='" + QString(FILE_URL_PREFIX) + ms_DataDirectory + "'>" + QDir::toNativeSeparators(ms_DataDirectory) + "</a>", lk_Dialog_);
    lk_DataDirLabel_->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard);
    lk_DataDirLabel_->setOpenExternalLinks(true);
    lk_VLayout_->addWidget(lk_DataDirLabel_);
    
    lk_HLayout_ = new QHBoxLayout(NULL);
    QPushButton* lk_PurgeCacheButton_ = new QPushButton(QIcon(":icons/edit-clear.png"), "&Purge cache");
    lk_HLayout_->addWidget(new QLabel("Remove cached script information and temporary files:", lk_Dialog_));
    lk_HLayout_->addStretch();
    lk_HLayout_->addWidget(lk_PurgeCacheButton_);
    lk_VLayout_->addLayout(lk_HLayout_);
    connect(lk_PurgeCacheButton_, SIGNAL(clicked()), this, SLOT(purgeCacheAndTempFiles()));
    
    lk_VLayout_->addStretch();
    
    lk_Frame_ = new QFrame(lk_Dialog_);
    lk_Frame_->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    lk_MainLayout_->addWidget(lk_Frame_);
    
    lk_HLayout_ = new QHBoxLayout(NULL);
    lk_HLayout_->setContentsMargins(8, 8, 8, 8);
    QPushButton* lk_CancelButton_ = new QPushButton(QIcon(":icons/dialog-cancel.png"), "Cancel", lk_Dialog_);
    connect(lk_CancelButton_, SIGNAL(clicked()), lk_Dialog_, SLOT(reject()));
    lk_HLayout_->addStretch();
    lk_HLayout_->addWidget(lk_CancelButton_);
    QPushButton* lk_OkButton_ = new QPushButton(QIcon(":icons/dialog-ok.png"), "Ok", lk_Dialog_);
    lk_OkButton_->setDefault(true);
    connect(lk_OkButton_, SIGNAL(clicked()), lk_Dialog_, SLOT(accept()));
    lk_HLayout_->addWidget(lk_OkButton_);
    lk_MainLayout_->addLayout(lk_HLayout_);
    lk_Dialog_->resize(600, 400);
    if (lk_Dialog_->exec())
    {
        mk_Configuration[CONFIG_AUTO_CHECK_FOR_UPDATES] = lk_AutoCheckForUpdates_->checkState() == Qt::Checked;
        mk_Configuration[CONFIG_SCRIPTS_URL] = lk_ScriptsUrlLineEdit_->text();
        mk_Configuration[CONFIG_FILETRACKER_URL] = lk_FileTrackerUrlLineEdit_->text();
        mk_Configuration[CONFIG_PATH_TO_RUBY] = lk_LanguagePathLineEdits["ruby"]->text();
        mk_Configuration[CONFIG_PATH_TO_PYTHON] = lk_LanguagePathLineEdits["python"]->text();
        mk_Configuration[CONFIG_PATH_TO_PHP] = lk_LanguagePathLineEdits["php"]->text();
        mk_Configuration[CONFIG_PATH_TO_PERL] = lk_LanguagePathLineEdits["perl"]->text();
        mk_Configuration[CONFIG_PATH_TO_JAVA] = lk_LanguagePathLineEdits["java"]->text();
        mk_Configuration[CONFIG_FOLLOW_NEW_BOXES] = lk_FollowNewBoxes_->checkState() == Qt::Checked;
        mk_Configuration[CONFIG_ANIMATION] = lk_Animation_->checkState() == Qt::Checked;
        mk_Configuration[CONFIG_APPEARANCE_SIZE] = lk_AppearanceComboBox_->currentIndex();
        this->saveConfiguration();
        updateConfigDependentStuff();
        checkRuby();
        emit configurationChanged();
    }
    delete lk_Dialog_;
}


void k_Proteomatic::remoteHubReadyReadSlot()
{
/*  QString ls_Result = QString(mk_pRemoteHubProcess->readAll());
    
    ms_RemoteHubPortion += ls_Result;
    if (ms_RemoteHubPortion.contains(QChar('\n')))
    {
        QStringList lk_Lines = ms_RemoteHubPortion.split(QChar('\n'));
        ms_RemoteHubPortion = lk_Lines.takeLast();
        emit remoteHubLineBatch(lk_Lines);
    }
    
    // don't care about stdout if we already know our rubylicious remote hub
    if (mk_pRemoteHubHttp.data() != NULL)
        return;
        
    ms_RemoteHubStdout += ls_Result;
    QRegExp lk_RegExp("(REMOTE-HUB-PORT-START)(\\d+)(REMOTE-HUB-PORT-END)");
    if (lk_RegExp.indexIn(ms_RemoteHubStdout) > -1) 
    {
        QString ls_Port = lk_RegExp.cap(2);
        bool lb_Ok;
        int li_HubPort = QVariant(ls_Port).toInt(&lb_Ok);
        mk_pRemoteHubHttp = QSharedPointer<QHttp>(new QHttp("127.0.0.1", li_HubPort));
        connect(mk_pRemoteHubHttp.data(), SIGNAL(requestFinished(int, bool)), this, SLOT(remoteHubRequestFinishedSlot(int, bool)));
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
    }*/
}


void k_Proteomatic::scriptMenuScriptClickedInternal()
{
    QAction* lk_Action_ = dynamic_cast<QAction*>(sender());
    if (lk_Action_ != NULL)
        emit scriptMenuScriptClicked(lk_Action_);
}


void k_Proteomatic::addRemoteScriptDialog()
{
    QSharedPointer<QDialog> lk_pDialog(new QDialog(mk_MessageBoxParent_));
    lk_pDialog->setWindowIcon(QIcon(":/icons/proteomatic.png"));
    lk_pDialog->setWindowTitle("Add remote script");
    QBoxLayout* lk_MainLayout_ = new QVBoxLayout(lk_pDialog.data());
    QBoxLayout* lk_Layout_ = new QHBoxLayout();
    QBoxLayout* lk_SubLayout_ = new QVBoxLayout();
    QLabel* lk_Icon_ = new QLabel();
    lk_Icon_->setPixmap(QPixmap(":/icons/applications-internet.png"));
    lk_SubLayout_->addWidget(lk_Icon_);
    lk_SubLayout_->addStretch();
    lk_Layout_->addLayout(lk_SubLayout_);
    lk_SubLayout_ = new QVBoxLayout(lk_pDialog.data());
    lk_SubLayout_->addWidget(new QLabel("Remote script URI:", lk_pDialog.data()));
    QLineEdit* lk_LineEdit_ = new QLineEdit(lk_pDialog.data());
    QCheckBox* lk_Remember_ = new QCheckBox("Remember this remote script", lk_pDialog.data());
    lk_Remember_->setCheckState(Qt::Checked);
    lk_SubLayout_->addWidget(lk_LineEdit_);
    lk_SubLayout_->addWidget(lk_Remember_);
    lk_SubLayout_->addStretch();
    lk_Layout_->addLayout(lk_SubLayout_);
    lk_MainLayout_->addLayout(lk_Layout_);
    lk_Layout_ = new QHBoxLayout(lk_pDialog.data());
    lk_Layout_->addStretch();
    QPushButton* lk_CancelButton_ = new QPushButton(QIcon(":/icons/dialog-cancel.png"), "Cancel", lk_pDialog.data());
    QPushButton* lk_AddButton_ = new QPushButton(QIcon(":/icons/list-add.png"), "Add", lk_pDialog.data());
    lk_AddButton_->setDefault(true);
    lk_AddButton_->setAutoDefault(true);
    lk_Layout_->addWidget(lk_CancelButton_);
    lk_Layout_->addWidget(lk_AddButton_);
    connect(lk_CancelButton_, SIGNAL(clicked()), lk_pDialog.data(), SLOT(reject()));
    connect(lk_AddButton_, SIGNAL(clicked()), lk_pDialog.data(), SLOT(accept()));
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


void k_Proteomatic::remoteHubRequestFinishedSlot(int /*ai_SocketId*/, bool /*ab_Error*/)
{
/*  QString ls_Response = QString(mk_pRemoteHubHttp->readAll());
    
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
                        updateConfigDependentStuff();
                    }
                }
                if (lr_RemoteRequest.mk_AdditionalInfo.contains("feedback"))
                    showMessageBox("Remote script added", "Successfully added " + ls_Caption + ".", ":/icons/applications-internet.png");
            }
        }
    }
    else
        emit remoteHubRequestFinished(ai_SocketId, ab_Error, ls_Response);*/
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


void k_Proteomatic::setConfiguration(QString as_Key, QVariant ak_Value)
{
    mk_Configuration[as_Key] = ak_Value;
}


tk_YamlMap& k_Proteomatic::getConfigurationRoot()
{
    return mk_Configuration;
}


void k_Proteomatic::saveConfiguration()
{
    k_Yaml::emitToFile(mk_Configuration, ms_ConfigurationPath);
}


QString k_Proteomatic::scriptsVersion()
{
    QString ls_Version = findMostRecentManagedScriptPackage().replace("proteomatic-scripts-", "").trimmed();
    return ls_Version;
}


QString k_Proteomatic::completePathForScript(QString as_ScriptFilename)
{
    foreach (QString ls_Path, mk_ScriptInfo.keys())
        if (QFileInfo(ls_Path).fileName() == as_ScriptFilename)
            return ls_Path;
    return QString();
}


QString k_Proteomatic::externalToolsPath() const
{
    return ms_ExternalToolsPath;
}


QMap<QString, QPair<QString, QStringList> > k_Proteomatic::textFileFormats() const
{
    return mk_OwnPlusScriptsTextFileFormats;
}


void k_Proteomatic::highlightScriptsMenu(QStringList ak_InputPaths)
{
    QSet<QString> lk_AllInputSuffixes;
    // always add the 'any' file suffix
    if (!ak_InputPaths.empty())
        lk_AllInputSuffixes << "";
    foreach (QString ls_Path, ak_InputPaths)
    {
        QString ls_Suffix = QFileInfo(ls_Path).completeSuffix().toLower();
        QStringList lk_SuffixList = ls_Suffix.split(".");
        for (int i = 0; i < lk_SuffixList.size(); ++i)
        {
            QString ls_SubSuffix = QStringList(lk_SuffixList.mid(lk_SuffixList.size() - 1 - i, i + 1)).join(".");
            lk_AllInputSuffixes << "." + ls_SubSuffix;
        }
    }
    // go through all script menu items and set icon according to ak_InputPaths,
    // empty ak_InputPaths means 'enable all'
    foreach (QMenu* lk_Menu_, mk_ExtensionsForScriptsMenuSubMenu.keys())
    {
        if (lk_AllInputSuffixes.empty() || (!(lk_AllInputSuffixes & mk_ExtensionsForScriptsMenuSubMenu[lk_Menu_]).empty()))
            lk_Menu_->setIcon(mk_FolderEnabledIcon);
        else
            lk_Menu_->setIcon(mk_FolderDisabledIcon);
    }
    foreach (QAction* lk_Action_, mk_ExtensionsForScriptsMenuAction.keys())
    {
        if (lk_AllInputSuffixes.empty() || (!(lk_AllInputSuffixes & mk_ExtensionsForScriptsMenuAction[lk_Action_]).empty()))
            lk_Action_->setIcon(mk_ScriptEnabledIcon);
        else
            lk_Action_->setIcon(mk_ScriptDisabledIcon);
    }
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
    lk_ErrorLabel_->setOpenExternalLinks(true);
    lk_HLayout_->addWidget(lk_ErrorLabel_);
    lk_VLayout_->addLayout(lk_HLayout_);
    
    QString ls_Platform = 
    #ifdef Q_OS_LINUX
        "Linux"
    #endif
    #ifdef Q_OS_MAC
        "Mac OS X"
    #endif
    #ifdef Q_OS_WIN32
        "Windows"
    #endif
        ;
    
    QLabel* lk_Instructions_ = 
        new QLabel(QString(
    #ifdef Q_OS_LINUX
            "<p><b>Debian/Ubuntu</b></p>\
<p>In order to install Ruby, please open a terminal (Applications &#8594; Accessories<br />&#8594; Terminal) and type the following:<p>\n\
<pre>\n\
$ sudo apt-get install ruby\n\
</pre>\n\
<p><b>Fedora</b></p>\
<p>In order to install Ruby, please open a terminal (Applications &#8594; System Tools<br />&#8594; Terminal) and type the following:<p>\n\
<pre>\n\
$ su\n\
# yum install ruby\n\
</pre>\n"
    #endif
    #ifdef Q_OS_WIN32
            "<p>Please download and run the Ruby 1.9.1 installer: <a href='http://rubyforge.org/frs/download.php/71078/rubyinstaller-1.9.1-p378.exe'>rubyinstaller-1.9.1-p378.exe</a>.</p>\n"
    #endif
    #ifdef Q_OS_MAC
        "<p>If you have installed MacPorts, you can install Ruby by opening a terminal<br />and typing the following:<p>\n\
<pre>\n\
% port install ruby\n\
</pre>"
    #endif
            ) + "<p>After you have installed Ruby, it might be necessary to quit and restart Proteomatic.</p>"
            , &mk_CheckRubyDialog);
            
    lk_Instructions_->setTextInteractionFlags(Qt::TextBrowserInteraction);
    lk_Instructions_->setOpenExternalLinks(true);
    
    k_FoldedHeader* lk_Header_ = new k_FoldedHeader("Detailed instructions for " + ls_Platform, lk_Instructions_, true, &mk_CheckRubyDialog);
    connect(lk_Header_, SIGNAL(showingBuddy()), this, SLOT(checkRubyResize()), Qt::QueuedConnection);
    connect(lk_Header_, SIGNAL(hidingBuddy()), this, SLOT(checkRubyResize()), Qt::QueuedConnection);
    lk_VLayout_->addWidget(lk_Header_);
    lk_VLayout_->addWidget(lk_Instructions_);
    lk_Header_->hideBuddy();
    
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
    
    connect(mk_CheckRubyRetryButton_, SIGNAL(clicked()), &mk_CheckRubyDialog, SLOT(accept()));
    connect(lk_QuitButton_, SIGNAL(clicked()), &mk_CheckRubyDialog, SLOT(reject()));
    connect(mk_CheckRubyLocation_, SIGNAL(textChanged(const QString&)), this, SLOT(checkRubyTextChanged(const QString&)));
    connect(lk_FindRubyButton_, SIGNAL(clicked()), this, SLOT(checkRubySearchDialog()));
    
    mk_CheckRubyDialog.setLayout(lk_VLayout_);
    mk_CheckRubyDialog.resize(300, 1);
    
    // see whether there's a local Ruby installed and prefer that
    // if there is a local Ruby then overwrite the configuration
    
    this->checkScriptingLanguages();
    
    bool lb_DidNotFindRuby = false;
    while (true)
    {
        mk_Configuration[CONFIG_PATH_TO_RUBY] = QVariant(mk_CheckRubyLocation_->text());
        QString ls_Version = syncRuby(QStringList() << "-v");
        QString ls_Error;
        if (!ls_Version.startsWith("ruby"))
        {
            lb_DidNotFindRuby = true;
            ls_Error = "Proteomatic cannot find Ruby, which is required in order to run the scripts.";
        }
        else
        {
            if (lb_DidNotFindRuby)
            {
                // if we're here, we have found a Ruby! Now we save the configuration so that the dialog won't pop up the next time.
                this->saveConfiguration();
                updateConfigDependentStuff();
            }
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

    this->checkScriptingLanguages();
}

void k_Proteomatic::checkRubyTextChanged(const QString& as_Text)
{
    mk_CheckRubyRetryButton_->setEnabled(!as_Text.isEmpty());
}


void k_Proteomatic::checkRubyResize()
{
    mk_CheckRubyDialog.resize(300, 1);
}


void k_Proteomatic::checkRubySearchDialog()
{
    QFileDialog lk_FileDialog(&mk_CheckRubyDialog, "Locate ruby executable", "", "");
    lk_FileDialog.setFileMode(QFileDialog::ExistingFile);
    if (lk_FileDialog.exec())
    {
        mk_CheckRubyLocation_->setText(mk_Configuration[CONFIG_PATH_TO_RUBY].toString());
        mk_CheckRubyRetryButton_->setEnabled(true);
    }
}


void k_Proteomatic::purgeCacheAndTempFiles()
{
    foreach (QFileInfo lk_FileInfo, this->getCacheAndTempFiles())
    {
        QString ls_Path = lk_FileInfo.absoluteFilePath();
        QFile::remove(ls_Path);
    }
    // :ATTENTION: This function will now disable the sender widget, if there is any
    QWidget* lk_Sender_ = dynamic_cast<QWidget*>(sender());
    if (lk_Sender_)
        lk_Sender_->setEnabled(false);
}


QList<QFileInfo> k_Proteomatic::getCacheAndTempFiles()
{
    return QDir(QDir::cleanPath(ms_DataDirectory + "/cache")).entryInfoList(QDir::Files)
        + QDir(QDir::cleanPath(ms_TempPath)).entryInfoList(QDir::Files);
}


bool k_Proteomatic::canPurgeCacheAndTempFiles()
{
    return !this->getCacheAndTempFiles().empty();
}


QString k_Proteomatic::findMostRecentManagedScriptPackage()
{
    
    QStringList lk_Temp = QDir(ms_ManagedScriptsPath).entryList(QDir::NoDotAndDotDot | QDir::AllDirs);
    QStringList lk_AvailableVersions;
    foreach (QString ls_Path, lk_Temp)
        if (ls_Path.contains("proteomatic-scripts"))
            if (!QFileInfo(ls_Path).fileName().contains(".part"))
                lk_AvailableVersions << ls_Path;
    if (lk_AvailableVersions.empty())
        return QString();
    else
    {
        // sort available versions, find most current one
        QMap<qint64, QString> lk_Versions;
        foreach (QString ls_OriginalPath, lk_AvailableVersions)
        {
            qint64 li_Key = 0;
            QString ls_Path = ls_OriginalPath;
            ls_Path = QFileInfo(ls_Path).fileName();
            ls_Path.replace("proteomatic-scripts-", "");
            int li_Place = 3;
            foreach (QString ls_Number, ls_Path.split("."))
            {
                qint64 li_Number = QVariant(ls_Number).toInt();
                li_Key |= li_Number << (li_Place * 16);
                --li_Place;
                if (li_Place < 0)
                    break;
            }
            lk_Versions[li_Key] = ls_OriginalPath;
        }
        // return last entry
        QMap<qint64, QString>::const_iterator lk_Iter = lk_Versions.constEnd();
        --lk_Iter;
        return QFileInfo(lk_Iter.value()).fileName();
    }
}


void k_Proteomatic::updateConfigDependentStuff()
{
    QString ls_FiletrackerUrl = getConfiguration(CONFIG_FILETRACKER_URL).toString();
    if (ls_FiletrackerUrl.isEmpty())
    {
        mk_FileTrackerIconLabel_->setEnabled(false);
        mk_FileTrackerLabel_->setText("No file tracker defined.");
        mk_StartButton_->setText("Start");
        mk_StartButton_->setPopupMode(QToolButton::DelayedPopup);
        mk_StartButton_->setMenu(NULL);
    }
    else
    {
        mk_FileTrackerIconLabel_->setEnabled(true);
        mk_FileTrackerLabel_->setText("File tracker: " + ls_FiletrackerUrl);
        #ifdef Q_OS_MAC
        mk_StartButton_->setText("Start & track");
        #else
        mk_StartButton_->setText("Start && track");
        #endif
        mk_StartButton_->setPopupMode(QToolButton::MenuButtonPopup);
        mk_StartButton_->setMenu(&mk_StartButtonMenu);
    }
    mk_StartButton_->setShortcut(QKeySequence("F5"));
    mk_StartButton_->setToolTip("Start (F5)");
}


QMessageBox::ButtonRole k_Proteomatic::outputFilesAlreadyExistDialog()
{
    QMessageBox lk_MessageBox(mk_MessageBoxParent_);
    lk_MessageBox.setText("<b>Some of the output files already exist.</b><br />Do you want to re-run the entire pipeline and overwrite existing files or do you want to keep existing output files untouched and update missing files?");
    lk_MessageBox.setWindowModality(Qt::ApplicationModal);
    lk_MessageBox.setWindowTitle("Starting pipeline");
    lk_MessageBox.setWindowIcon(QIcon(":icons/proteomatic-pipeline.png"));
    lk_MessageBox.setIconPixmap(QPixmap(":icons/emblem-important.png"));
    QAbstractButton* lk_CancelButton_ = new QPushButton(QIcon(":icons/dialog-cancel.png"), "&Cancel", &lk_MessageBox);
    lk_MessageBox.addButton(lk_CancelButton_, QMessageBox::RejectRole);
    QAbstractButton* lk_OverwriteButton_ = new QPushButton(QIcon(":icons/edit-clear.png"), "&Overwrite existing files", &lk_MessageBox);
    lk_MessageBox.addButton(lk_OverwriteButton_, QMessageBox::DestructiveRole);
    QAbstractButton* lk_UpdateButton_ = new QPushButton(QIcon(":icons/dialog-ok.png"), "&Update", &lk_MessageBox);
    lk_MessageBox.addButton(lk_UpdateButton_, QMessageBox::AcceptRole);
    lk_MessageBox.setEscapeButton(lk_CancelButton_);
    lk_MessageBox.setDefaultButton(dynamic_cast<QPushButton*>(lk_UpdateButton_));
    lk_MessageBox.exec();
    return lk_MessageBox.buttonRole(lk_MessageBox.clickedButton());
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


void k_Proteomatic::openFileLink(QString as_Path)
{
    QDesktopServices::openUrl(QUrl(QString(FILE_URL_PREFIX) + as_Path, QUrl::TolerantMode));
}


QString k_Proteomatic::md5ForFile(QString as_Path, bool ab_ShowProgress)
{   
    QSharedPointer<QProgressDialog> lk_pProgressDialog;
    if (ab_ShowProgress)
    {
        lk_pProgressDialog = QSharedPointer<QProgressDialog>(new QProgressDialog("Determining MD5 hash...", "Cancel", 0, QFileInfo(as_Path).size(), mk_MessageBoxParent_));
        lk_pProgressDialog->setWindowModality(Qt::ApplicationModal);
        lk_pProgressDialog->setWindowFlags(lk_pProgressDialog->windowFlags() | Qt::WindowStaysOnTopHint);
    }

    QFile lk_File(as_Path);
    if (!lk_File.open(QIODevice::ReadOnly))
        return QString();
    
    // calculate MD5 of file content
    md5_state_s lk_Md5State;
    md5_init(&lk_Md5State);
    qint64 li_BytesRead = 0;
    while (!lk_File.atEnd())
    {
        if (ab_ShowProgress)
        {
            if (lk_pProgressDialog->wasCanceled())
                return QString();
        }
        QByteArray lk_Bytes = lk_File.read(8 * 1024 * 1024);
        li_BytesRead += lk_Bytes.size();
        if (ab_ShowProgress)
            lk_pProgressDialog->setValue(li_BytesRead);
        md5_append(&lk_Md5State, (md5_byte_t*)lk_Bytes.constData(), lk_Bytes.size());
    }
    if (ab_ShowProgress)
        lk_pProgressDialog->setValue(QFileInfo(as_Path).size());
    lk_File.close();
    unsigned char lk_Md5[16];
    md5_finish(&lk_Md5State, (md5_byte_t*)(&lk_Md5));
    QString ls_HashString;
    for (int i = 0; i < 16; ++i)
        ls_HashString.append(QString("%1").arg(lk_Md5[i], 2, 16, QChar('0')));
    return ls_HashString;
}


QString k_Proteomatic::md5ForString(QString as_Content)
{
    md5_state_s lk_Md5State;
    md5_init(&lk_Md5State);
    QByteArray lk_ByteArray = as_Content.toAscii();
    md5_append(&lk_Md5State, (md5_byte_t*)lk_ByteArray.constData(), lk_ByteArray.size());
    unsigned char lk_Md5[16];
    md5_finish(&lk_Md5State, (md5_byte_t*)(&lk_Md5));
    QString ls_HashString;
    for (int i = 0; i < 16; ++i)
        ls_HashString.append(QString("%1").arg(lk_Md5[i], 2, 16, QChar('0')));
    return ls_HashString;
}


void k_Proteomatic::reloadScripts()
{
    collectScriptInfo(true);
    createProteomaticScriptsMenu();
}


QToolButton* k_Proteomatic::startButton()
{
    return mk_StartButton_;
}


QAction* k_Proteomatic::startUntrackedAction()
{
    return mk_StartUntrackedAction_;
}


QLabel* k_Proteomatic::fileTrackerIconLabel()
{
    return mk_FileTrackerIconLabel_;
}


QLabel* k_Proteomatic::fileTrackerLabel()
{
    return mk_FileTrackerLabel_;
}


QString k_Proteomatic::scriptInterpreter(const QString& as_Language)
{
    return mk_Configuration[configKeyForScriptingLanguage(as_Language)].toString();
}


QString k_Proteomatic::scriptInterpreterAbsoluteNativePath(const QString& as_Language)
{
    QString ls_Path = this->scriptInterpreter(as_Language);
    // qDebug() << "Original:" << ls_Path;
    QFileInfo lk_FileInfo(ls_Path);
    if (lk_FileInfo.path() != ".")
        ls_Path = QDir::toNativeSeparators(QFileInfo(ls_Path).dir().absolutePath()) + QDir::separator() + QFileInfo(ls_Path).fileName();
    // qDebug() << "Smooth:" << ls_Path;
    return ls_Path;
}


QString k_Proteomatic::configKeyForScriptingLanguage(const QString& as_Language)
{
    if (as_Language == "ruby")
        return CONFIG_PATH_TO_RUBY;
    else if (as_Language == "python")
        return CONFIG_PATH_TO_PYTHON;
    else if (as_Language == "php")
        return CONFIG_PATH_TO_PHP;
    else if (as_Language == "perl")
        return CONFIG_PATH_TO_PERL;
    else if (as_Language == "java")
        return CONFIG_PATH_TO_JAVA;
    return "";
}


bool k_Proteomatic::stringToBool(const QString& as_String)
{
    QString ls_String = as_String.toLower();
    return ls_String == "yes" || ls_String == "true" || ls_String == "1";
}
