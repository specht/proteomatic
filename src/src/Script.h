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
#include "IScript.h"
#include "FoldedHeader.h"
#include "Proteomatic.h"
#include "ProfileManager.h"
#include "RefPtr.h"
#include "Yaml.h"


class k_Script: public QObject, public IScript
{
	Q_OBJECT

public:
	k_Script(r_ScriptLocation::Enumeration ae_Location, QString as_ScriptUri, k_Proteomatic& ak_Proteomatic, bool ab_IncludeOutputFiles = true, bool ab_ProfileMode = false);
	virtual ~k_Script();

	bool isGood() const;
	
	// ---------------------------------------------------------
	// IScript
	// ---------------------------------------------------------
	
	// general script properties
	virtual QString uri() const;
	virtual r_ScriptLocation::Enumeration location() const;
	virtual r_ScriptType::Enumeration type() const;
	virtual r_ScriptStatus::Enumeration status() const;

	// script information
	virtual QString title() const;
	virtual QString description() const;
	virtual bool hasParameters() const;
	virtual QHash<QString, QString> info() const;
	
	// parameter widget
	virtual QWidget* parameterWidget() const;
	
	// script parameters
	virtual QStringList parameterKeys() const;
	virtual QString parameterLabel(const QString& as_Key) const;
	virtual QString parameterValue(const QString& as_Key) const;
	virtual QString parameterDefault(const QString& as_Key) const;
	virtual QString humanReadableParameterValue(const QString& as_Key, const QString& as_Value) const;
	virtual QString humanReadableParameterValue(const QString& as_Key) const;
	virtual void setParameter(const QString& as_Key, const QString& as_Value);
	
	// script configuration
	
	// get all parameter key/value pairs
	virtual QHash<QString, QString> configuration() const;
	// get all parameter key/default value pairs
	virtual QHash<QString, QString> defaultConfiguration() const;
	// get all parameter key/value pairs that have been 'set',
	// that is, whose values are non-default
	virtual QHash<QString, QString> nonDefaultConfiguration() const;
	virtual void setConfiguration(const QHash<QString, QString>& ak_Configuration);
	
	// profiles
	virtual tk_YamlMap profile() const;
	virtual QString profileDescription() const;
	virtual void applyProfile(const tk_YamlMap& ak_Profile);
	
	// input files
	virtual QStringList inputGroupKeys() const;
	virtual QString inputGroupLabel(const QString& as_Key) const;
	virtual QStringList inputGroupExtensions(const QString& as_Key) const;
	virtual QString inputGroupForFilename(const QString& as_Path) const;
	// checkInputFiles doesn't care whether files are actually there,
	// it just checks whether all min/max requirements are fulfilled.
	// ak_Files is a hash {group => set of filenames}
	virtual bool checkInputFiles(const QHash<QString, QSet<QString> >& ak_Files, QString& as_InputFilesErrorMessage) const;

	// output files
	virtual QString outputDirectory() const;
	virtual void setOutputDirectory(const QString& as_Path);
	virtual QString outputFilePrefix() const;
	virtual void setOutputFilePrefix(const QString& as_Prefix);
	virtual QStringList outputFileKeys() const;
	virtual QHash<QString, QString> outputFileDetails(const QString& as_Key) const;
	
	virtual QString readAll() = 0;
	
signals:
	virtual void scriptStarted();
	virtual void scriptFinished(int ai_ExitCode);
	virtual void parameterChanged(const QString& as_Key);
	void profileDescriptionChanged(const QString&);
	void proposePrefixButtonClicked();
	
public slots:
	virtual void reset();
	virtual void resetAndUncheck();
	virtual QString start(const QStringList& ak_InputFiles, tk_StringStringHash ak_Parameters = tk_StringStringHash()) = 0;
	virtual void kill(const QString& as_Ticket = QString()) = 0;

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
	void adjustDependentParameters();
	QStringList commandLineArguments() const;

	k_Proteomatic& mk_Proteomatic;

	QString ms_Uri;
	r_ScriptLocation::Enumeration me_Location;
	r_ScriptType::Enumeration me_Type;
	r_ScriptStatus::Enumeration me_Status;
	bool mb_IncludeOutputFiles;
	bool mb_ProfileMode;
	bool mb_IsGood;
	
	QString ms_Title;
	QString ms_Description;
	bool mb_HasParameters;
	QString ms_DefaultOutputDirectory;

	RefPtr<QWidget> mk_pParameterWidget;
	
	QStringList mk_ParameterKeys;
	QHash<QString, QStringList> mk_GroupParameters;
	
	QHash<QString, QString> mk_Configuration;
	QHash<QString, QString> mk_DefaultConfiguration;
	
	QHash<QString, QWidget*> mk_ParameterValueWidgets;
	QHash<QString, QWidget*> mk_ParameterDisplayWidgets;
	QHash<QString, QList<QWidget*> > mk_ParameterMultiChoiceWidgets;
	QHash<QString, QDialog*> mk_ParameterMultiChoiceDialogs;
	QHash<QString, k_FoldedHeader*> mk_FoldedHeaders;
	
	QHash<QString, QHash<QString, QString> > mk_OutFileDetails;
	QHash<QString, QWidget*> mk_WidgetLabelsOrCheckBoxes;
	QHash<QString, QHash<QString, QString> > mk_ParameterDefs;
	QHash<QString, QHash<QString, QString> > mk_ParameterValueLabels;
	
	QHash<QString, QString> mk_Info;
	
	QStringList mk_InputGroupKeys;
	QHash<QString, QString> mk_InputGroupLabels;
	QHash<QString, QStringList> mk_InputGroupExtensions;
	QHash<QString, QString> mk_InputGroupDescriptions;
	// these two hashes contain the optional input file min/max counts
	// if a min/max value has not been defined, there will be no entry
	// in the hashes. simple as that.
	QHash<QString, int> mk_InputGroupMinimum;
	QHash<QString, int> mk_InputGroupMaximum;
	
	QStringList mk_DependentParameters;
	
	RefPtr<QLineEdit> mk_pOutputDirectory;
	RefPtr<QLineEdit> mk_pOutputPrefix;
	RefPtr<QToolButton> mk_pClearOutputDirectoryButton;
};
