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
	QString ls_CacheFilename = QString("cache/%1.parameters").arg(lk_FileInfo.baseName());

	QFile lk_File(as_ScriptPath);
	lk_File.open(QIODevice::ReadOnly);
	QString ls_Marker;
	QTextStream lk_Stream(&lk_File);
	while (!lk_Stream.atEnd() && (ls_Marker.isEmpty() || ls_Marker.startsWith("#")))
		ls_Marker = lk_Stream.readLine().trimmed();
	lk_File.close();
		
	if (ls_Marker == "require 'include/proteomatic'" || ls_Marker == "require \"include/proteomatic\"")
	{
		QString ls_Response;
		bool lb_UseCache = mk_Proteomatic.getConfiguration(CONFIG_CACHE_SCRIPT_INFO).toBool() && mk_Proteomatic.fileUpToDate(ls_CacheFilename, QStringList() << as_ScriptPath);
		if (lb_UseCache)
		{
			// see if cached info showed no errors or unresolved dependencies
			QFile lk_File(ls_CacheFilename);
			lk_File.open(QIODevice::ReadOnly);
			QTextStream lk_Stream(&lk_File);
			QString ls_Line = lk_Stream.readLine().trimmed();
			if (ls_Line != "---getParameters")
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
			ls_Response = mk_Proteomatic.syncRuby(QStringList() << as_ScriptPath << "---getParameters");
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
		QStringList lk_Response = ls_Response.split(QChar('\n'));
		if (!lk_Response.empty())
		{
			QString ls_FirstLine = lk_Response.takeFirst().trimmed();
			if (ls_FirstLine == "---getParametersUnresolvedDependencies")
			{
				QStringList lk_Tools;
				foreach (QString ls_Tool, lk_Response)
					if (ls_Tool != "")
						lk_Tools << ls_Tool.trimmed();
				QString ls_MissingTools = lk_Tools.join(", ");
						
				int li_Result = mk_Proteomatic.showMessageBox("Unresolved dependencies", "This script requires the following external tools that are currently not installed:\n\n"
				+ ls_MissingTools + "\n\nWould you like to install them now?", ":/icons/package-x-generic.png", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes, QMessageBox::No);
				if (li_Result == QMessageBox::Yes)
				{
					RefPtr<k_RubyWindow> lk_pRubyWindow(new k_RubyWindow(mk_Proteomatic, QStringList() << as_ScriptPath << "--resolveDependencies", "Installing external tools", ":/icons/package-x-generic.png"));
					lk_pRubyWindow->exec();
					// retry loading the script
					ls_Response = mk_Proteomatic.syncRuby(QStringList() << as_ScriptPath << "---getParameters");
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
					lk_Response = ls_Response.split(QChar('\n'));
					ls_FirstLine = lk_Response.takeFirst().trimmed();
				}
			}
			
			if (ls_FirstLine == "---getParameters")
			{
				createParameterWidget(lk_Response);
				mk_DefaultConfiguration = configuration();
				mb_IsGood = true;
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


QString k_LocalScript::start(const QStringList& ak_Files, tk_StringStringHash ak_AdditionalParameters)
{
	QStringList lk_AdditionalParameters;
	foreach (QString ls_Key, ak_AdditionalParameters.keys())
	{
		lk_AdditionalParameters << ls_Key;
		lk_AdditionalParameters << ak_AdditionalParameters[ls_Key];
	}
	mk_Process.start(mk_Proteomatic.getConfiguration(CONFIG_PATH_TO_RUBY).toString(), (QStringList() << this->uri()) + commandLineArguments() + lk_AdditionalParameters + ak_Files, QIODevice::ReadOnly | QIODevice::Unbuffered);
	return QString();
}


QString k_LocalScript::proposePrefix(QStringList ak_Parameters)
{
	return mk_Proteomatic.syncRuby((QStringList() << this->uri()) << this->commandLineArguments() << "--proposePrefix" << ak_Parameters);
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
