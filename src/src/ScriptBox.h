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

#include <QtGui>
#include "ConsoleString.h"
#include "IDesktopBox.h"
#include "IScriptBox.h"
#include "DesktopBox.h"
#include "HintLineEdit.h"
#include "RefPtr.h"


class k_Proteomatic;

class k_ScriptBox: public k_DesktopBox, public IScriptBox
{
	Q_OBJECT
public:
	k_ScriptBox(RefPtr<IScript> ak_pScript, k_Desktop* ak_Parent_, k_Proteomatic& ak_Proteomatic);
	virtual ~k_ScriptBox();
	
	virtual IScript* script();
	virtual bool checkReady(QString& as_Error);
	virtual bool checkReadyToGo();
	virtual QStringList iterationKeys();
	virtual QString outputDirectory() const;
	virtual QWidget* paneWidget();
	virtual bool hasExistingOutputFiles();
	
public slots:
	virtual void start(const QString& as_IterationKey);
	virtual void abort();
	
protected slots:
	virtual void outputFileActionToggled();
	virtual void handleBoxConnected(IDesktopBox* ak_Other_, bool ab_Incoming);
	virtual void handleBoxDisconnected(IDesktopBox* ak_Other_, bool ab_Incoming);
	virtual void updateBatchMode();
	virtual void updateOutputFilenames();
	virtual void proposePrefixButtonClicked();
	virtual void readyReadSlot();
	virtual void addOutput(QString as_String);
	virtual void showOutputBox(bool ab_Flag = true);
	virtual void scriptParameterChanged(const QString& as_Key);
	virtual void chooseOutputDirectory();
	
signals:
	virtual void scriptStarted();
	virtual void scriptFinished(int ai_ExitCode);
	virtual void readyRead();
	
protected:
	virtual void setupLayout();
	
	RefPtr<IScript> mk_pScript;
	RefPtr<QWidget> mk_pParameterProxyWidget;
	QHash<QString, IDesktopBox*> mk_OutputFileBoxes;
	QHash<QString, QCheckBox*> mk_Checkboxes;
	
	k_HintLineEdit mk_Prefix;
	k_HintLineEdit mk_OutputDirectory;

	k_ConsoleString ms_Output;
	QTextEdit mk_OutputBox;
	QTabWidget* mk_TabWidget_;
	QString ms_ConverterFilenamePattern;
	QSet<QString> mk_ConverterFilenameAffectingParameters;
};
