#pragma once

#include <QtCore>
#include "Script.h"


class k_LocalScript: public k_Script
{
	Q_OBJECT
	
public:
	k_LocalScript(QString as_ScriptPath, k_Proteomatic& ak_Proteomatic, bool ab_IncludeOutputFiles = true, bool ab_ProfileMode = false);
	virtual ~k_LocalScript();
	
	virtual void start(QStringList ak_Parameters = QStringList());
	virtual void kill();
	virtual bool running();
	virtual QString readAll();

signals:
	virtual void started();
	virtual void finished(int, QProcess::ExitStatus);
	virtual void readyRead();

protected:
	QProcess mk_Process;
};
