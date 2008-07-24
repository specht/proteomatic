#pragma once

#include <QtCore>
#include "Proteomatic.h"
#include "RefPtr.h"
#include "Script.h"


class k_RemoteScript: public k_Script
{
	Q_OBJECT
	
public:
	k_RemoteScript(QString as_ScriptUri, k_Proteomatic& ak_Proteomatic, bool ab_IncludeOutputFiles = true, bool ab_ProfileMode = false);
	virtual ~k_RemoteScript();
	
	virtual void start(QStringList ak_Parameters);
	virtual void kill();
	virtual bool running();
	virtual QString readAll();
	
protected:
	QString ms_Host;
};
