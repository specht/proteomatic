#include "Script.h"
#include "FoldedHeader.h"
#include "FileList.h"
#include "CiListWidgetItem.h"
#include "Proteomatic.h"
#include "RefPtr.h"
#include "RubyWindow.h"
#include <limits.h>
#include <float.h>


k_Script::k_Script(r_ScriptType::Enumeration ae_Type, QString as_ScriptUri, k_Proteomatic& ak_Proteomatic, bool ab_IncludeOutputFiles, bool ab_ProfileMode)
	: me_Type(ae_Type)
	, ms_ScriptUri(as_ScriptUri)
	, ms_Prefix("")
	, mk_Proteomatic(ak_Proteomatic)
	, mb_IsGood(false)
	, ms_Title(ak_Proteomatic.scriptInfo(as_ScriptUri, "title"))
	, ms_Description(ak_Proteomatic.scriptInfo(as_ScriptUri, "description"))
	, mk_OutputDirectory_(NULL)
	, mk_ClearOutputDirectory_(NULL)
	, mb_HasParameters(false)
{
}


k_Script::~k_Script()
{
}


bool k_Script::isGood()
{
	return mb_IsGood;
}


bool k_Script::hasParameters()
{
	return mb_HasParameters;
}


r_ScriptType::Enumeration k_Script::type() const
{
	return me_Type;
}


k_SizeWatchWidget* k_Script::parameterWidget()
{
	return mk_pParameterWidget.get_Pointer();
}


QString k_Script::uri()
{
	return ms_ScriptUri;
}


QString k_Script::title()
{
	return ms_Title;
}


QString k_Script::description()
{
	return ms_Description;
}


void k_Script::reset()
{
	setConfiguration(ms_DefaultConfiguration);
}


void k_Script::setPrefix(QString as_Prefix)
{
	ms_Prefix = as_Prefix;
}


QString k_Script::prefix()
{
	return ms_Prefix;
}


QList<QString> k_Script::outFiles()
{
	return mk_OutFileDetails.keys();
}


QHash<QString, QString> k_Script::outFileDetails(QString as_Key)
{
	return mk_OutFileDetails[as_Key];
}


QStringList k_Script::commandLineArguments()
{
	QStringList lk_Result;
	foreach (QString ls_Key, mk_ParameterValueWidgets.keys())
	{
		lk_Result.push_back("-" + ls_Key);
		lk_Result.push_back(getParameterValue(ls_Key));
	}
	return lk_Result;
}


QString k_Script::getParameterValue(QString as_Key)
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

void k_Script::setParameterValue(QString as_Key, QString as_Value)
{
	if (!mk_ParameterValueWidgets.contains(as_Key))
		return;
	QWidget* lk_Widget_ = mk_ParameterValueWidgets[as_Key];
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


QString k_Script::getConfiguration()
{
	QString ls_Result;
	QTextStream lk_Stream(&ls_Result);

	lk_Stream << QString("Proteomatic Parameters, version %1\n").arg(mk_Proteomatic.version());
	lk_Stream << ms_Title << "\n";

	foreach (QString ls_Key, mk_ParameterValueWidgets.keys())
		lk_Stream << ls_Key << ": " << getParameterValue(ls_Key) << "\n";

	lk_Stream.flush();
	return ls_Result;
}


void k_Script::setConfiguration(QString as_Configuration)
{
	QTextStream lk_Stream(&as_Configuration);

	QString ls_Line = lk_Stream.readLine();
	if (!ls_Line.startsWith("Proteomatic Parameters, version"))
	{
		mk_Proteomatic.showMessageBox("Error loading parameters", "The file you specified is not a valid Proteomatic parameters file.",
			":/icons/stop.png", QMessageBox::Ok, QMessageBox::Ok, QMessageBox::Ok);
		return;
	}
	ls_Line = lk_Stream.readLine();
	if (ls_Line != ms_Title)
	{
		mk_Proteomatic.showMessageBox("Error loading parameters", "The parameter file you specified is not meant for the currently loaded script.",
			":/icons/stop.png", QMessageBox::Ok, QMessageBox::Ok, QMessageBox::Ok);
		return;
	}

	int li_LineCount = 2;
	QString ls_Error = "";
	QString ls_Warnings = "";
	while (!lk_Stream.atEnd())
	{
		QString ls_Line = lk_Stream.readLine();
		li_LineCount += 1;
		if (!ls_Line.contains(QChar(':')))
		{
			ls_Error = QString("There is an error on line %1.").arg(li_LineCount);
			break;
		}

		QString ls_Key = ls_Line.left(ls_Line.indexOf(QChar(':'))).trimmed();
		QString ls_Value = ls_Line.right(ls_Line.length() - ls_Key.length() - 1).trimmed();

		setParameterValue(ls_Key, ls_Value);
	}

	if (ls_Error != "")
	{
		mk_Proteomatic.showMessageBox("Error loading parameters", ls_Error,
			":/icons/stop.png", QMessageBox::Ok, QMessageBox::Ok, QMessageBox::Ok);
		return;
	}
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

	for (int i = 0; i < lk_Source_->count(); ++i)
	{
		QListWidgetItem* lk_Item_ = lk_Source_->item(i);
		if (ak_Choices.contains(lk_Item_->data(Qt::UserRole).toString()))
		{
			k_CiListWidgetItem* lk_NewItem_ = new k_CiListWidgetItem(lk_Item_->text(), lk_Target_);
			lk_NewItem_->setData(Qt::UserRole, lk_Item_->data(Qt::UserRole));
			delete lk_Item_;
		}
	}

}


void k_Script::toggleGroup()
{
	k_FoldedHeader* lk_Header_ = dynamic_cast<k_FoldedHeader*>(sender());
	if (lk_Header_ != NULL)
	{
		if (!lk_Header_->buddyVisible())
		{
			/*
			// hide all other folded headers
			foreach (k_FoldedHeader* lk_Other_, mk_FoldedHeaders)
			{
				if (lk_Other_ != lk_Header_)
					lk_Other_->hideBuddy();
			}
			*/
		}
		lk_Header_->toggleBuddy();
	}
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
}


void k_Script::clearOutputDirectoryButtonClicked()
{
	if (mk_OutputDirectory_ == NULL)
		return;
	
	mk_OutputDirectory_->clear();
}


void k_Script::setOutputDirectoryButtonClicked()
{
	if (mk_OutputDirectory_ == NULL)
		return;
	
	QString ls_Path = QFileDialog::getExistingDirectory(mk_pParameterWidget.get_Pointer(), tr("Select output directory"), QDir::homePath());
	if (ls_Path.length() > 0)
		mk_OutputDirectory_->setText(ls_Path);
}


void k_Script::toggleUi()
{
	if (mk_ClearOutputDirectory_ != NULL)
		mk_ClearOutputDirectory_->setEnabled(!mk_OutputDirectory_->text().isEmpty());
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


void k_Script::createParameterWidget(QStringList ak_Definition, bool ab_IncludeOutputFiles, bool ab_ProfileMode)
{
	mk_InputFileDescriptionList.clear();
	mk_OutputDirectory_ = NULL;
	mk_ClearOutputDirectory_ = NULL;
	mk_pParameterWidget = RefPtr<k_SizeWatchWidget>(new k_SizeWatchWidget());

	QList<QString> lk_ParametersOrder;
	QHash<QString, QHash<QString, QString> > lk_Parameters;
	QHash<QString, QList<QString> > lk_ParametersValues;

	QString ls_Parameter;
	while (!ak_Definition.empty())
	{
		ls_Parameter = ak_Definition.takeFirst().trimmed();
		QHash<QString, QString> lk_Parameter;
		QList<QString> lk_EnumValues;
		if (ls_Parameter == "!!!begin parameter")
		{
			// collect key/value pairs
			while (true)
			{
				QString ls_Key = ak_Definition.takeFirst().trimmed();
				if (ls_Key == "!!!end parameter")
					break;
				if (ls_Key == "!!!begin values")
				{
					while (true)
					{
						QString ls_Value = ak_Definition.takeFirst().trimmed();
						if (ls_Value == "!!!end values")
							break;
						lk_EnumValues.push_back(ls_Value);
					}
				}
				else
				{
					QString ls_Value = ak_Definition.takeFirst().trimmed();
					lk_Parameter[ls_Key] = ls_Value;
				}
			}
		}
		if (ls_Parameter == "!!!begin input")
		{
			while (true)
			{
				QString ls_Key = ak_Definition.takeFirst().trimmed();
				if (ls_Key == "!!!end input")
					break;
				mk_InputFileDescriptionList.push_back(ls_Key);
			}
		}
		if (lk_Parameter["key"].length() > 0)
		{
			lk_Parameters[lk_Parameter["key"]] = lk_Parameter;
			lk_ParametersValues[lk_Parameter["key"]] = lk_EnumValues;
			lk_ParametersOrder.push_back(lk_Parameter["key"]);
		}
	}

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
	if (!ab_ProfileMode)
	{
		QLabel* lk_Label_ = new QLabel("<b>" + title() + "</b>", lk_InternalWidget_);
		lk_ParameterLayout_->addWidget(lk_Label_);
		if (!description().isEmpty())
		{
			lk_Label_ = new QLabel("<i></i>" + description(), lk_InternalWidget_);
			lk_Label_->setWordWrap(true);
			lk_ParameterLayout_->addWidget(lk_Label_);
		}
		if (!mk_InputFileDescriptionList.empty())
		{
			QString ls_List;
			foreach (QString ls_Item, mk_InputFileDescriptionList)
				ls_List += "<li>" + ls_Item + "</li>";
			lk_Label_ = new QLabel("<i></i>Input files:<ul>" + ls_List + "</ul>", lk_InternalWidget_);
			lk_Label_->setWordWrap(true);
			lk_ParameterLayout_->addWidget(lk_Label_);
		}
	}
	QFrame* lk_Frame_ = new QFrame(lk_InternalWidget_);
	lk_Frame_->setFrameStyle(QFrame::HLine | QFrame::Sunken);
	lk_ParameterLayout_->addWidget(lk_Frame_);
	
	//lk_ParameterLayout_->setContentsMargins(0, 0, 0, 0);
	//mk_UpperLayout_->insertWidget(3, lk_InternalWidget_);

	//lk_InternalWidget_->show();

	foreach (QString ls_Key, lk_ParametersOrder)
	{
		if (ls_Key.startsWith("[output]") && ls_Key != "[output]prefix" && ls_Key != "[output]directory")
			mk_OutFileDetails[ls_Key] = lk_Parameters[ls_Key];

		if (!ab_IncludeOutputFiles && ls_Key.startsWith("[output]"))
			continue;

		QString ls_Group = lk_Parameters[ls_Key]["group"];
		if (lk_Containers[ls_Group] == NULL)
		{
			QWidget* lk_Container_ = new QWidget(lk_InternalWidget_);
			k_FoldedHeader* lk_Header_ = new k_FoldedHeader("<b>" + ls_Group + "</b>", lk_Container_, lk_InternalWidget_);
			mk_FoldedHeaders.append(lk_Header_);
			connect(lk_Header_, SIGNAL(clicked()), this, SLOT(toggleGroup()));
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
		if (!ab_IncludeOutputFiles && ls_Key.startsWith("[output]"))
			continue;

		QHash<QString, QString> lk_Parameter = lk_Parameters[ls_Key];
		QString ls_Group = lk_Parameter["group"];
		QWidget* lk_Container_ = lk_Containers[ls_Group];

		QBoxLayout* lk_Layout_ = new QBoxLayout(QBoxLayout::LeftToRight);
		lk_Layout_->setContentsMargins(0, 0, 0, 0);
		lk_Layout_->setSpacing(5);
		int li_ColSpan = 1;
		if (lk_Parameter.contains("colspan"))
			li_ColSpan = QVariant(lk_Parameter["colspan"]).toInt();
		if (li_ColSpan > 1)
		{
			if (lk_GroupBoxX[ls_Group] != 0)
			{
				lk_GroupBoxX[ls_Group] = 0;
				lk_GroupBoxY[ls_Group] += 1;
			}
		}
		lk_GridLayouts[ls_Group]->addLayout(lk_Layout_, lk_GroupBoxY[ls_Group], lk_GroupBoxX[ls_Group], 1, li_ColSpan);
		if (true || li_ColSpan > 1)
		{
			lk_GroupBoxX[ls_Group] = 0;
			lk_GroupBoxY[ls_Group] += 1;
		}
		else
		{
			lk_GroupBoxX[ls_Group] += 1;
			if (lk_GroupBoxX[ls_Group] > 1)
			{
				lk_GroupBoxX[ls_Group] = 0;
				lk_GroupBoxY[ls_Group] += 1;
			}
		}

		bool lb_AddLabel = true;

		bool lb_Ok;

		// use lk_Widget_ for insertion into layout and as value unless
		// lk_ValueWidget_ != NULL, then use this as the value!!
		QWidget* lk_Widget_ = NULL;
		QWidget* lk_ValueWidget_ = NULL;
		
		QString ls_Type = lk_Parameter["type"];
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
		} 
		else if (ls_Type == "string")
		{
			QLineEdit* lk_LineEdit_ = new QLineEdit(lk_Container_);
			lk_Widget_ = lk_LineEdit_;
			lk_LineEdit_->setText(lk_Parameter["default"]);
			if (lk_Parameter["key"] == "[output]directory")
			{
				mk_OutputDirectory_ = lk_LineEdit_;
				QWidget* lk_SubContainer_ = new QWidget(lk_Container_);
				lk_SubContainer_->setContentsMargins(0, 0, 0, 0);
				QBoxLayout* lk_GroupBoxLayout_ = new QHBoxLayout(lk_SubContainer_);
				lk_GroupBoxLayout_->setMargin(0);
				lk_GroupBoxLayout_->addWidget(lk_LineEdit_);
				QToolButton* lk_ClearOutputDirectoryButton_ = new QToolButton(lk_SubContainer_);
				lk_ClearOutputDirectoryButton_->setIcon(QIcon(":/icons/dialog-cancel.png"));
				mk_ClearOutputDirectory_ = lk_ClearOutputDirectoryButton_;
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
		}
		else if (ls_Type == "flag")
		{
			QString ls_Label = lk_Parameter["label"];
			if (ls_Key.startsWith("[output]"))
				ls_Label += QString(" (%1)").arg(lk_Parameter["filename"]);
			QCheckBox* lk_CheckBox_ = new QCheckBox(ls_Label, lk_Container_);
			lk_Widget_ = lk_CheckBox_;
			lk_CheckBox_->setChecked((lk_Parameter["default"] == "true") || (lk_Parameter["default"] == "yes"));
			if (lk_Parameter.contains("force"))
				lk_CheckBox_->setEnabled(false);
			lb_AddLabel = false;
		}
		else if (ls_Type == "enum")
		{
			QComboBox* lk_ComboBox_ = new QComboBox(lk_Container_);
			lk_Widget_ = lk_ComboBox_;
			int li_Index = 0;
			int li_DefaultIndex = 0;
			foreach (QString ls_Item, lk_ParametersValues[ls_Key])
			{
				QString ls_Key = ls_Item;
				QString ls_Label = ls_Item;
				if (ls_Item.contains(QChar(':')))
				{
					QStringList lk_Item = ls_Item.split(":");
					ls_Key = lk_Item[0].trimmed();
					ls_Label = lk_Item[1].trimmed();
				}
				lk_ComboBox_->addItem(ls_Label, QVariant(ls_Key));
				if (ls_Key == lk_Parameter["default"])
					li_DefaultIndex = li_Index;
				li_Index += 1;
			}
			lk_ComboBox_->setCurrentIndex(li_DefaultIndex);
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
				foreach (QString ls_Item, lk_ParametersValues[ls_Key])
				{
					QCheckBox* lk_CheckBox_ = new QCheckBox(lk_Widget_);
					QString ls_Key = ls_Item;
					QString ls_Label = ls_Item;
					if (ls_Item.contains(QChar(':')))
					{
						QStringList lk_Item = ls_Item.split(":");
						ls_Key = lk_Item[0].trimmed();
						ls_Label = lk_Item[1].trimmed();
					}
					lk_CheckBox_->setText(ls_Label);
					lk_WidgetLayout_->addWidget(lk_CheckBox_);
					lk_CheckBox_->setChecked(lk_DefaultChecks.contains(ls_Key));
					lk_CheckWidgets.append(lk_CheckBox_);
					lk_CheckBox_->setProperty("ProteomaticValue", QVariant(ls_Key));
				}
				lk_WidgetLayout_->addStretch();
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
				connect(lk_Dialog_, SIGNAL(rejected()), this, SLOT(resetDialog()));
				connect(lk_ListWidget_, SIGNAL(remove(QList<QListWidgetItem *>)), this, SLOT(removeChoiceItems(QList<QListWidgetItem *>)));
				connect(lk_ChoicesWidget_, SIGNAL(doubleClick()), lk_Dialog_, SLOT(accept()));
				foreach (QString ls_Item, lk_ParametersValues[ls_Key])
				{
					QString ls_Key = ls_Item;
					QString ls_Label = ls_Item;
					if (ls_Item.contains(QChar(':')))
					{
						QStringList lk_Item = ls_Item.split(":");
						ls_Key = lk_Item[0].trimmed();
						ls_Label = lk_Item[1].trimmed();
					}
					k_CiListWidgetItem * lk_Item_ = new k_CiListWidgetItem(ls_Label, lk_ChoicesWidget_);
					lk_Item_->setData(Qt::UserRole, QVariant(ls_Key));
				}

			}
			mk_ParameterMultiChoiceWidgets[ls_Key] = lk_CheckWidgets;
		}
		if (lb_AddLabel)
		{
			QWidget* lk_Label_;
			if (ab_ProfileMode)
			{
				lk_Label_ = new QCheckBox(lk_Parameter["label"] + ":", lk_Container_);
				lk_Widget_->setEnabled(false);
				dynamic_cast<QCheckBox*>(lk_Label_)->setCheckState(Qt::Unchecked);
				connect(lk_Label_, SIGNAL(stateChanged(int)), this, SLOT(toggleParameter(int)));
				lk_Label_->setProperty("key", QVariant(ls_Key));
			}
			else
				lk_Label_ = new QLabel(lk_Parameter["label"] + ":", lk_Container_);
				
			//QCheckBox* 
			if (lk_Parameter.contains("description"))
				lk_Label_->setToolTip(lk_Parameter["description"]);
			lk_Layout_->addWidget(lk_Label_);
		}
		if (lk_Widget_ != NULL)
		{
			if (lk_Parameter.contains("description"))
				lk_Widget_->setToolTip(lk_Parameter["description"]);
			lk_Layout_->addWidget(lk_Widget_);
			mk_ParameterDisplayWidgets[ls_Key] = lk_Widget_;
			if (lk_ValueWidget_ == NULL)
				lk_ValueWidget_ = lk_Widget_;
			mk_ParameterValueWidgets[ls_Key] = lk_ValueWidget_;
			if (!ls_Key.startsWith("[output]"))
				mb_HasParameters = true;
		}
	}
	lk_ParameterLayout_->addStretch();
	toggleUi();
}


void k_Script::toggleParameter(int ai_State)
{
	QObject* lk_CheckBox_ = sender();
	QString ls_Key = lk_CheckBox_->property("key").toString();
	mk_ParameterDisplayWidgets[ls_Key]->setEnabled(ai_State == Qt::Checked);
}
