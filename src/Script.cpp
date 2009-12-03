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

#include "Script.h"
#include "ClickableLabel.h"
#include "FoldedHeader.h"
#include "FileList.h"
#include "CiListWidgetItem.h"
#include "Proteomatic.h"
#include "RefPtr.h"
#include "RubyWindow.h"
#include <limits.h>
#include <float.h>
#include <sstream>
#include <yaml-cpp/yaml.h>


k_Script::k_Script(r_ScriptLocation::Enumeration ae_Location, QString as_ScriptUri, k_Proteomatic& ak_Proteomatic, bool ab_IncludeOutputFiles, bool ab_ProfileMode)
	: mk_Proteomatic(ak_Proteomatic)
	, ms_Uri(as_ScriptUri)
	, me_Location(ae_Location)
	, me_Type(r_ScriptType::Processor)
	, me_Status(r_ScriptStatus::Idle)
	, mb_IncludeOutputFiles(ab_IncludeOutputFiles)
	, mb_ProfileMode(ab_ProfileMode)
	, mb_IsGood(false)
	, ms_Title(QString())
	, ms_Description(QString())
	, mb_HasParameters(false)
{
	if (mk_Proteomatic.hasScriptInfo(ms_Uri))
	{
		// if this script is from the scripts menu, title and description are
		// already known
		ms_Title = mk_Proteomatic.scriptInfo(ms_Uri, "title");
		ms_Description = mk_Proteomatic.scriptInfo(ms_Uri, "description");
	}
	else
	{
		// retrieve script info (title, description)
		QProcess lk_QueryProcess;
		QFileInfo lk_FileInfo(ms_Uri);
		lk_QueryProcess.setWorkingDirectory(lk_FileInfo.absolutePath());
		QStringList lk_Arguments;
		lk_Arguments << ms_Uri << "---yamlInfo" << "--short";
        // ignore Ruby warnings for ---yamlInfo
        if (mk_Proteomatic.interpreterKeyForScript(ms_Uri) == "ruby")
            lk_Arguments.insert(0, "-W0");
        
		lk_QueryProcess.setProcessChannelMode(QProcess::MergedChannels);
		
		lk_QueryProcess.start(mk_Proteomatic.interpreterForScript(ms_Uri), lk_Arguments, QIODevice::ReadOnly | QIODevice::Unbuffered);
		if (lk_QueryProcess.waitForFinished())
		{
			QString ls_Response = lk_QueryProcess.readAll();
            ls_Response.replace("---yamlInfo\r\n", "---yamlInfo\n");
            if (ls_Response.startsWith("---yamlInfo\n"))
            {
                ls_Response = ls_Response.right(ls_Response.length() - QString("---yamlInfo\n").length());
                QVariant lk_Response = k_Yaml::parseFromString(ls_Response);
                if (lk_Response.canConvert<tk_YamlMap>())
                {
                    ms_Title = lk_Response.toMap()["title"].toString();
                    if (lk_Response.toMap().contains("description"))
                        ms_Description = lk_Response.toMap()["description"].toString();
                }
            }
		}
	}
}


k_Script::~k_Script()
{
#ifdef DEBUG
	printf("k_Script::~k_Script()\n");
#endif
}


bool k_Script::isGood() const
{
	return mb_IsGood;
}


bool k_Script::hasParameters() const
{
	return mb_HasParameters;
}


r_ScriptLocation::Enumeration k_Script::location() const
{
	return me_Location;
}


r_ScriptType::Enumeration k_Script::type() const
{
	return me_Type;
}


r_ScriptStatus::Enumeration k_Script::status() const
{
	return me_Status;
}


QHash<QString, QString> k_Script::info() const
{
	return mk_Info;
}


QWidget* k_Script::parameterWidget() const
{
	return mk_pParameterWidget.get_Pointer();
}


QString k_Script::uri() const
{
	return ms_Uri;
}


QString k_Script::title() const
{
	return ms_Title;
}


QString k_Script::description() const
{
	return ms_Description;
}


void k_Script::reset()
{
	setConfiguration(mk_DefaultConfiguration);
}


void k_Script::resetAndUncheck()
{
	this->reset();
	// uncheck all checkboxes, if any
	if (mb_ProfileMode)
	{
		foreach (QString ls_Key, mk_WidgetLabelsOrCheckBoxes.keys())
		{
			QCheckBox* lk_CheckBox_ = dynamic_cast<QCheckBox*>(mk_WidgetLabelsOrCheckBoxes[ls_Key]);
			if (lk_CheckBox_)
				lk_CheckBox_->setCheckState(Qt::Unchecked);
		}
	}
}


void k_Script::setOutputFilePrefix(const QString& as_Prefix)
{
	if (mk_pOutputPrefix)
		mk_pOutputPrefix->setText(as_Prefix);
}


QString k_Script::outputDirectory() const
{
	if (mk_pOutputDirectory)
		return mk_pOutputDirectory->text();
	else
		return QString();
}


QString k_Script::outputFilePrefix() const
{
	if (mk_pOutputPrefix)
		return mk_pOutputPrefix->text();
	else
		return QString();
}


QStringList k_Script::outputFileKeys() const
{
	return mk_OutFileDetails.keys();
}


QHash<QString, QString> k_Script::outputFileDetails(const QString& as_Key) const
{
	return mk_OutFileDetails[as_Key];
}


QStringList k_Script::commandLineArguments() const
{
	QStringList lk_Result;
	foreach (QString ls_Key, mk_ParameterValueWidgets.keys())
	{
		lk_Result.push_back("-" + ls_Key);
		lk_Result.push_back(this->parameterValue(ls_Key));
	}
	return lk_Result;
}


QString k_Script::profileDescription() const
{
	QStringList lk_ProfileDescription;
	foreach (QString ls_Key, mk_ParameterKeys)
	{
		QWidget* lk_Widget_ = mk_WidgetLabelsOrCheckBoxes[ls_Key];
		QCheckBox* lk_CheckBox_ = dynamic_cast<QCheckBox*>(lk_Widget_);
		if (lk_CheckBox_ && (lk_CheckBox_->checkState() == Qt::Checked))
			lk_ProfileDescription.push_back(QString("%1: %2").arg(parameterLabel(ls_Key)).arg(humanReadableParameterValue(ls_Key, parameterValue(ls_Key))));
	}
	
	QString ls_Description;
	if (lk_ProfileDescription.isEmpty())
		lk_ProfileDescription << "<i>No parameters have been specified.</i>";

	ls_Description = lk_ProfileDescription.join("<br />");
		
	return ls_Description;
}


QStringList k_Script::inputGroupKeys() const
{
	return mk_InputGroupKeys;
}


QString k_Script::inputGroupLabel(const QString& as_Key) const
{
	return mk_InputGroupLabels[as_Key];
}


QStringList k_Script::inputGroupExtensions(const QString& as_Key) const
{
	return mk_InputGroupExtensions[as_Key];
}


void k_Script::setOutputDirectory(const QString& as_Path)
{
	mk_pOutputDirectory->setText(as_Path);
}


bool k_Script::checkInputFiles(const QHash<QString, QSet<QString> >& ak_Files, QString& as_ErrorMessage) const
{
	as_ErrorMessage = "";
	bool lb_Result = true;
	// check minimum counts
	foreach (QString ls_Key, mk_InputGroupMinimum.keys())
	{
		int li_Minimum = mk_InputGroupMinimum[ls_Key];
		if (!ak_Files.contains(ls_Key) || ak_Files[ls_Key].size() < li_Minimum)
		{
			lb_Result = false;
			as_ErrorMessage += QString("At least %1 %2 %3 required.<br />").arg(li_Minimum).arg(mk_InputGroupLabels[ls_Key]).arg(li_Minimum == 1 ? "file is" : "files are");
		}
	}
	// check maximum counts
	foreach (QString ls_Key, mk_InputGroupMaximum.keys())
	{
		int li_Maximum = mk_InputGroupMaximum[ls_Key];
		if (ak_Files.contains(ls_Key) && ak_Files[ls_Key].size() > li_Maximum)
		{
			lb_Result = false;
			as_ErrorMessage += QString("At most %1 %2 %3 allowed.<br />").arg(li_Maximum).arg(mk_InputGroupLabels[ls_Key]).arg(li_Maximum == 1 ? "file is" : "files are");
		}
	}
	return lb_Result;
}


bool intStringLessThan(const QString& as_First, const QString& as_Second)
{
	return QVariant(as_First).toInt() < QVariant(as_Second).toInt();
}


QString k_Script::mergeFilenames(QStringList ak_Files)
{
	if (ak_Files.empty())
		return QString();
	
	QStringList lk_Files;
	foreach (QString ls_Path, ak_Files)
		lk_Files.push_back(QFileInfo(ls_Path).completeBaseName().split(".").first());
	
	if (lk_Files.size() == 1)
		return lk_Files.first();
	
	QString ls_AllPattern;
	QList<QSet<QString> > lk_AllParts;
	foreach (QString ls_Path, lk_Files)
	{
		QString ls_Pattern;
		QList<QString> lk_Parts;
		for (int i = 0; i < ls_Path.size(); ++i)
		{
			bool lb_IsDigit = ls_Path[i].toAscii() >= '0' && ls_Path[i].toAscii() <= '9';
			QString ls_Marker = lb_IsDigit ? "0" : "a";
			if (ls_Pattern.isEmpty())
			{
				ls_Pattern = ls_Marker;
				lk_Parts.push_back("");
			}
			if (ls_Pattern.right(1) != ls_Marker)
			{
				ls_Pattern += ls_Marker;
				lk_Parts.push_back("");
			}
			lk_Parts.last() += ls_Path[i];
		}
		if (ls_AllPattern.isEmpty())
			ls_AllPattern = ls_Pattern;
		else
			if (ls_AllPattern != ls_Pattern)
				return QString();
			
		if (lk_AllParts.empty())
			for (int i = 0; i < lk_Parts.size(); ++i)
				lk_AllParts.push_back(QSet<QString>());
			
		for (int i = 0; i < lk_Parts.size(); ++i)
			lk_AllParts[i].insert(lk_Parts[i]);
	}
	
	QString ls_Prefix = "";
	for (int i = 0; i < ls_AllPattern.length(); ++i)
	{
		QStringList lk_Part = lk_AllParts[i].toList();
		if (ls_AllPattern[i] == QChar('a'))
			qSort(lk_Part.begin(), lk_Part.end());
		else
			qSort(lk_Part.begin(), lk_Part.end(), &intStringLessThan);
		
		if (lk_Part.size() == 1)
			ls_Prefix += lk_Part.first();
		else
		{
			if (ls_AllPattern[i] == QChar('0'))
			{
				// we have multiple entries and it's a number part, try to find ranges!
				QString ls_Start;
				QString ls_Stop;
				QString ls_Last;
				QStringList lk_OldPart = lk_Part;
				lk_Part.clear();
				foreach (QString si, lk_OldPart)
				{
					int li_Number = QVariant(si).toInt();
					if (ls_Start.isEmpty())
					{
						ls_Start = si;
						ls_Stop = si;
						ls_Last = si;
						continue;
					}
					if (li_Number == QVariant(ls_Last).toInt() + 1)
					{
						// extend range
						ls_Stop = si;
						ls_Last = si;
						continue;
					}
					else
					{
						if (QVariant(ls_Start).toInt() == QVariant(ls_Stop).toInt())
							lk_Part.push_back(ls_Start);
						else if (QVariant(ls_Start).toInt() + 1 == QVariant(ls_Stop).toInt())
							lk_Part.push_back(ls_Start + "," + ls_Stop);
						else
							lk_Part.push_back(ls_Start + "-" + ls_Stop);
						ls_Start = si;
						ls_Stop = si;
						ls_Last = si;
					}
				}
				if (QVariant(ls_Start).toInt() == QVariant(ls_Stop).toInt())
					lk_Part.push_back(ls_Start);
				else if (QVariant(ls_Start).toInt() + 1 == QVariant(ls_Stop).toInt())
					lk_Part.push_back(ls_Start + "," + ls_Stop);
				else
					lk_Part.push_back(ls_Start + "-" + ls_Stop);
			}
			ls_Prefix += lk_Part.join(",");
		}
	}

	return ls_Prefix;
}


QString k_Script::proposePrefix(QStringList ak_Files)
{
	QString ls_AllPrefix;
	foreach (QString ls_Group, mk_ProposePrefixList)
	{
		QStringList lk_Files;
		foreach (QString ls_Path, ak_Files)
			if (inputGroupForFilename(ls_Path) == ls_Group)
				lk_Files.push_back(ls_Path);
		QString ls_Prefix = mergeFilenames(lk_Files);
		if (!ls_Prefix.isEmpty())
			ls_AllPrefix += ls_Prefix + "-";
	}
	return ls_AllPrefix;
}


QStringList k_Script::ambiguousInputGroups()
{
	return mk_AmbiguousInputGroups;
}


QString k_Script::parameterValue(const QString& as_Key) const
{
	QWidget* lk_Widget_ = mk_ParameterValueWidgets[as_Key];
	if (mk_ParameterMultiChoiceWidgets.contains(as_Key))
	{
		QStringList lk_Choice;
		QList<QWidget*> lk_ChoiceWidgets = mk_ParameterMultiChoiceWidgets[as_Key];
		foreach (QWidget* lk_ChoiceWidget_, lk_ChoiceWidgets)
		{
			QCheckBox* lk_CheckBox_ = dynamic_cast<QCheckBox*>(lk_ChoiceWidget_);
			if (lk_CheckBox_ != NULL && lk_CheckBox_->checkState() == Qt::Checked)
				lk_Choice.append(lk_CheckBox_->property("ProteomaticValue").toString());

			QListWidget* lk_ListWidget_ = dynamic_cast<QListWidget*>(lk_ChoiceWidget_);
			if (lk_ListWidget_ != NULL)
			{
				int li_Count = lk_ListWidget_->count();
				for (int i = 0; i < li_Count; ++i)
				{
					QListWidgetItem* lk_Item_ = lk_ListWidget_->item(i);
					lk_Choice.append(lk_Item_->data(Qt::UserRole).toString());
				}
			}
		}
		return lk_Choice.join(",");
	}
	else
	{
		if (dynamic_cast<QDoubleSpinBox*>(lk_Widget_) != NULL)
			return QVariant(dynamic_cast<QDoubleSpinBox*>(lk_Widget_)->value()).toString();
		else if (dynamic_cast<QSpinBox*>(lk_Widget_) != NULL)
			return QVariant(dynamic_cast<QSpinBox*>(lk_Widget_)->value()).toString();
		else if (dynamic_cast<QLineEdit*>(lk_Widget_) != NULL)
			return QVariant(dynamic_cast<QLineEdit*>(lk_Widget_)->text()).toString();
		else if (dynamic_cast<QComboBox*>(lk_Widget_) != NULL)
		{
			QComboBox* lk_ComboBox_ = dynamic_cast<QComboBox*>(lk_Widget_);
			return QVariant(lk_ComboBox_->itemData(lk_ComboBox_->currentIndex())).toString();
		}
		else if (dynamic_cast<QCheckBox*>(lk_Widget_) != NULL)
		{
			QCheckBox* lk_CheckBox_ = dynamic_cast<QCheckBox*>(lk_Widget_);
			return lk_CheckBox_->checkState() == Qt::Checked ? "true" : "false";
		}
	}
	return "";
}


QString k_Script::parameterDefault(const QString& as_Key) const
{
	return mk_DefaultConfiguration[as_Key];
}


QString k_Script::humanReadableParameterValue(const QString& as_Key, const QString& as_Value) const
{
	QString ls_Result;
	QString ls_Type = mk_ParameterDefs[as_Key]["type"];
	if (ls_Type == "csvString")
	{
		QStringList lk_Values = as_Value.split(",");
		for (int i = 0; i < lk_Values.size(); ++i)
		{
			QString ls_Value = lk_Values.at(i);
			ls_Result += mk_ParameterValueLabels[as_Key][ls_Value];
			if (i < lk_Values.size() - 1)
				ls_Result += ", ";
		}
	}
	else
	{
		if (ls_Type == "float" || ls_Type == "int" || ls_Type == "string")
		{
			if (mk_ParameterDefs[as_Key].contains("prefix"))
				ls_Result += mk_ParameterDefs[as_Key]["prefix"] + " ";
			ls_Result = as_Value;
			if (mk_ParameterDefs[as_Key].contains("suffix"))
				ls_Result += " " + mk_ParameterDefs[as_Key]["suffix"];
		}
		else if (ls_Type == "enum")
		{
			ls_Result = mk_ParameterValueLabels[as_Key][as_Value];
			if (ls_Result == "")
				ls_Result = as_Value;
		}
		else if (ls_Type == "flag")
			ls_Result = (as_Value == "true")? "yes": "no";
	}
	
	if (ls_Result == "")
		ls_Result = "<i>empty</i>";
		
	return ls_Result;
}


void k_Script::setParameter(const QString& as_Key, const QString& as_Value)
{
	if (!mk_ParameterValueWidgets.contains(as_Key))
		return;
	
	QWidget* lk_Widget_ = mk_ParameterValueWidgets[as_Key];
	
	if (mb_ProfileMode)
	{
		QCheckBox* lk_CheckBox_ = dynamic_cast<QCheckBox*>(mk_WidgetLabelsOrCheckBoxes[as_Key]);
		if (lk_CheckBox_ != NULL)
			lk_CheckBox_->setCheckState(Qt::Checked);
	}
	
	if (mk_ParameterMultiChoiceWidgets.contains(as_Key))
	{
		QStringList lk_Choices = as_Value.split(",");
		QList<QWidget*> lk_ChoiceWidgets = mk_ParameterMultiChoiceWidgets[as_Key];
		foreach (QWidget* lk_ChoiceWidget_, lk_ChoiceWidgets)
		{
			QCheckBox* lk_CheckBox_ = dynamic_cast<QCheckBox*>(lk_ChoiceWidget_);
			if (lk_CheckBox_ != NULL)
				lk_CheckBox_->setChecked(lk_Choices.contains(lk_CheckBox_->property("ProteomaticValue").toString()));

			k_FileList* lk_ListWidget_ = dynamic_cast<k_FileList*>(lk_ChoiceWidget_);
			if (lk_ListWidget_ != NULL)
			{
				lk_ListWidget_->selectAll();
				lk_ListWidget_->forceRemove(lk_ListWidget_->selectedItems());
				addChoiceItems(as_Key, lk_Choices);
			}
		}
		parameterChangedWithKey(as_Key);
	}
	else
	{
		if (dynamic_cast<QDoubleSpinBox*>(lk_Widget_) != NULL)
			(dynamic_cast<QDoubleSpinBox*>(lk_Widget_))->setValue(QVariant(as_Value).toDouble());
		else if (dynamic_cast<QSpinBox*>(lk_Widget_) != NULL)
			(dynamic_cast<QSpinBox*>(lk_Widget_))->setValue(QVariant(as_Value).toInt());
		else if (dynamic_cast<QLineEdit*>(lk_Widget_) != NULL)
			(dynamic_cast<QLineEdit*>(lk_Widget_))->setText(as_Value);
		else if (dynamic_cast<QComboBox*>(lk_Widget_) != NULL)
			(dynamic_cast<QComboBox*>(lk_Widget_))->setCurrentIndex((dynamic_cast<QComboBox*>(lk_Widget_))->findData(as_Value));
		else if (dynamic_cast<QCheckBox*>(lk_Widget_) != NULL)
			(dynamic_cast<QCheckBox*>(lk_Widget_))->setCheckState(as_Value == "true"? Qt::Checked : Qt::Unchecked);
	}
}


QStringList k_Script::parameterKeys() const
{
	return mk_ParameterKeys;
}


QString k_Script::parameterLabel(const QString& as_Key) const
{
	return mk_ParameterDefs[as_Key]["label"];
}


QString k_Script::humanReadableParameterValue(const QString& as_Key) const
{
	return this->humanReadableParameterValue(as_Key, this->parameterValue(as_Key));
}


QHash<QString, QString> k_Script::configuration() const
{
	QHash<QString, QString> lk_Result;

	foreach (QString ls_Key, mk_ParameterValueWidgets.keys())
		lk_Result[ls_Key] = parameterValue(ls_Key);

	return lk_Result;
}


QHash<QString, QString> k_Script::defaultConfiguration() const
{
	return mk_DefaultConfiguration;
}


QHash<QString, QString> k_Script::nonDefaultConfiguration() const
{
	QHash<QString, QString> lk_Result;

	foreach (QString ls_Key, mk_ParameterValueWidgets.keys())
	{
		if (mk_DefaultConfiguration[ls_Key] != parameterValue(ls_Key))
			lk_Result[ls_Key] = parameterValue(ls_Key);
	}

	return lk_Result;
}


void k_Script::setConfiguration(const QHash<QString, QString>& ak_Configuration)
{
	// for files, include version number and script URI basename!!
	QHash<QString, QString>::const_iterator lk_Iter = ak_Configuration.constBegin();
	while (lk_Iter != ak_Configuration.constEnd()) 
	{
		this->setParameter(lk_Iter.key(), lk_Iter.value());
		++lk_Iter;
	}
}


tk_YamlMap k_Script::profile() const
{
	tk_YamlMap lk_Profile;
	if (!mb_ProfileMode)
		return lk_Profile;
		
	foreach (QString ls_Key, mk_ParameterKeys)
	{
		QWidget* lk_Widget_ = mk_WidgetLabelsOrCheckBoxes[ls_Key];
		QCheckBox* lk_CheckBox_ = dynamic_cast<QCheckBox*>(lk_Widget_);
		if (lk_CheckBox_ != NULL)
		{
			if (lk_CheckBox_->checkState() == Qt::Checked)
				lk_Profile[ls_Key] = this->parameterValue(ls_Key);
		}
	}
	return lk_Profile;
}


void k_Script::applyProfile(const tk_YamlMap& ak_Profile)
{
	foreach (QString ls_Key, mk_ParameterKeys)
		if (ak_Profile.contains(ls_Key))
			this->setParameter(ls_Key, ak_Profile[ls_Key].toString());
}


void k_Script::addChoiceItems(QString as_Key, QStringList ak_Choices)
{
	// find file list child in sender dialog
	QWidget* lk_Widget_ = mk_ParameterMultiChoiceWidgets[as_Key].first();
	QListWidget* lk_Target_ = dynamic_cast<QListWidget*>(lk_Widget_);
	if (lk_Target_ == NULL)
		return;

	QListWidget* lk_Source_ = lk_Target_->findChild<QListWidget*>("choices-stock");
	if (lk_Source_ == NULL)
		return;
		
	QList<QListWidgetItem*> lk_ItemsToBeDeleted;
	
	for (int i = 0; i < lk_Source_->count(); ++i)
	{
		QListWidgetItem* lk_Item_ = lk_Source_->item(i);
		if (ak_Choices.contains(lk_Item_->data(Qt::UserRole).toString()))
		{
			k_CiListWidgetItem* lk_NewItem_ = new k_CiListWidgetItem(lk_Item_->text(), lk_Target_);
			lk_NewItem_->setData(Qt::UserRole, lk_Item_->data(Qt::UserRole));
			lk_ItemsToBeDeleted.push_back(lk_Item_);
		}
	}
	
	foreach (QListWidgetItem* lk_Item_, lk_ItemsToBeDeleted)
		delete lk_Item_;
}


void k_Script::addChoiceItems()
{
	// find file list child in sender dialog
	QDialog* lk_Dialog_ = dynamic_cast<QDialog*>(sender());
	if (lk_Dialog_ == NULL)
		return;

	QListWidget* lk_Source_ = lk_Dialog_->findChild<QListWidget*>("choices-stock");
	if (lk_Source_ == NULL)
		return;

	QListWidget* lk_Target_ = dynamic_cast<QListWidget*>(lk_Dialog_->parent());
	if (lk_Target_ == NULL)
		return;

	QList<QListWidgetItem *> lk_Items = lk_Source_->selectedItems();
	foreach (QListWidgetItem* lk_Item_, lk_Items)
	{
		k_CiListWidgetItem* lk_NewItem_ = new k_CiListWidgetItem(lk_Item_->text(), lk_Target_);
		lk_NewItem_->setData(Qt::UserRole, lk_Item_->data(Qt::UserRole));
		delete lk_Item_;
	}
}


void k_Script::removeChoiceItems(QList<QListWidgetItem *> ak_Items)
{
	QListWidget* lk_Source_ = dynamic_cast<QListWidget*>(sender());
	if (lk_Source_ == NULL)
		return;

	QListWidget* lk_Target_ = lk_Source_->findChild<QListWidget*>("choices-stock");
	if (lk_Target_ == NULL)
		return;

	foreach (QListWidgetItem* lk_Item_, ak_Items)
	{
		k_CiListWidgetItem* lk_NewItem_ = new k_CiListWidgetItem(lk_Item_->text(), lk_Target_);
		lk_NewItem_->setData(Qt::UserRole, lk_Item_->data(Qt::UserRole));
		delete lk_Item_;
	}
	
	parameterChangedWithKey(lk_Source_->property("key").toString());
}


void k_Script::clearOutputDirectoryButtonClicked()
{
	if (mk_pOutputDirectory)
		mk_pOutputDirectory->clear();
}


// BIG TODO: prefix proposal does not take amibiguous input files into account,
// any file which is ambiguous is just counted as the first group, not according
// to how the user actually assigned the file

QString k_Script::inputGroupForFilename(const QString& as_Path) const
{
	QString ls_Path = as_Path.toLower();
	foreach (QString ls_Key, mk_InputGroupKeys)
	{
		foreach (QString ls_Extension, mk_InputGroupExtensions[ls_Key])
		{
			if (ls_Path.endsWith(ls_Extension.toLower()))
				return ls_Key;
		}
	}
	return "";
}


QString k_Script::defaultOutputDirectoryInputGroup() const
{
	return ms_DefaultOutputDirectory;
}


void k_Script::setOutputDirectoryButtonClicked()
{
	if (!mk_pOutputDirectory)
		return;
	
	QString ls_Path = QFileDialog::getExistingDirectory(mk_pParameterWidget.get_Pointer(), tr("Select output directory"), mk_Proteomatic.getConfiguration(CONFIG_REMEMBER_OUTPUT_PATH).toString());
	if (ls_Path.length() > 0)
	{
		this->setOutputDirectory(ls_Path);
		mk_Proteomatic.getConfigurationRoot()[CONFIG_REMEMBER_OUTPUT_PATH] = ls_Path;
	}
}


void k_Script::toggleUi()
{
	this->adjustDependentParameters();
	if (mk_pClearOutputDirectoryButton)
		mk_pClearOutputDirectoryButton->setEnabled(!mk_pOutputDirectory->text().isEmpty());
}


void k_Script::resetDialog()
{
	QDialog* lk_Dialog_ = dynamic_cast<QDialog*>(sender());
	if (lk_Dialog_ == NULL)
		return;

	QListWidget* lk_ListWidget_ = lk_Dialog_->findChild<QListWidget*>();
	if (lk_ListWidget_ != NULL)
	{
		lk_ListWidget_->clearSelection();
		lk_ListWidget_->setFocus();
	}
}


void k_Script::createParameterWidget(QString as_Info)
{
    ms_DefaultOutputDirectory.clear();
	mk_ProposePrefixList.clear();
	mk_AmbiguousInputGroups.clear();
	mk_InputGroupKeys.clear();
	mk_InputGroupLabels.clear();
	mk_InputGroupDescriptions.clear();
	mk_InputGroupExtensions.clear();
	mk_pParameterWidget = RefPtr<QWidget>(new QWidget());

	QList<QString> lk_ParametersOrder;
	QHash<QString, QHash<QString, QString> > lk_Parameters;
	QHash<QString, QList<QVariant> > lk_ParametersValues;
    
    tk_YamlMap lk_Info = k_Yaml::parseFromString(as_Info).toMap();

    if (lk_Info.contains("type"))
        me_Type = (lk_Info["type"].toString() == "processor") ? r_ScriptType::Processor : r_ScriptType::Converter;
    if (lk_Info.contains("converterKey"))
        mk_Info["converterKey"] = lk_Info["converterKey"].toString();
    if (lk_Info.contains("converterLabel"))
        mk_Info["converterLabel"] = lk_Info["converterLabel"].toString();
    if (lk_Info.contains("converterFilename"))
        mk_Info["converterFilename"] = lk_Info["converterFilename"].toString();
    if (lk_Info.contains("parameters"))
    {
        foreach (QVariant lk_ParameterMap, lk_Info["parameters"].toList())
        {
            QHash<QString, QString> lk_Parameter;
            QList<QVariant> lk_EnumValues;
            foreach (QString ls_Key, lk_ParameterMap.toMap().keys())
            {
                if (ls_Key == "choices")
                    foreach (QVariant lk_Choice, lk_ParameterMap.toMap()[ls_Key].toList())
                        lk_EnumValues << lk_Choice;
                else
                    lk_Parameter[ls_Key] = lk_ParameterMap.toMap()[ls_Key].toString();
            }
            if (lk_Parameter["key"].length() > 0)
            {
                lk_Parameters[lk_Parameter["key"]] = lk_Parameter;
                lk_ParametersValues[lk_Parameter["key"]] = lk_EnumValues;
                lk_ParametersOrder.push_back(lk_Parameter["key"]);
            }
            foreach (QVariant lk_Item, lk_ParametersValues[lk_Parameter["key"]])
            {
                QString ls_Key, ls_Label;
                if (lk_Item.canConvert<tk_YamlMap>())
                {
                    ls_Key = lk_Item.toMap().keys().first();
                    ls_Label = lk_Item.toMap()[ls_Key].toString();
                }
                else
                    ls_Key = ls_Label = lk_Item.toString();
                
                if (!mk_ParameterValueLabels.contains(lk_Parameter["key"]))
                    mk_ParameterValueLabels[lk_Parameter["key"]] = QHash<QString, QString>();
                mk_ParameterValueLabels[lk_Parameter["key"]][ls_Key] = ls_Label;
            }
        }
    }
    if (lk_Info.contains("input"))
    {
        foreach (QVariant lk_InputMap, lk_Info["input"].toList())
        {
            tk_YamlMap lk_Map;
            foreach (QString ls_Key, lk_InputMap.toMap().keys())
            {
                QString ls_Value = lk_InputMap.toMap()[ls_Key].toString();
                lk_Map[ls_Key] = ls_Value;
            }
            mk_InputGroupKeys << lk_Map["key"].toString();
            if (lk_Map.contains("label"))
                mk_InputGroupLabels[lk_Map["key"].toString()] = lk_Map["label"].toString();
            if (lk_Map.contains("description"))
                mk_InputGroupDescriptions[lk_Map["key"].toString()] = lk_Map["description"].toString();
            if (lk_Map.contains("extensions"))
                mk_InputGroupExtensions[lk_Map["key"].toString()] = lk_Map["extensions"].toString().split("/");
            if (lk_Map.contains("min"))
                mk_InputGroupMinimum[lk_Map["key"].toString()] = lk_Map["min"].toString().toInt();
            if (lk_Map.contains("max"))
                mk_InputGroupMaximum[lk_Map["key"].toString()] = lk_Map["max"].toString().toInt();
        }
    }
    
    if (lk_Info.contains("defaultOutputDirectory"))
        ms_DefaultOutputDirectory = lk_Info["defaultOutputDirectory"].toString();

    if (lk_Info.contains("proposePrefixList"))
    {
        mk_ProposePrefixList.clear();
        foreach (QVariant lk_Item, lk_Info["proposePrefixList"].toList())
            mk_ProposePrefixList << lk_Item.toString();
    }

    if (lk_Info.contains("ambiguousInputGroups"))
    {
        mk_AmbiguousInputGroups.clear();
        foreach (QVariant lk_Item, lk_Info["ambiguousInputGroups"].toList())
            mk_AmbiguousInputGroups << lk_Item.toString();
    }
	
	// TODO: finish this. Right now only leading {...} are stripped but
	// not taken into account for sorting the parameter groups.
	// adjust lk_ParametersOrder - {1} makes a group appear at the front,
	// these go into lk_Positives. No {...} makes a group appear in the 
	// middle, these go into lk_Remaining. {-1} makes a group appear at
	// the end of the parameters list, these go into lk_Negatives.
	// Make of that what you want, it was done to shift the target/decoy
	// options up in the Run OMSSA script, because they are quite important.
	// Probably there's something better.
	
	QStringList lk_Positives;
	QStringList lk_Remaining;
	QStringList lk_Negatives;
	foreach (QString ls_Key, lk_ParametersOrder)
	{
		QString ls_Group = lk_Parameters[ls_Key]["group"].trimmed();
		int li_StartIndex = ls_Group.indexOf("{");
		int li_EndIndex = ls_Group.indexOf("}");
		if (li_StartIndex == 0 && li_EndIndex > 0)
		{
			QString ls_Number = ls_Group.mid(li_StartIndex + 1, li_EndIndex - li_StartIndex - 1).trimmed();
			ls_Group.remove(li_StartIndex, li_EndIndex - li_StartIndex + 1);
			ls_Group = ls_Group.trimmed();
			lk_Parameters[ls_Key]["group"] = ls_Group;
		}
	}
	mk_ParameterKeys = lk_ParametersOrder;

	QHash<QString, QWidget* > lk_Containers;
	QHash<QString, QGridLayout* > lk_GridLayouts;
	QHash<QString, int> lk_GroupBoxX;
	QHash<QString, int> lk_GroupBoxY;
	int li_ContainersAdded = 0;

	QWidget* lk_InternalWidget_ = mk_pParameterWidget.get_Pointer();
	QVBoxLayout* lk_ParameterLayout_ = new QVBoxLayout(lk_InternalWidget_);
	lk_InternalWidget_->setWindowIcon(QIcon(":/icons/proteomatic.png"));
	lk_InternalWidget_->setWindowTitle(ms_Title);
	lk_InternalWidget_->setWindowFlags(Qt::WindowStaysOnTopHint);
	if (!mb_ProfileMode)
	{
		QLabel* lk_Label_ = new QLabel("<b>" + title() + "</b>", lk_InternalWidget_);
		lk_ParameterLayout_->addWidget(lk_Label_);
		if (!description().isEmpty())
		{
			lk_Label_ = new QLabel("<i></i>" + description(), lk_InternalWidget_);
			lk_Label_->setWordWrap(true);
			lk_ParameterLayout_->addWidget(lk_Label_);
		}
		if (!mk_InputGroupKeys.empty())
		{
			QString ls_List;
			foreach (QString ls_Item, mk_InputGroupKeys)
				ls_List += "<li>" + mk_InputGroupDescriptions[ls_Item] + "</li>";
			lk_Label_ = new QLabel("Input files:<ul>" + ls_List + "</ul>", lk_InternalWidget_);
			lk_Label_->setWordWrap(true);
			lk_ParameterLayout_->addWidget(lk_Label_);
		}
		if (!ms_DefaultOutputDirectory.isEmpty())
		{
			lk_Label_ = new QLabel("Unless an output directory is specified, the output files will be written to the directory of one of the " + mk_InputGroupLabels[ms_DefaultOutputDirectory] + " files.", lk_InternalWidget_);
			lk_Label_->setWordWrap(true);
			lk_ParameterLayout_->addWidget(lk_Label_);
		}
	}
	
	if (!mb_ProfileMode)
	{
		QFrame* lk_Frame_ = new QFrame(lk_InternalWidget_);
		lk_Frame_->setFrameStyle(QFrame::HLine | QFrame::Sunken);
		lk_ParameterLayout_->addWidget(lk_Frame_);
	}
	
	//lk_ParameterLayout_->setContentsMargins(0, 0, 0, 0);
	//mk_UpperLayout_->insertWidget(3, lk_InternalWidget_);

	//lk_InternalWidget_->show();

	foreach (QString ls_Key, lk_ParametersOrder)
	{
		mk_ParameterDefs[ls_Key] = lk_Parameters[ls_Key];
		if (mk_ParameterDefs[ls_Key].contains("enabled"))
			mk_DependentParameters.push_back(ls_Key);
		if (ls_Key.startsWith("output") && ls_Key != "outputPrefix" && ls_Key != "outputDirectory")
			mk_OutFileDetails[ls_Key] = lk_Parameters[ls_Key];

		if (!mb_IncludeOutputFiles && ls_Key.startsWith("output"))
			continue;

		QString ls_Group = lk_Parameters[ls_Key]["group"];
		if (!mk_GroupParameters.contains(ls_Group))
			mk_GroupParameters[ls_Group] = QStringList();
		mk_GroupParameters[ls_Group].push_back(ls_Key);
		
		if (lk_Containers[ls_Group] == NULL)
		{
			QWidget* lk_Container_ = new QWidget(lk_InternalWidget_);
			k_FoldedHeader* lk_Header_ = new k_FoldedHeader("<b>" + ls_Group + "</b>", lk_Container_, lk_InternalWidget_);
			mk_FoldedHeaders[ls_Group] = lk_Header_;
			QFrame* lk_Frame_ = new QFrame(lk_InternalWidget_);
			lk_Frame_->setFrameStyle(QFrame::HLine | QFrame::Sunken);

			if (li_ContainersAdded < 1)
				lk_Header_->showBuddy();
			else
				lk_Header_->hideBuddy();
				
			lk_Container_->setContentsMargins(2, 0, 2, 0);
	
			lk_ParameterLayout_->addWidget(lk_Header_);
			lk_ParameterLayout_->addWidget(lk_Container_);
			lk_ParameterLayout_->addWidget(lk_Frame_);
			//lk_TabWidget_->addWidget(lk_Container_);

			QGridLayout* lk_GridLayout_ = new QGridLayout(lk_Container_);
			lk_Container_->setLayout(lk_GridLayout_);
			lk_GridLayout_->setMargin(0);

			lk_Containers[ls_Group] = lk_Container_;
			lk_GridLayouts[ls_Group] = lk_GridLayout_;
			lk_GroupBoxX[ls_Group] = 0;
			lk_GroupBoxY[ls_Group] = 0;
			li_ContainersAdded += 1;
		}
	}

	foreach (QString ls_Key, lk_ParametersOrder)
	{
		if (!mb_IncludeOutputFiles && ls_Key.startsWith("output"))
			continue;

		QHash<QString, QString> lk_Parameter = lk_Parameters[ls_Key];
		QString ls_Group = lk_Parameter["group"];
		QWidget* lk_Container_ = lk_Containers[ls_Group];

		QBoxLayout* lk_Layout_ = new QBoxLayout(QBoxLayout::LeftToRight);
		lk_Layout_->setContentsMargins(0, 0, 0, 0);
		lk_Layout_->setSpacing(5);
		lk_GridLayouts[ls_Group]->addLayout(lk_Layout_, lk_GroupBoxY[ls_Group], lk_GroupBoxX[ls_Group], 1, 1);
		lk_GroupBoxX[ls_Group] = 0;
		lk_GroupBoxY[ls_Group] += 1;

		bool lb_AddLabel = true;
		bool lb_WidgetFirst = false;

		bool lb_Ok;

		// use lk_Widget_ for insertion into layout and as value unless
		// lk_ValueWidget_ != NULL, then use this as the value!!
		QWidget* lk_Widget_ = NULL;
		QWidget* lk_ValueWidget_ = NULL;
		
		QString ls_Type = lk_Parameter["type"];
		QString ls_Key = lk_Parameter["key"];
		
		QString ls_Description = "";
		if (lk_Parameter.contains("description"))
			ls_Description = lk_Parameter["description"] + " ";
		ls_Description += QString("(default: %1)").arg(humanReadableParameterValue(ls_Key, lk_Parameter["default"]));
		
		if (ls_Type == "float")
		{
			QDoubleSpinBox* lk_SpinBox_ = new QDoubleSpinBox(lk_Container_);
			lk_Widget_ = lk_SpinBox_;
			if (lk_Parameter.contains("decimals"))
				lk_SpinBox_->setDecimals(QVariant(lk_Parameter["decimals"]).toInt(&lb_Ok));
			if (lk_Parameter.contains("min"))
				lk_SpinBox_->setMinimum(QVariant(lk_Parameter["min"]).toDouble(&lb_Ok));
			else
				lk_SpinBox_->setMinimum(DBL_MIN);
			if (lk_Parameter.contains("max"))
				lk_SpinBox_->setMaximum(QVariant(lk_Parameter["max"]).toDouble(&lb_Ok));
			else
				lk_SpinBox_->setMaximum(DBL_MAX);
			if (lk_Parameter.contains("suffix"))
				lk_SpinBox_->setSuffix(" " + lk_Parameter["suffix"]);
			if (lk_Parameter.contains("prefix"))
				lk_SpinBox_->setPrefix(lk_Parameter["prefix"] + " ");
			if (lk_Parameter.contains("step"))
				lk_SpinBox_->setSingleStep(QVariant(lk_Parameter["step"]).toDouble(&lb_Ok));
			lk_SpinBox_->setValue(QVariant(lk_Parameter["default"]).toDouble(&lb_Ok));
			
			lk_SpinBox_->setProperty("key", QVariant(ls_Key));
			connect(lk_SpinBox_, SIGNAL(valueChanged(double)), this, SLOT(parameterChanged()));
		} 
		else if (ls_Type == "int")
		{
			QSpinBox* lk_SpinBox_ = new QSpinBox(lk_Container_);
			lk_Widget_ = lk_SpinBox_;
			if (lk_Parameter.contains("min"))
				lk_SpinBox_->setMinimum(QVariant(lk_Parameter["min"]).toInt(&lb_Ok));
			else
				lk_SpinBox_->setMinimum(LONG_MIN);
			if (lk_Parameter.contains("max"))
				lk_SpinBox_->setMaximum(QVariant(lk_Parameter["max"]).toInt(&lb_Ok));
			else
				lk_SpinBox_->setMaximum(LONG_MAX);
			if (lk_Parameter.contains("suffix"))
				lk_SpinBox_->setSuffix(" " + lk_Parameter["suffix"]);
			if (lk_Parameter.contains("prefix"))
				lk_SpinBox_->setPrefix(lk_Parameter["prefix"] + " ");
			if (lk_Parameter.contains("step"))
				lk_SpinBox_->setSingleStep(QVariant(lk_Parameter["step"]).toInt(&lb_Ok));
			lk_SpinBox_->setValue(QVariant(lk_Parameter["default"]).toInt(&lb_Ok));
		
			lk_SpinBox_->setProperty("key", QVariant(ls_Key));
			connect(lk_SpinBox_, SIGNAL(valueChanged(int)), this, SLOT(parameterChanged()));
		} 
		else if (ls_Type == "string")
		{
			QLineEdit* lk_LineEdit_ = new QLineEdit(lk_Container_);
			lk_Widget_ = lk_LineEdit_;
			lk_LineEdit_->setText(lk_Parameter["default"]);
			if (!ls_Key.startsWith("output"))
			{
				lk_LineEdit_->setProperty("key", QVariant(ls_Key));
				connect(lk_LineEdit_, SIGNAL(textChanged(const QString&)), this, SLOT(parameterChanged()));
			}
			
			if (ls_Key == "outputDirectory")
			{
				mk_pOutputDirectory = RefPtr<QLineEdit>(lk_LineEdit_);
				QWidget* lk_SubContainer_ = new QWidget(lk_Container_);
				lk_SubContainer_->setContentsMargins(0, 0, 0, 0);
				QBoxLayout* lk_GroupBoxLayout_ = new QHBoxLayout(lk_SubContainer_);
				lk_GroupBoxLayout_->setMargin(0);
				lk_GroupBoxLayout_->addWidget(lk_LineEdit_);
				QToolButton* lk_ClearOutputDirectoryButton_ = new QToolButton(lk_SubContainer_);
				lk_ClearOutputDirectoryButton_->setIcon(QIcon(":/icons/dialog-cancel.png"));
				mk_pClearOutputDirectoryButton = RefPtr<QToolButton>(lk_ClearOutputDirectoryButton_);
				connect(lk_ClearOutputDirectoryButton_, SIGNAL(clicked()), this, SLOT(clearOutputDirectoryButtonClicked()));
				((QHBoxLayout*)lk_GroupBoxLayout_)->addWidget(lk_ClearOutputDirectoryButton_, 0, Qt::AlignTop);
				QToolButton* lk_SetOutputDirectoryButton_ = new QToolButton(lk_SubContainer_);
				lk_SetOutputDirectoryButton_->setIcon(QIcon(":/icons/folder.png"));
				connect(lk_SetOutputDirectoryButton_, SIGNAL(clicked()), this, SLOT(setOutputDirectoryButtonClicked()));
				((QHBoxLayout*)lk_GroupBoxLayout_)->addWidget(lk_SetOutputDirectoryButton_, 0, Qt::AlignTop);
				lk_Container_->setLayout(lk_GroupBoxLayout_);
				lk_LineEdit_->setReadOnly(true);
				connect(lk_LineEdit_, SIGNAL(textChanged(const QString&)), this, SLOT(toggleUi()));
				lk_Widget_ = lk_SubContainer_;
				lk_ValueWidget_ = lk_LineEdit_;
			}
			else if (ls_Key == "outputPrefix")
			{
				mk_pOutputPrefix = RefPtr<QLineEdit>(lk_LineEdit_);
				QWidget* lk_SubContainer_ = new QWidget(lk_Container_);
				lk_SubContainer_->setContentsMargins(0, 0, 0, 0);
				QBoxLayout* lk_GroupBoxLayout_ = new QHBoxLayout(lk_SubContainer_);
				lk_GroupBoxLayout_->setMargin(0);
				lk_GroupBoxLayout_->addWidget(lk_LineEdit_);
				QToolButton* lk_ProposePrefixButton_ = new QToolButton(lk_SubContainer_);
				lk_ProposePrefixButton_->setIcon(QIcon(":/icons/select-continuous-area.png"));
				//lk_ProposePrefixButton_->setHint("Let Proteomatic propose a prefix");
				connect(lk_ProposePrefixButton_, SIGNAL(clicked()), this, SIGNAL(proposePrefixButtonClicked()));
				((QHBoxLayout*)lk_GroupBoxLayout_)->addWidget(lk_ProposePrefixButton_, 0, Qt::AlignTop);
				lk_Container_->setLayout(lk_GroupBoxLayout_);
				lk_Widget_ = lk_SubContainer_;
				lk_ValueWidget_ = lk_LineEdit_;
			}
		}
		else if (ls_Type == "flag")
		{
			if (mb_ProfileMode)
			{
				QComboBox* lk_ComboBox_ = new QComboBox(lk_Container_);
				lk_Widget_ = lk_ComboBox_;
				lk_ComboBox_->addItem("yes", QVariant("true"));
				lk_ComboBox_->addItem("no", QVariant("false"));
				lk_ComboBox_->setCurrentIndex((lk_Parameter["default"] == "true") || (lk_Parameter["default"] == "yes") ? 0 : 1);
				
				lk_ComboBox_->setProperty("key", QVariant(ls_Key));
				connect(lk_ComboBox_, SIGNAL(currentIndexChanged(int)), this, SLOT(parameterChanged()));
			}
			else
			{
				lb_WidgetFirst = true;
				QCheckBox* lk_CheckBox_ = new QCheckBox(lk_Container_);
				lk_Widget_ = lk_CheckBox_;
				lk_CheckBox_->setChecked((lk_Parameter["default"] == "true") || (lk_Parameter["default"] == "yes"));
				lk_CheckBox_->setProperty("key", QVariant(ls_Key));
				if (!ls_Key.startsWith("output"))
					connect(lk_CheckBox_, SIGNAL(stateChanged(int)), this, SLOT(parameterChanged()));
			}
		}
		else if (ls_Type == "enum")
		{
			QComboBox* lk_ComboBox_ = new QComboBox(lk_Container_);
			lk_Widget_ = lk_ComboBox_;
			int li_Index = 0;
			int li_DefaultIndex = 0;
			foreach (QVariant lk_Item, lk_ParametersValues[ls_Key])
			{
                QString ls_ValueKey;
                QString ls_Label;
                if (lk_Item.canConvert<tk_YamlMap>())
                {
                    ls_ValueKey = lk_Item.toMap().keys().first();
                    ls_Label = lk_Item.toMap()[ls_ValueKey].toString();
                }
                else
                    ls_ValueKey = ls_Label = lk_Item.toString();
                
				lk_ComboBox_->addItem(ls_Label, QVariant(ls_ValueKey));
				if (ls_ValueKey == lk_Parameter["default"])
					li_DefaultIndex = li_Index;
				li_Index += 1;
			}
			lk_ComboBox_->setCurrentIndex(li_DefaultIndex);
			
			lk_ComboBox_->setProperty("key", QVariant(ls_Key));
			connect(lk_ComboBox_, SIGNAL(currentIndexChanged(int)), this, SLOT(parameterChanged()));
		}
		else if (ls_Type == "csvString")
		{
			QList<QWidget*> lk_CheckWidgets;
			QStringList lk_DefaultChecks = lk_Parameter["default"].split(",");
			bool lb_Row = false;
			if (lk_Parameter.contains("display"))
				lb_Row = lk_Parameter["display"] == "row";
			if (lb_Row)
			{
				lk_Widget_ = new QWidget(lk_Container_);
				QHBoxLayout* lk_WidgetLayout_ = new QHBoxLayout(lk_Widget_);
				lk_WidgetLayout_->addStretch();
				foreach (QVariant lk_Item, lk_ParametersValues[ls_Key])
				{
					QCheckBox* lk_CheckBox_ = new QCheckBox(lk_Widget_);
                    QString ls_ValueKey;
                    QString ls_Label;
                    if (lk_Item.canConvert<tk_YamlMap>())
                    {
                        ls_ValueKey = lk_Item.toMap().keys().first();
                        ls_Label = lk_Item.toMap()[ls_ValueKey].toString();
                    }
                    else
                        ls_ValueKey = ls_Label = lk_Item.toString();
                    
					lk_CheckBox_->setText(ls_Label);
					lk_WidgetLayout_->addWidget(lk_CheckBox_);
					lk_CheckBox_->setChecked(lk_DefaultChecks.contains(ls_ValueKey));
					lk_CheckWidgets.append(lk_CheckBox_);
					lk_CheckBox_->setProperty("ProteomaticValue", QVariant(ls_ValueKey));
			
					lk_CheckBox_->setProperty("key", QVariant(ls_Key));
					connect(lk_CheckBox_, SIGNAL(stateChanged(int)), this, SLOT(parameterChanged()));
				}
				lk_WidgetLayout_->setMargin(0);
			}
			else
			{
				// TODO: auto add default selected items in list widget!
				lk_Layout_->setDirection(QBoxLayout::TopToBottom);
				lk_Widget_ = new QWidget(lk_Container_);
				QHBoxLayout* lk_WidgetLayout_ = new QHBoxLayout(lk_Widget_);
				QVBoxLayout* lk_ButtonLayout_ = new QVBoxLayout(lk_Widget_);
				k_FileList* lk_ListWidget_ = new k_FileList(lk_Widget_, false);
				lk_ListWidget_->setSelectionMode(QAbstractItemView::ExtendedSelection);
				lk_WidgetLayout_->addWidget(lk_ListWidget_);
				QToolButton* lk_AddButton_ = new QToolButton(lk_Widget_);
				lk_AddButton_->setIcon(QIcon(":/icons/list-add.png"));
				lk_ButtonLayout_->addWidget(lk_AddButton_);
				QToolButton* lk_RemoveButton_ = new QToolButton(lk_Widget_);
				lk_RemoveButton_->setEnabled(false);
				lk_RemoveButton_->setIcon(QIcon(":/icons/list-remove.png"));
				connect(lk_RemoveButton_, SIGNAL(clicked()), lk_ListWidget_, SLOT(removeSelection()));
				
				lk_RemoveButton_->setProperty("key", QVariant(ls_Key));
				lk_ListWidget_->setProperty("key", QVariant(ls_Key));
				
				connect(lk_ListWidget_, SIGNAL(selectionChanged(bool)), lk_RemoveButton_, SLOT(setEnabled(bool)));
				lk_ButtonLayout_->addWidget(lk_RemoveButton_);
				lk_ButtonLayout_->addStretch();
				lk_WidgetLayout_->addLayout(lk_ButtonLayout_);
				lk_WidgetLayout_->setMargin(0);
				lk_ButtonLayout_->setMargin(0);
				lk_ButtonLayout_->setSpacing(2);
				lk_WidgetLayout_->setSpacing(2);
				lk_CheckWidgets.append(lk_ListWidget_);
				QDialog* lk_Dialog_ = new QDialog(lk_ListWidget_);
				mk_ParameterMultiChoiceDialogs[ls_Key] = lk_Dialog_;
				lk_Dialog_->setModal(true);
				lk_Dialog_->setWindowTitle(lk_Parameter["label"]);
				lk_Dialog_->resize(240, 280);
				QBoxLayout* lk_VLayout_ = new QVBoxLayout(lk_Dialog_);
				QBoxLayout* lk_HLayout_ = new QHBoxLayout(lk_Dialog_);
				k_FileList* lk_ChoicesWidget_ = new k_FileList(lk_Dialog_, false);
				lk_ChoicesWidget_->setSortingEnabled(true);
				lk_ChoicesWidget_->setObjectName("choices-stock");
				lk_VLayout_->addWidget(lk_ChoicesWidget_);
				QPushButton* lk_DialogAddButton_ = new QPushButton(QIcon(":/icons/list-add.png"), "Add", lk_Dialog_);
				QPushButton* lk_DialogCancelButton_ = new QPushButton(QIcon(":/icons/dialog-cancel.png"), "Cancel", lk_Dialog_);
				connect(lk_DialogAddButton_, SIGNAL(clicked()), lk_Dialog_, SLOT(accept()));
				connect(lk_DialogCancelButton_, SIGNAL(clicked()), lk_Dialog_, SLOT(reject()));
				lk_DialogAddButton_->setObjectName("add");
				lk_DialogCancelButton_->setObjectName("cancel");
				lk_DialogAddButton_->setEnabled(false);
				lk_HLayout_->addStretch();
				lk_HLayout_->addWidget(lk_DialogCancelButton_);
				lk_HLayout_->addWidget(lk_DialogAddButton_);
				lk_VLayout_->addLayout(lk_HLayout_);
				lk_Dialog_->setLayout(lk_VLayout_);
				connect(lk_AddButton_, SIGNAL(clicked()), lk_Dialog_, SLOT(exec()));
				connect(lk_ChoicesWidget_, SIGNAL(selectionChanged(bool)), lk_DialogAddButton_, SLOT(setEnabled(bool)));
				connect(lk_Dialog_, SIGNAL(accepted()), this, SLOT(addChoiceItems()));
				connect(lk_Dialog_, SIGNAL(accepted()), this, SLOT(resetDialog()));
				
				lk_Dialog_->setProperty("key", QVariant(ls_Key));
				connect(lk_Dialog_, SIGNAL(accepted()), this, SLOT(parameterChanged()));

				connect(lk_Dialog_, SIGNAL(rejected()), this, SLOT(resetDialog()));
				connect(lk_ListWidget_, SIGNAL(remove(QList<QListWidgetItem *>)), this, SLOT(removeChoiceItems(QList<QListWidgetItem *>)));
				connect(lk_ChoicesWidget_, SIGNAL(doubleClick()), lk_Dialog_, SLOT(accept()));
				foreach (QVariant lk_Item, lk_ParametersValues[ls_Key])
				{
                    QString ls_ValueKey;
                    QString ls_Label;
                    if (lk_Item.canConvert<tk_YamlMap>())
                    {
                        ls_ValueKey = lk_Item.toMap().keys().first();
                        ls_Label = lk_Item.toMap()[ls_ValueKey].toString();
                    }
                    else
                        ls_ValueKey = ls_Label = lk_Item.toString();
					k_CiListWidgetItem * lk_Item_ = new k_CiListWidgetItem(ls_Label, lk_ChoicesWidget_);
					lk_Item_->setData(Qt::UserRole, QVariant(ls_ValueKey));
				}
			}
			mk_ParameterMultiChoiceWidgets[ls_Key] = lk_CheckWidgets;
		}
		QWidget* lk_Label_ = NULL;
		if (lb_AddLabel)
		{
			QString ls_Label = lk_Parameter["label"];
			if (ls_Key.startsWith("output") && !lk_Parameter["filename"].isEmpty())
				ls_Label = "Write " + ls_Label + QString(" (%1)").arg(lk_Parameter["filename"]);
			if (!lb_WidgetFirst)
				ls_Label += ":";
				
			if (mb_ProfileMode)
			{
				lk_Label_ = new QCheckBox(ls_Label, lk_Container_);
				lk_Widget_->setEnabled(false);
				dynamic_cast<QCheckBox*>(lk_Label_)->setCheckState(Qt::Unchecked);
				lk_Label_->setProperty("key", QVariant(ls_Key));
				connect(lk_Label_, SIGNAL(stateChanged(int)), this, SLOT(toggleParameter(int)));
				connect(lk_Label_, SIGNAL(stateChanged(int)), this, SLOT(parameterChanged()));
			}
			else
				lk_Label_ = new k_ClickableLabel(ls_Label, lk_Container_);
				
			mk_WidgetLabelsOrCheckBoxes[ls_Key] = lk_Label_;
				
			if (!ls_Description.isEmpty() && !ls_Key.startsWith("output"))
				lk_Label_->setToolTip(ls_Description);
		}
		if (lk_Widget_ != NULL)
		{
			if (!ls_Description.isEmpty() && !ls_Key.startsWith("output"))
				lk_Widget_->setToolTip(ls_Description);
			
			mk_ParameterDisplayWidgets[ls_Key] = lk_Widget_;
			if (lk_ValueWidget_ == NULL)
				lk_ValueWidget_ = lk_Widget_;
			mk_ParameterValueWidgets[ls_Key] = lk_ValueWidget_;
			if (!ls_Key.startsWith("output"))
				mb_HasParameters = true;
		}
		if (lb_WidgetFirst)
		{
			lk_Layout_->setSpacing(0);
			if (lk_Widget_)
				lk_Layout_->addWidget(lk_Widget_);
			if (lb_AddLabel && lk_Label_)
				lk_Layout_->addWidget(lk_Label_);
			lk_Layout_->addStretch();
			connect(lk_Label_, SIGNAL(clicked()), dynamic_cast<QCheckBox*>(lk_Widget_), SLOT(toggle()));
		}
		else
		{
			if (lb_AddLabel && lk_Label_)
				lk_Layout_->addWidget(lk_Label_);
			if (lk_Widget_)
				lk_Layout_->addWidget(lk_Widget_);
		}
	}
	lk_ParameterLayout_->addStretch();
	toggleUi();
}


void k_Script::toggleParameter(int ai_State)
{
	QObject* lk_Sender_ = sender();
	QString ls_Key = lk_Sender_->property("key").toString();
	mk_ParameterDisplayWidgets[ls_Key]->setEnabled(ai_State == Qt::Checked);
}


void k_Script::parameterChanged()
{
	QObject* lk_Sender_ = sender();
	QString ls_Key = lk_Sender_->property("key").toString();
	parameterChangedWithKey(ls_Key);
}


void k_Script::parameterChangedWithKey(QString as_Key)
{
	bool lb_Default = (mk_DefaultConfiguration[as_Key] == this->parameterValue(as_Key));
	QWidget* lk_Widget_ = mk_WidgetLabelsOrCheckBoxes[as_Key];
	QLabel* lk_Label_ = dynamic_cast<QLabel*>(lk_Widget_);
	QCheckBox* lk_CheckBox_ = dynamic_cast<QCheckBox*>(lk_Widget_);
	QString ls_String;
	
	if (lk_Label_)
		ls_String = lk_Label_->text();
	else if (lk_CheckBox_)
		ls_String = lk_CheckBox_->text();
		
	if (!mb_ProfileMode)
	{
		if (ls_String.startsWith("<u>"))
		{
			ls_String.replace("<u>", "");
			ls_String.replace("</u>", "");
		}
		if (!lb_Default)
			ls_String = "<u>" + ls_String + "</u>";
	}
	
	if (lk_Label_)
		lk_Label_->setText(ls_String);
	else if (lk_CheckBox_)
		lk_CheckBox_->setText(ls_String);
		
	QString ls_Group = mk_ParameterDefs[as_Key]["group"];
	int li_Count = 0;
	
	if (!mb_ProfileMode)
	{
		foreach (QString ls_Key, mk_GroupParameters[ls_Group])
		{
			if (this->parameterValue(ls_Key) != this->parameterDefault(ls_Key))
				li_Count += 1;
		}
	}
	else
	{
		foreach (QString ls_Key, mk_GroupParameters[ls_Group])
		{
			QWidget* lk_Widget_ = mk_WidgetLabelsOrCheckBoxes[ls_Key];
			QCheckBox* lk_CheckBox_ = dynamic_cast<QCheckBox*>(lk_Widget_);
			if (lk_CheckBox_ != NULL)
			{
				if (lk_CheckBox_->checkState() == Qt::Checked)
					li_Count += 1;
			}
		}
	}
		
	if (li_Count == 0)
		mk_FoldedHeaders[mk_ParameterDefs[as_Key]["group"]]->setSuffix("");
	else if (li_Count == 1)
		mk_FoldedHeaders[mk_ParameterDefs[as_Key]["group"]]->setSuffix(mb_ProfileMode? QString("(1 setting)"): QString("(1 change)"));
	else
		mk_FoldedHeaders[mk_ParameterDefs[as_Key]["group"]]->setSuffix(mb_ProfileMode? QString("(%1 settings)").arg(li_Count): QString("(%1 changes)").arg(li_Count));

	if (mb_ProfileMode)
	{
		QString ls_Description = this->profileDescription();
		emit profileDescriptionChanged(ls_Description);
	}
	emit parameterChanged(as_Key);
}


void k_Script::adjustDependentParameters()
{
	foreach (QString ls_Key, mk_DependentParameters)
	{
		// do something with the dependent parameter, like, enable or disable
		// it according to its dependencies
	}
}
