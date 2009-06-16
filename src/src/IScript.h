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

#include "Yaml.h"
#include "RefPtr.h"

class QWidget;

typedef QHash<QString, QString> tk_StringStringHash;

struct r_ScriptLocation
{
	enum Enumeration
	{
		Local,
		Remote
	};
};


struct r_ScriptType
{
	enum Enumeration
	{
		Processor,
		Converter
	};
};


struct r_ScriptStatus
{
	enum Enumeration
	{
		Idle,
		Waiting,
		Running
	};
};


struct IScript
{
	virtual ~IScript() {}
	
	// general script properties
	virtual QString uri() const = 0;
	virtual r_ScriptLocation::Enumeration location() const = 0;
	virtual r_ScriptType::Enumeration type() const = 0;
	virtual r_ScriptStatus::Enumeration status() const = 0;

	// script information
	virtual QString title() const = 0;
	virtual QString description() const = 0;
	virtual bool hasParameters() const = 0;
	virtual QHash<QString, QString> info() const = 0;
	
	// parameter widget
	virtual QWidget* parameterWidget() const = 0;
	
	// script parameters
	virtual QStringList parameterKeys() const = 0;
	virtual QString parameterLabel(const QString& as_Key) const = 0;
	virtual QString parameterValue(const QString& as_Key) const = 0;
	virtual QString parameterDefault(const QString& as_Key) const = 0;
	virtual QString humanReadableParameterValue(const QString& as_Key, const QString& as_Value) const = 0;
	virtual QString humanReadableParameterValue(const QString& as_Key) const = 0;
	virtual void setParameter(const QString& as_Key, const QString& as_Value) = 0;
	
	// script configuration
	
	// get all parameter key/value pairs
	virtual QHash<QString, QString> configuration() const = 0;
	// get all parameter key/default value pairs
	virtual QHash<QString, QString> defaultConfiguration() const = 0;
	// get all parameter key/value pairs that have been 'set',
	// that is, whose values are non-default
	virtual QHash<QString, QString> nonDefaultConfiguration() const = 0;
	virtual void setConfiguration(const QHash<QString, QString>& ak_Configuration) = 0;
	
	// profiles
	virtual tk_YamlMap profile() const = 0;
	virtual QString profileDescription() const = 0;
	virtual void applyProfile(const tk_YamlMap& ak_Profile) = 0;
	
	// input files
	virtual QStringList inputGroupKeys() const = 0;
	virtual QString inputGroupLabel(const QString& as_Key) const = 0;
	virtual QStringList inputGroupExtensions(const QString& as_Key) const = 0;
	virtual QString inputGroupForFilename(const QString& as_Path) const = 0;
	virtual QString defaultOutputDirectoryInputGroup() const = 0;
	// checkInputFiles doesn't care whether files are actually there,
	// it just checks whether all min/max requirements are fulfilled.
	// ak_Files is a hash {group => set of filenames}
	virtual bool checkInputFiles(const QHash<QString, QSet<QString> >& ak_Files, QString& as_InputFilesErrorMessage) const = 0;
	virtual QString proposePrefix(QStringList ak_Files) = 0;

	// output files
	virtual QString outputDirectory() const = 0;
	virtual void setOutputDirectory(const QString& as_Path) = 0;
	virtual QString outputFilePrefix() const = 0;
	virtual void setOutputFilePrefix(const QString& as_Prefix) = 0;
	virtual QStringList outputFileKeys() const = 0;
	virtual QHash<QString, QString> outputFileDetails(const QString& as_Key) const = 0;
	
	virtual QString readAll() = 0;
	
	// slots
	// reset all parameters to their default values
	virtual void reset() = 0;
	// reset all parameters to their default values and uncheck check boxes (profile mode)
	virtual void resetAndUncheck() = 0;
	virtual QString start(const QStringList& ak_InputFiles, tk_StringStringHash ak_AdditionalParameters = tk_StringStringHash()) = 0;
	virtual void kill(const QString& as_Ticket = QString()) = 0;
	
	// signals
	virtual void scriptStarted() = 0;
	virtual void scriptFinished(int ai_ExitCode) = 0;
	virtual void readyRead() = 0;
	virtual void parameterChanged(const QString& as_Key) = 0;
};
