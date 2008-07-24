#pragma once

#include <QtCore>
#include "Proteomatic.h"
#include "Script.h"


class k_ScriptFactory
{
public:
	static k_Script* makeScript(QString as_ScriptUri, k_Proteomatic& ak_Proteomatic, bool ab_IncludeOutputFiles = true, bool ab_ProfileMode = false);
};
