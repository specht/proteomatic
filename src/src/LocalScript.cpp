#include "LocalScript.h"
#include "RubyWindow.h"


k_LocalScript::k_LocalScript(QString as_ScriptPath, k_Proteomatic& ak_Proteomatic, bool ab_IncludeOutputFiles, bool ab_ProfileMode)
	: k_Script(r_ScriptType::Local, as_ScriptPath, ak_Proteomatic, ab_IncludeOutputFiles, ab_ProfileMode)
{
	mk_Process.setProcessChannelMode(QProcess::MergedChannels);
	connect(&mk_Process, SIGNAL(started()), this, SIGNAL(started()));
	connect(&mk_Process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SIGNAL(finished(int, QProcess::ExitStatus)));
	connect(&mk_Process, SIGNAL(readyReadStandardError()), this, SIGNAL(readyRead()));
	connect(&mk_Process, SIGNAL(readyReadStandardOutput()), this, SIGNAL(readyRead()));

	QFileInfo lk_FileInfo(as_ScriptPath);
	mk_Process.setWorkingDirectory(lk_FileInfo.path());

	QFile lk_File(as_ScriptPath);
	lk_File.open(QIODevice::ReadOnly | QIODevice::Unbuffered);
	
	QString ls_Marker;
	if (lk_File.size() >= 29)
		ls_Marker = QString(lk_File.read(29));
	
	lk_File.close();
		
	if (ls_Marker == "require 'include/proteomatic'")
	{
		QString ls_Response = mk_Proteomatic.syncRuby(QStringList() << as_ScriptPath << "---getParameters");
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
					QString ls_Response = mk_Proteomatic.syncRuby(QStringList() << as_ScriptPath << "---getParameters");
					lk_Response = ls_Response.split(QChar('\n'));
					ls_FirstLine = lk_Response.takeFirst().trimmed();
				}
			}
			
			if (ls_FirstLine == "---getParameters")
			{
				createParameterWidget(lk_Response, ab_IncludeOutputFiles, ab_ProfileMode);
				mk_DefaultConfiguration = getConfiguration();
				mb_IsGood = true;
			}
		}
	}
}


k_LocalScript::~k_LocalScript()
{
}


void k_LocalScript::start(QStringList ak_Parameters)
{
	mk_Process.start(mk_Proteomatic.rubyPath(), (QStringList() << ms_ScriptUri) + commandLineArguments() + ak_Parameters, QIODevice::ReadOnly | QIODevice::Unbuffered);
}


void k_LocalScript::kill()
{
	mk_Process.kill();
}


bool k_LocalScript::running()
{
	return mk_Process.state() != QProcess::NotRunning;
}


QString k_LocalScript::readAll()
{
	return QString(mk_Process.readAll());
}
