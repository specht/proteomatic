#include "RemoteScript.h"


k_RemoteScript::k_RemoteScript(QString as_ScriptUri, k_Proteomatic& ak_Proteomatic, bool ab_IncludeOutputFiles, bool ab_ProfileMode)
	: k_Script(r_ScriptType::Remote, as_ScriptUri, ak_Proteomatic, ab_IncludeOutputFiles, ab_ProfileMode)
	, ms_Host("")
{
	QStringList lk_Response = ak_Proteomatic.scriptInfo(as_ScriptUri, "parameters").split(QChar('\n'));
	if (!lk_Response.empty())
	{
		QString ls_FirstLine = lk_Response.takeFirst().trimmed();
		if (ls_FirstLine == "---getParameters")
		{
			createParameterWidget(lk_Response, ab_IncludeOutputFiles);
			mk_DefaultConfiguration = getConfiguration();
			mb_IsGood = true;
		}
	}
	
	ms_Host = as_ScriptUri;
	ms_Host = ms_Host.replace("druby://", "").split(":").first();
}


k_RemoteScript::~k_RemoteScript()
{
}


void k_RemoteScript::start(QStringList ak_Parameters)
{
}


void k_RemoteScript::kill()
{
}


bool k_RemoteScript::running()
{
	return false;
}


QString k_RemoteScript::readAll()
{
	return "";
}

