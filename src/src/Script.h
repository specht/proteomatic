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
#include "RefPtr.h"
#include "FoldedHeader.h"
#include "Proteomatic.h"
#include "SizeWatchWidget.h"
#include "ProfileManager.h"
#include "Yaml.h"


struct r_ScriptType
{
	enum Enumeration
	{
		Local = 0,
		Remote
	};
};


class k_Script: public QObject
{
	Q_OBJECT

public:
	k_Script(r_ScriptType::Enumeration ae_Type, QString as_ScriptUri, k_Proteomatic& ak_Proteomatic, bool ab_IncludeOutputFiles = true, bool ab_ProfileMode = false);
	virtual ~k_Script();
	bool isGood() const;
	bool hasParameters() const;
	
	r_ScriptType::Enumeration type() const;
	k_SizeWatchWidget* parameterWidget() const;
	QString uri() const;
	virtual QString title() const;
	virtual QString description() const;

	void reset();

	void setPrefix(QString as_Prefix);
	QString prefix() const;
	QList<QString> outFiles() const;
	QHash<QString, QString> outFileDetails(QString as_Key) const;

	QString getParameterValue(QString as_Key) const;
	QString getHumanReadableParameterValue(QString as_Key, QString as_Value) const;
	void setParameterValue(QString as_Key, QString as_Value);
	QStringList getParameterKeys() const;
	QString getHumanReadableParameterKey(QString as_Key) const;
	QString getHumanReadableParameterValue(QString as_Key) const;

	QHash<QString, QString> getConfiguration() const;
	void setConfiguration(QHash<QString, QString> ak_Configuration);
	
	tk_YamlMap getProfile() const;
	void applyProfile(tk_YamlMap ak_Profile);

	QStringList commandLineArguments() const;
	QString profileDescription() const;
	
	virtual void start(QStringList ak_Parameters) = 0;
	virtual void kill() = 0;
	virtual bool running() = 0;
	virtual QString readAll() = 0;
	
signals:
	void profileDescriptionChanged(const QString&);

protected slots:
	void addChoiceItems();
	void removeChoiceItems(QList<QListWidgetItem *> ak_Items);
	void setOutputDirectoryButtonClicked();
	void clearOutputDirectoryButtonClicked();
	void resetDialog();
	void toggleUi();
	void toggleParameter(int ai_State);
	void parameterChanged();
	void parameterChangedWithKey(QString as_Key);

protected:
	void addChoiceItems(QString as_Key, QStringList ak_Choices);
	void createParameterWidget(QStringList ak_Definition);

	r_ScriptType::Enumeration me_Type;
	QString ms_ScriptUri;
	QString ms_Title;
	QString ms_Description;
	QString ms_DefaultOutputDirectory;
	RefPtr<k_SizeWatchWidget> mk_pParameterWidget;
	QHash<QString, QWidget* > mk_ParameterValueWidgets;
	QHash<QString, QWidget* > mk_ParameterDisplayWidgets;
	QHash<QString, QList<QWidget*> > mk_ParameterMultiChoiceWidgets;
	QHash<QString, QDialog* > mk_ParameterMultiChoiceDialogs;
	QHash<QString, k_FoldedHeader*> mk_FoldedHeaders;
	QHash<QString, QHash<QString, QString> > mk_OutFileDetails;
	QHash<QString, QWidget*> mk_WidgetLabelsOrCheckBoxes;
	QHash<QString, QHash<QString, QString> > mk_ParameterDefs;
	QHash<QString, QHash<QString, QString> > mk_ParameterValueLabels;
	QStringList mk_ParametersOrder;
	QHash<QString, bool> mk_ParametersAtDefault;
	QHash<QString, QStringList> mk_GroupParameters;
	QString ms_Prefix;
	QHash<QString, QString> mk_DefaultConfiguration;
	k_Proteomatic& mk_Proteomatic;
	bool mb_IsGood;
	bool mb_HasParameters;
	bool mb_IncludeOutputFiles;
	bool mb_ProfileMode;
	QLineEdit* mk_OutputDirectory_;
	QToolButton* mk_ClearOutputDirectory_;
	QStringList mk_InputFileDescriptionList;
};
