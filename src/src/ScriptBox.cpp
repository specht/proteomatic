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

#include "ScriptBox.h"
#include "Desktop.h"
#include "DesktopBoxFactory.h"
#include "HintLineEdit.h"
#include "OutFileListBox.h"
#include "ScriptFactory.h"
#include "Tango.h"
#include "UnclickableLabel.h"


k_ScriptBox::k_ScriptBox(const QString& as_ScriptUri, k_Desktop* ak_Parent_, k_Proteomatic& ak_Proteomatic)
	: k_DesktopBox(ak_Parent_, ak_Proteomatic, false)
	, mk_pScript(k_ScriptFactory::makeScript(as_ScriptUri, ak_Proteomatic, false, false))
{
	connect(this, SIGNAL(boxConnected(IDesktopBox*, bool)), this, SLOT(handleBoxConnected(IDesktopBox*, bool)));
	connect(this, SIGNAL(boxDisconnected(IDesktopBox*, bool)), this, SLOT(handleBoxDisconnected(IDesktopBox*, bool)));
	setupLayout();
}


k_ScriptBox::~k_ScriptBox()
{
	// remove mk_pScript's parameter widget from parameter widget proxy
	mk_pScript->parameterWidget()->setParent(NULL);
	foreach (QString ls_Key, mk_OutputFileBoxes.keys())
		mk_Desktop_->removeBox(mk_OutputFileBoxes[ls_Key]);
}


IScript* k_ScriptBox::script()
{
	return mk_pScript.get_Pointer();
}

	
void k_ScriptBox::outFileCheckboxClicked()
{
	QCheckBox* lk_CheckBox_ = dynamic_cast<QCheckBox*>(sender());
	QString ls_Key = lk_CheckBox_->property("key").toString();
	if (lk_CheckBox_->checkState() == Qt::Checked)
	{
		IDesktopBox* lk_Box_ = 
		k_DesktopBoxFactory::makeOutFileListBox(
			mk_Desktop_, mk_Proteomatic, 
			mk_pScript->outputFileDetails(ls_Key)["label"],
			mk_pScript->outputFileDetails(ls_Key)["filename"]);
		dynamic_cast<QObject*>(lk_Box_)->setProperty("key", ls_Key);
		connect(dynamic_cast<QObject*>(lk_Box_), SIGNAL(deleted()), this, SLOT(outFileBoxDeleted()));
		mk_OutputFileBoxes[ls_Key] = lk_Box_;
		mk_Desktop_->addBox(lk_Box_);
		this->connectOutgoingBox(lk_Box_);
		dynamic_cast<k_OutFileListBox*>(lk_Box_)->setListMode(batchMode());
	}
	else
	{
		if (mk_OutputFileBoxes.contains(ls_Key))
		{
			mk_Desktop_->removeBox(mk_OutputFileBoxes[ls_Key]);
			mk_OutputFileBoxes.remove(ls_Key);
		}
	}
}


void k_ScriptBox::outFileBoxDeleted()
{
	QString ls_Key = sender()->property("key").toString();
	if (mk_Checkboxes.contains(ls_Key))
	{
		// remove this output file box so that outFileCheckboxClicked won't
		// try to delete the box a second time
		IDesktopBox* lk_Box_ = mk_OutputFileBoxes[ls_Key];
		mk_OutputFileBoxes.remove(ls_Key);
		mk_Checkboxes[ls_Key]->setChecked(false);
	}
}


void k_ScriptBox::handleBoxConnected(IDesktopBox* ak_Other_, bool ab_Incoming)
{
	if (ab_Incoming)
	{
		// enter batch mode if source box is in batch mode...
		if (ak_Other_->batchMode())
			setBatchMode(true);
		// if this box is in batch mode, put all output boxes in list mode
		foreach (IDesktopBox* lk_Box_, mk_OutputFileBoxes.values())
			dynamic_cast<k_OutFileListBox*>(lk_Box_)->setListMode(batchMode());
		// ...and watch future changes
		connect(dynamic_cast<QObject*>(ak_Other_), SIGNAL(batchModeChanged(bool)), this, SLOT(inputFileBoxBatchModeChanged(bool)));
		updateBatchOutputFilenames();
		connect(dynamic_cast<QObject*>(ak_Other_), SIGNAL(filenamesChanged()), this, SLOT(updateBatchOutputFilenames()));
	}
}


void k_ScriptBox::handleBoxDisconnected(IDesktopBox* ak_Other_, bool ab_Incoming)
{
	/*
	if (!ab_Incoming)
	{
		k_DesktopBox* lk_Box_ = dynamic_cast<k_DesktopBox*>(ak_Other_);
		if (lk_Box_)
		{
			QString ls_Key = lk_Box_->property("key").toString();
			if (mk_Checkboxes.contains(ls_Key))
			{
				delete mk_OutputFileBoxes[ls_Key];
				// remove this output file box so that outFileCheckboxClicked won't
				// try to delete the box a second time
				mk_OutputFileBoxes.remove(ls_Key);
				mk_Checkboxes[ls_Key]->setChecked(false);
			}
		}
	}
	*/
	// TODO: update batch mode and output file boxes list mode
}


void k_ScriptBox::inputFileBoxBatchModeChanged(bool ab_Enabled)
{
	// batch mode this box if at least one incoming box is in batch mode
	bool lb_BatchMode = false;
	foreach (IDesktopBox* lk_Box_, mk_ConnectedIncomingBoxes)
	{
		if (lk_Box_->batchMode())
		{
			lb_BatchMode = true;
			break;
		}
	}
	setBatchMode(lb_BatchMode);
	// if this box is in batch mode, put all output boxes in list mode
	foreach (IDesktopBox* lk_Box_, mk_OutputFileBoxes.values())
		dynamic_cast<k_OutFileListBox*>(lk_Box_)->setListMode(lb_BatchMode);
}


void k_ScriptBox::updateBatchOutputFilenames()
{
	if (batchMode())
	{
		foreach (QString ls_Key, mk_OutputFileBoxes.keys())
		{
			k_OutFileListBox* lk_OutBox_ = dynamic_cast<k_OutFileListBox*>(mk_OutputFileBoxes[ls_Key]);
			QStringList lk_Filenames;
			// combine files from all batch mode input file list boxes
			foreach (IDesktopBox* lk_Box_, mk_ConnectedIncomingBoxes)
			{
				if (lk_Box_->batchMode())
				{
					IFileBox* lk_SourceBox_ = dynamic_cast<IFileBox*>(lk_Box_);
					foreach (QString ls_Path, lk_SourceBox_->filenames())
					{
						QString ls_OutFilename = lk_OutBox_->filename();
						QString ls_OutPath;
						if (ls_OutFilename.contains("."))
						{
							QStringList lk_OutFilename = ls_OutFilename.split(".");
							QString ls_Base = lk_OutFilename.takeFirst();
							ls_OutPath = ls_Base + "-" + QFileInfo(ls_Path).baseName() + lk_OutFilename.join(".");
						}
						else
							ls_OutPath = ls_OutFilename + "-" + QFileInfo(ls_Path).baseName(); 
						
						lk_Filenames.append(ls_OutPath);
					}
				}
			}
			lk_OutBox_->setFilenames(lk_Filenames);
		}
	}
	else
	{
		// use original filename
	}
}


void k_ScriptBox::setupLayout()
{
	QBoxLayout* lk_VLayout_;
	QBoxLayout* lk_HLayout_;
	
	// build parameter proxy widget
	
	mk_pParameterProxyWidget = RefPtr<QWidget>(new QDialog());
	
	mk_pParameterProxyWidget->resize(500, 600);
	mk_pParameterProxyWidget->setWindowTitle(mk_pScript->title());
	mk_pParameterProxyWidget->setWindowIcon(QIcon(":icons/proteomatic.png"));
	
	lk_VLayout_ = new QVBoxLayout(mk_pParameterProxyWidget.get_Pointer());
	lk_VLayout_->setContentsMargins(0, 0, 0, 0);
	lk_VLayout_->setSpacing(0);
	
	QToolBar* lk_ToolBar_ = new QToolBar(mk_pParameterProxyWidget.get_Pointer());
	lk_ToolBar_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

	lk_ToolBar_->addAction(QIcon(":/icons/document-properties.png"), "Profiles", this, SLOT(showProfileManager()));
	lk_ToolBar_->addAction(QIcon(":/icons/edit-clear.png"), "Reset", dynamic_cast<QObject*>(mk_pScript.get_Pointer()), SLOT(reset()));
	QWidget* lk_StretchLabel_ = new QWidget(mk_pParameterProxyWidget.get_Pointer());
	lk_StretchLabel_->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum));
	lk_StretchLabel_->setContentsMargins(0, 0, 0, 0);
	lk_ToolBar_->addWidget(lk_StretchLabel_);
	lk_ToolBar_->addAction(QIcon(":/icons/dialog-ok.png"), "Close", mk_pParameterProxyWidget.get_Pointer(), SLOT(accept()));
	
	lk_VLayout_->addWidget(lk_ToolBar_);

	QScrollArea* lk_ScrollArea_ = new QScrollArea();
	lk_ScrollArea_->setWidget(mk_pScript->parameterWidget());
	lk_ScrollArea_->setWidgetResizable(true);
	lk_VLayout_->addWidget(lk_ScrollArea_);

	// now the script box ...
	
	lk_VLayout_ = new QVBoxLayout(this);
	
	// script title
	QLabel* lk_ScriptTitle_ = new k_UnclickableLabel("<b>" + mk_pScript->title() + "</b>", this);
	lk_VLayout_->addWidget(lk_ScriptTitle_);

	// horizontal rule
	QFrame* lk_Frame_ = new QFrame(this);
	lk_Frame_->setFrameStyle(QFrame::HLine | QFrame::Plain);
	lk_Frame_->setLineWidth(1);
	lk_Frame_->setStyleSheet("color: " + QString(TANGO_ALUMINIUM_3) + ";");
	lk_VLayout_->addWidget(lk_Frame_);
	
	// buttons
	lk_HLayout_ = new QHBoxLayout();
	lk_VLayout_->addLayout(lk_HLayout_);
	
	QToolButton* lk_ParametersToolButton_ = new QToolButton();
	lk_ParametersToolButton_->setIcon(QIcon(":/icons/preferences-system.png"));
	lk_HLayout_->addWidget(lk_ParametersToolButton_);
	connect(lk_ParametersToolButton_, SIGNAL(clicked()), mk_pParameterProxyWidget.get_Pointer(), SLOT(show()));
	lk_HLayout_->addStretch();
	
	mk_Prefix.setHint("output file prefix");
	lk_VLayout_->addWidget(&mk_Prefix);
	
	/*
	// horizontal rule
	lk_Frame_ = new QFrame(this);
	lk_Frame_->setFrameStyle(QFrame::HLine | QFrame::Plain);
	lk_Frame_->setLineWidth(1);
	lk_Frame_->setStyleSheet("color: " + QString(TANGO_ALUMINIUM_3) + ";");
	lk_VLayout_->addWidget(lk_Frame_);
	*/
	
	// output file checkboxes
	foreach (QString ls_Key, mk_pScript->outputFileKeys())
	{
		QHash<QString, QString> lk_OutputFileDetails = mk_pScript->outputFileDetails(ls_Key);
		QCheckBox* lk_CheckBox_ = new QCheckBox("Write " + lk_OutputFileDetails["label"]);
		lk_CheckBox_->setProperty("key", ls_Key);
		connect(lk_CheckBox_, SIGNAL(toggled(bool)), this, SLOT(outFileCheckboxClicked()));
		lk_VLayout_->addWidget(lk_CheckBox_);
		mk_Checkboxes[ls_Key] = lk_CheckBox_;
		if (lk_OutputFileDetails["default"] == "yes" || lk_OutputFileDetails["default"] == "true")
			lk_CheckBox_->setChecked(true);
	}
}
