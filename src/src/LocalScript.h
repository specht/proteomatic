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
