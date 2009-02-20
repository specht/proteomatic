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

#include "RemoteScript.h"


k_RemoteScript::k_RemoteScript(QString as_ScriptUri, k_Proteomatic& ak_Proteomatic, bool ab_IncludeOutputFiles, bool ab_ProfileMode)
	: k_Script(r_ScriptLocation::Remote, as_ScriptUri, ak_Proteomatic, ab_IncludeOutputFiles, ab_ProfileMode)
	, ms_Host("")
{
	QStringList lk_Response = ak_Proteomatic.scriptInfo(as_ScriptUri, "parameters").split(QChar('\n'));
	if (!lk_Response.empty())
	{
		QString ls_FirstLine = lk_Response.takeFirst().trimmed();
		if (ls_FirstLine == "---getParameters")
		{
			createParameterWidget(lk_Response);
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

