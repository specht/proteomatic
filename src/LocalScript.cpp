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

#include "LocalScript.h"
#include "RubyWindow.h"
#include "Proteomatic.h"

k_LocalScript::k_LocalScript(QString as_ScriptPath, k_Proteomatic& ak_Proteomatic, bool ab_IncludeOutputFiles, bool ab_ProfileMode)
    : k_Script(r_ScriptLocation::Local, as_ScriptPath, ak_Proteomatic, ab_IncludeOutputFiles, ab_ProfileMode)
{
    mk_Process.setProcessChannelMode(QProcess::MergedChannels);
    connect(&mk_Process, SIGNAL(started()), this, SLOT(scriptStartedSlot()));
    connect(&mk_Process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(scriptFinishedSlot(int, QProcess::ExitStatus)));
    connect(&mk_Process, SIGNAL(readyReadStandardError()), this, SIGNAL(readyRead()));
    connect(&mk_Process, SIGNAL(readyReadStandardOutput()), this, SIGNAL(readyRead()));

    QFileInfo lk_FileInfo(as_ScriptPath);
    mk_Process.setWorkingDirectory(lk_FileInfo.path());
    QString ls_CacheFilename = QString("cache/%1.%2.parameters").arg(lk_FileInfo.fileName()).arg(mk_Proteomatic.md5ForString(as_ScriptPath));

    QFile lk_File(as_ScriptPath);
    lk_File.open(QIODevice::ReadOnly);
    QString ls_Marker;
    QByteArray lk_Head = lk_File.read(1024);
    bool lb_SeenProteomatic =  lk_Head.toLower().contains("proteomatic");
    lk_File.close();
    
    if (lb_SeenProteomatic)
    {
        QString ls_Response;
        bool lb_UseCache = mk_Proteomatic.getConfiguration(CONFIG_CACHE_SCRIPT_INFO).toBool() && mk_Proteomatic.fileUpToDate(ls_CacheFilename, QStringList() << as_ScriptPath);
        // disable cache if we're in develop!
        if (mk_Proteomatic.version() == "develop")
            lb_UseCache = false;
            
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
        // check if cached file is newer than script
        if (lb_UseCache)
        {
            if (!(QFileInfo(ls_CacheFilename).lastModified() > QFileInfo(as_ScriptPath).lastModified()))
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
            QString ls_ScriptCachePath = QFileInfo(lk_FileInfo.absolutePath() + "/cache/" + lk_FileInfo.fileName() + ".long.yamlinfo").filePath();
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
                QStringList lk_Arguments;
                lk_Arguments << as_ScriptPath << "---yamlInfo";
                lk_Arguments << "--extToolsPath";
                lk_Arguments << mk_Proteomatic.externalToolsPath();
                // ignore Ruby warnings for ---yamlInfo
                if (mk_Proteomatic.interpreterKeyForScript(as_ScriptPath) == "ruby")
                    lk_Arguments.insert(0, "-W0");
                ls_Response = mk_Proteomatic.syncScriptNoFile(lk_Arguments, mk_Proteomatic.interpreterKeyForScript(as_ScriptPath));
            }
            
            if (!ls_Response.isEmpty())
            {
                if (mk_Proteomatic.getConfiguration(CONFIG_CACHE_SCRIPT_INFO).toBool())
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
            createParameterWidget(ls_Response);
            mk_DefaultConfiguration = configuration();
            mb_IsGood = true;
            return;
        }
        ls_Response.replace("---hasUnresolvedDependencies\r\n", "---hasUnresolvedDependencies\n");
        if (ls_Response.startsWith("---hasUnresolvedDependencies\n"))
        {
            ls_Response = ls_Response.right(ls_Response.length() - QString("---hasUnresolvedDependencies\n").length());
            QStringList lk_Response = ls_Response.split(QChar('\n'));
            QStringList lk_Tools;
            foreach (QString ls_Tool, lk_Response)
                if (ls_Tool != "")
                    lk_Tools << ls_Tool.trimmed();
            QString ls_MissingTools = lk_Tools.join(", ");
                    
            int li_Result = mk_Proteomatic.showMessageBox("Unresolved dependencies", "This script requires the following external tools that are currently not installed:\n\n"
            + ls_MissingTools + "\n\nWould you like to install them now?", ":/icons/package-x-generic.png", QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes, QMessageBox::Cancel,
                QString(), QString(), "Install");
            if (li_Result == QMessageBox::Yes)
            {
                QSharedPointer<k_RubyWindow> lk_pRubyWindow(new k_RubyWindow(mk_Proteomatic, QStringList() << as_ScriptPath << "--resolveDependencies" << "--extToolsPath" << mk_Proteomatic.externalToolsPath(), "Installing external tools", ":/icons/package-x-generic.png"));
                lk_pRubyWindow->exec();
                // retry loading the script
                QStringList lk_Arguments;
                lk_Arguments << as_ScriptPath << "---yamlInfo";
                lk_Arguments << "--extToolsPath";
                lk_Arguments << mk_Proteomatic.externalToolsPath();
                // ignore Ruby warnings for ---yamlInfo
                if (mk_Proteomatic.interpreterKeyForScript(as_ScriptPath) == "ruby")
                    lk_Arguments.insert(0, "-W0");
                ls_Response = mk_Proteomatic.syncScriptNoFile(lk_Arguments, mk_Proteomatic.interpreterKeyForScript(as_ScriptPath));
                if (mk_Proteomatic.getConfiguration(CONFIG_CACHE_SCRIPT_INFO).toBool())
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
                ls_Response.replace("---yamlInfo\r\n", "---yamlInfo\n");
                if (ls_Response.startsWith("---yamlInfo\n"))
                {
                    ls_Response = ls_Response.right(ls_Response.length() - QString("---yamlInfo\n").length());
                    createParameterWidget(ls_Response);
                    mk_DefaultConfiguration = configuration();
                    mb_IsGood = true;
                    return;
                }
            }
        }
        
    }
}


k_LocalScript::~k_LocalScript()
{
#ifdef DEBUG
    printf("k_LocalScript::~k_LocalScript()\n");
#endif
}


QString k_LocalScript::start(const QStringList& ak_Files, tk_StringStringHash ak_AdditionalParameters, bool ab_UseFileTrackerIfAvailable)
{
    QStringList lk_AdditionalParameters;
    foreach (QString ls_Key, ak_AdditionalParameters.keys())
    {
        lk_AdditionalParameters << ls_Key;
        lk_AdditionalParameters << ak_AdditionalParameters[ls_Key];
    }
    if (ab_UseFileTrackerIfAvailable && (!mk_Proteomatic.getConfiguration(CONFIG_FILETRACKER_URL).toString().isEmpty()))
    {
        lk_AdditionalParameters << "--useFileTracker";
        lk_AdditionalParameters << mk_Proteomatic.getConfiguration(CONFIG_FILETRACKER_URL).toString();
    }
    lk_AdditionalParameters << "--extToolsPath";
    lk_AdditionalParameters << mk_Proteomatic.externalToolsPath();
    QStringList lk_Arguments;
    lk_Arguments << this->uri();
    lk_Arguments += commandLineArguments();
    lk_Arguments += lk_AdditionalParameters;
    lk_Arguments += ak_Files;
    QString ls_InterpreterKey = mk_Proteomatic.interpreterKeyForScript(this->uri());
    if (ls_InterpreterKey != "ruby")
    {
        lk_Arguments.insert(1, "--pathToRuby");
        lk_Arguments.insert(2, mk_Proteomatic.scriptInterpreterAbsoluteNativePath("ruby"));
    }
    mk_Process.start(mk_Proteomatic.interpreterForScript(this->uri()), lk_Arguments, QIODevice::ReadOnly | QIODevice::Unbuffered);
    return QString();
}


void k_LocalScript::kill(const QString& /*as_Ticket*/)
{
    mk_Process.kill();
}


void k_LocalScript::scriptStartedSlot()
{
    me_Status = r_ScriptStatus::Running;
    emit scriptStarted();
}


void k_LocalScript::scriptFinishedSlot(int ai_ExitCode, QProcess::ExitStatus ae_Status)
{
    me_Status = r_ScriptStatus::Idle;
    emit scriptFinished(ae_Status == QProcess::CrashExit ? 1 : ai_ExitCode);
}


QString k_LocalScript::readAll()
{
    return QString(mk_Process.readAll());
}
