#include "ScriptFactory.h"
#include "LocalScript.h"
#include "RemoteScript.h"


k_Script* k_ScriptFactory::makeScript(QString as_ScriptUri, k_Proteomatic& ak_Proteomatic, bool ab_IncludeOutputFiles, bool ab_ProfileMode)
{
	k_Script* lk_Script_ = NULL;
	if (as_ScriptUri.startsWith("druby://"))
		lk_Script_ = new k_RemoteScript(as_ScriptUri, ak_Proteomatic, ab_IncludeOutputFiles, ab_ProfileMode);
	else
		lk_Script_ = new k_LocalScript(as_ScriptUri, ak_Proteomatic, ab_IncludeOutputFiles, ab_ProfileMode);
		
	return lk_Script_;
}
