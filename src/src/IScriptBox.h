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

#include "IScript.h"


struct IScriptBox
{
	virtual ~IScriptBox() {};
	
	virtual IScript* script() = 0;
	virtual bool checkReady(QString& as_Error) = 0;
	virtual bool checkReadyToGo() = 0;
	virtual QStringList iterationKeys() = 0;
	virtual void start(const QString& as_IterationKey) = 0;
	virtual void showOutputBox(bool ab_Flag = true) = 0;
	
	// signals
	virtual void scriptStarted() = 0;
	virtual void scriptFinished(int ai_ExitCode) = 0;
	virtual void readyRead() = 0;
};
