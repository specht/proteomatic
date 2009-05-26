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
#include "FoldedHeader.h"
#include "Desktop.h"
#include "DesktopBoxFactory.h"
#include "FileListBox.h"
#include "HintLineEdit.h"
#include "OutFileListBox.h"
#include "PipelineMainWindow.h"
#include "ScriptFactory.h"
#include "Tango.h"
#include "UnclickableLabel.h"


k_ScriptBox::k_ScriptBox(const QString& as_ScriptUri, k_Desktop* ak_Parent_, k_Proteomatic& ak_Proteomatic)
	: k_DesktopBox(ak_Parent_, ak_Proteomatic, false)
	, mk_pScript(k_ScriptFactory::makeScript(as_ScriptUri, ak_Proteomatic, false, false))
	, mk_OutputBox(this)
{
	connect(this, SIGNAL(boxConnected(IDesktopBox*, bool)), this, SLOT(handleBoxConnected(IDesktopBox*, bool)));
	connect(this, SIGNAL(boxDisconnected(IDesktopBox*, bool)), this, SLOT(handleBoxDisconnected(IDesktopBox*, bool)));
	connect(&ak_Parent_->pipelineMainWindow(), SIGNAL(outputDirectoryChanged(const QString&)), this, SLOT(updateOutputFilenames()));
	connect(dynamic_cast<QObject*>(mk_pScript.get_Pointer()), SIGNAL(scriptStarted()), this, SIGNAL(scriptStarted()));
	connect(dynamic_cast<QObject*>(mk_pScript.get_Pointer()), SIGNAL(scriptFinished(int)), this, SIGNAL(scriptFinished(int)));
	connect(dynamic_cast<QObject*>(mk_pScript.get_Pointer()), SIGNAL(readyRead()), this, SIGNAL(readyRead()));
	connect(this, SIGNAL(readyRead()), this, SLOT(readyReadSlot()));
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


bool k_ScriptBox::checkReady(QString& as_Error)
{
	return true;
}


bool k_ScriptBox::checkReadyToGo()
{
	// check whether all input files are there
	QStringList lk_InputFiles;
	
	foreach (IDesktopBox* lk_Box_, mk_ConnectedIncomingBoxes)
	{
		IFileBox* lk_FileBox_ = dynamic_cast<IFileBox*>(lk_Box_);
		if (lk_FileBox_)
		{
			foreach (QString ls_Path, lk_FileBox_->filenames())
				if (!QFileInfo(ls_Path).exists())
					return false;
		}
	}
	return true;
}


QStringList k_ScriptBox::iterationKeys()
{
	return QStringList() << "";
}


QString k_ScriptBox::outputDirectory() const
{
	if (!mk_OutputDirectory.text().isEmpty())
		return mk_OutputDirectory.text();

	return mk_Desktop_->pipelineMainWindow().outputDirectory();
}


void k_ScriptBox::outputFileActionToggled()
{
	QCheckBox* lk_CheckBox_ = dynamic_cast<QCheckBox*>(sender());
	QString ls_Key = lk_CheckBox_->property("key").toString();
	if (lk_CheckBox_->checkState() == Qt::Checked)
	{
		IDesktopBox* lk_Box_ = 
		k_DesktopBoxFactory::makeOutFileListBox(
			mk_Desktop_, mk_Proteomatic, 
			mk_pScript->outputFileDetails(ls_Key)["label"]);
		dynamic_cast<QObject*>(lk_Box_)->setProperty("key", ls_Key);
		mk_OutputFileBoxes[ls_Key] = lk_Box_;
		mk_Desktop_->addBox(lk_Box_);
		mk_Desktop_->connectBoxes(this, lk_Box_);
		dynamic_cast<k_OutFileListBox*>(lk_Box_)->setListMode(batchMode());
		updateOutputFilenames();
	}
	else
	{
		if (mk_OutputFileBoxes.contains(ls_Key))
		{
			// only delete the box if it's really there
			if (dynamic_cast<IDesktopBox*>(mk_OutputFileBoxes[ls_Key]))
				mk_Desktop_->removeBox(mk_OutputFileBoxes[ls_Key]);
			mk_OutputFileBoxes.remove(ls_Key);
		}
	}
}


void k_ScriptBox::handleBoxConnected(IDesktopBox* ak_Other_, bool ab_Incoming)
{
	if (ab_Incoming)
	{
		updateBatchMode();
		connect(dynamic_cast<QObject*>(ak_Other_), SIGNAL(batchModeChanged(bool)), this, SLOT(updateBatchMode()));
		connect(dynamic_cast<QObject*>(ak_Other_), SIGNAL(filenamesChanged()), this, SLOT(updateOutputFilenames()));
	}
}


void k_ScriptBox::handleBoxDisconnected(IDesktopBox* ak_Other_, bool ab_Incoming)
{
	if (!ab_Incoming)
	{
		// check whether it's an output file box and we have to uncheck the checkbox!
		// :TODO: speed this up, it's slow. maybe another hash?
		QString ls_Key = mk_OutputFileBoxes.key(ak_Other_);
		if (!ls_Key.isEmpty())
			mk_Checkboxes[ls_Key]->setChecked(Qt::Unchecked);
	}
	updateBatchMode();
	updateOutputFilenames();
}


void k_ScriptBox::updateBatchMode()
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
	
	updateOutputFilenames();
}


void k_ScriptBox::updateOutputFilenames()
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
						QString ls_OutFilename = mk_Prefix.text() + mk_pScript->outputFileDetails(ls_Key)["filename"];
						QString ls_Suffix = QFileInfo(ls_OutFilename).completeSuffix();
						ls_OutFilename.remove(ls_OutFilename.length() - ls_Suffix.length() - 1, ls_Suffix.length() + 1);
						QString ls_OutPath = ls_OutFilename + "-" + lk_SourceBox_->tagForFilename(ls_Path) + "." + ls_Suffix;
						ls_OutPath = ls_OutPath;
						lk_Filenames.append(QFileInfo(QDir(mk_Desktop_->pipelineMainWindow().outputDirectory()), ls_OutPath).absoluteFilePath());
					}
				}
			}
			lk_OutBox_->setFilenames(lk_Filenames);
		}
	}
	else
	{
		// use original filename
		foreach (QString ls_Key, mk_OutputFileBoxes.keys())
		{
			k_OutFileListBox* lk_OutBox_ = dynamic_cast<k_OutFileListBox*>(mk_OutputFileBoxes[ls_Key]);
			QStringList lk_Filenames;
			lk_Filenames.append(outputDirectory() + "/" + mk_Prefix.text() + mk_pScript->outputFileDetails(ls_Key)["filename"]);
			lk_OutBox_->setFilenames(lk_Filenames);
		}
	}
}


void k_ScriptBox::proposePrefixButtonClicked()
{
	if (mk_pScript->location() == r_ScriptLocation::Local)
	{
		// collect all input files
		QStringList lk_InputFiles;
		foreach (IDesktopBox* lk_Box_, mk_ConnectedIncomingBoxes)
		{
			IFileBox* lk_FileBox_ = dynamic_cast<IFileBox*>(lk_Box_);
			if (lk_FileBox_)
			{
				foreach (QString ls_Path, lk_FileBox_->filenames())
				{
					QString ls_Group = mk_pScript->inputGroupForFilename(ls_Path);
					// TODO: only append files that are in the correct group!!
					lk_InputFiles.append(ls_Path);
				}
			}
		}
		QHash<QString, QString> lk_Tags;
		QString ls_CommonPrefix;
		mk_Desktop_->createFilenameTags(lk_InputFiles, lk_Tags, ls_CommonPrefix);
		if ((!ls_CommonPrefix.isEmpty()) && (ls_CommonPrefix.right(1) != "-"))
			ls_CommonPrefix += "-";
		mk_Prefix.setText(ls_CommonPrefix);
		/*
		QString ls_Result = (dynamic_cast<k_LocalScript*>(mk_pScript.get_Pointer()))->proposePrefix(lk_Arguments);
		if (ls_Result.startsWith("--proposePrefix"))
		{
			QStringList lk_Result = ls_Result.split("\n");
			mk_PrefixWidget.setText(lk_Result[1].trimmed());
		}
		else
		{
			mk_Proteomatic.showMessageBox("Propose prefix", 
				"<p>Sorry, but Proteomatic was unable to propose a prefix.</p>", 
				":/icons/emblem-important.png", QMessageBox::Ok, QMessageBox::Ok, QMessageBox::Ok);
		}
		*/
	}
}


void k_ScriptBox::start(const QString& as_IterationKey)
{
	QHash<QString, QString> lk_Parameters;

	// set output directory
	lk_Parameters["-outputDirectory"] = this->outputDirectory();

	// set output prefix
	lk_Parameters["-outputPrefix"] = mk_Prefix.text();
	
	// set output files
	foreach (QString ls_Key, mk_pScript->outputFileKeys())
		lk_Parameters["-" + ls_Key] = mk_Checkboxes[ls_Key]->checkState() == Qt::Checked ? "yes" : "no";
	
	// collect input files
	QStringList lk_InputFiles;
	
	foreach (IDesktopBox* lk_Box_, mk_ConnectedIncomingBoxes)
	{
		IFileBox* lk_FileBox_ = dynamic_cast<IFileBox*>(lk_Box_);
		if (lk_FileBox_)
		{
			lk_InputFiles += lk_FileBox_->filenames();
		}
	}

	ms_Output.clear();
	mk_pScript->start(lk_InputFiles, lk_Parameters);
}


void k_ScriptBox::readyReadSlot()
{
	addOutput(mk_pScript->readAll());
}


void k_ScriptBox::addOutput(QString as_Text)
{
	ms_Output.append(as_Text);
	mk_OutputBox.setText(ms_Output.text());
	mk_OutputBox.moveCursor(QTextCursor::End);
	mk_OutputBox.ensureCursorVisible();
}


void k_ScriptBox::showOutputBox(bool ab_Flag/* = true*/)
{
	mk_OutputBox.setVisible(ab_Flag);
}


/*
void k_ScriptBox::showPopupMenu()
{
	QToolButton* lk_ToolButton_ = dynamic_cast<QToolButton*>(sender());
	QPoint lk_Point = lk_ToolButton_->pos() + pos() + QPoint(0, lk_ToolButton_->height());
	mk_PopupMenu_->show();
	mk_PopupMenu_->raise();
	mk_PopupMenu_->exec(lk_Point);
}
*/


void k_ScriptBox::setupLayout()
{
	QBoxLayout* lk_VLayout_;
	QBoxLayout* lk_HLayout_;
	
	// build parameter proxy widget
	
	mk_pParameterProxyWidget = RefPtr<QWidget>(new QDialog());
	
	//mk_pParameterProxyWidget->resize(500, 600);
	//mk_pParameterProxyWidget->setWindowTitle(mk_pScript->title());
	//mk_pParameterProxyWidget->setWindowIcon(QIcon(":icons/proteomatic.png"));
	
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
	
/*	QToolButton* lk_ParametersToolButton_ = new QToolButton(this);
	lk_ParametersToolButton_->setIcon(QIcon(":/icons/preferences-system.png"));
	lk_HLayout_->addWidget(lk_ParametersToolButton_);*/

	//mk_Desktop_->pipelineMainWindow().tabWidget()->addTab(mk_pParameterProxyWidget.get_Pointer(), mk_pScript->title());
	//lk_DockWidget_->hide();
	//connect(lk_ParametersToolButton_, SIGNAL(clicked()), lk_DockWidget_, SLOT(show()));
	//mk_pParameterProxyWidget->setParent(&(mk_Desktop_->pipelineMainWindow()), Qt::Tool);
	//mk_pParameterProxyWidget->hide();

	/*
	mk_PopupMenu_ = new QMenu(this);
	foreach (QString ls_Key, mk_pScript->outputFileKeys())
	{
		QHash<QString, QString> lk_OutputFileDetails = mk_pScript->outputFileDetails(ls_Key);
		QString ls_Label = lk_OutputFileDetails["label"];
		QAction* lk_Action_ = mk_PopupMenu_->addAction(QIcon(":icons/text-x-generic.png"), "Write " + ls_Label);
		lk_Action_->setProperty("key", ls_Key);
		lk_Action_->setCheckable(true);
		//connect(lk_Action_, SIGNAL(toggled(bool)), this, SLOT(outputFileActionToggled()));
		if (lk_OutputFileDetails["default"] == "yes" || lk_OutputFileDetails["default"] == "true")
			lk_Action_->setChecked(true);
	}
	mk_PopupMenu_->addSeparator();
	mk_PopupMenu_->addAction(QIcon(":icons/folder.png"), "Set output directory");
	QToolButton* lk_OutputFilesButton_ = new QToolButton(this);
	lk_OutputFilesButton_->setText("Output files");
	//lk_OutputFilesButton_->setPopupMode(QToolButton::InstantPopup);
	//lk_OutputFilesButton_->setMenu(lk_OutputFilesMenu_);
	lk_OutputFilesButton_->setIcon(QIcon(":icons/folder.png"));
	lk_OutputFilesButton_->setToolButtonStyle(Qt::ToolButtonIconOnly);
	lk_HLayout_->addWidget(lk_OutputFilesButton_);
	//mk_Desktop_->graphicsScene().addWidget(lk_OutputFilesMenu_);
	connect(lk_OutputFilesButton_, SIGNAL(clicked()), this, SLOT(showPopupMenu()));
	*/
	
/*	QToolButton* lk_WatchOutputButton_ = new QToolButton(this);
	lk_WatchOutputButton_->setIcon(QIcon(":/icons/utilities-terminal.png"));
	lk_HLayout_->addWidget(lk_WatchOutputButton_);*/

	// make this window auto-close on exit
/*	mk_OutputBox.setAttribute(Qt::WA_QuitOnClose, false);
	mk_OutputBox.setWindowIcon(QIcon(":/icons/utilities-terminal.png"));
	mk_OutputBox.setWindowTitle(mk_pScript->title());
	connect(lk_WatchOutputButton_, SIGNAL(clicked()), this, SLOT(showOutputBox()));*/
	
/*	lk_HLayout_->addStretch();*/
	
// 	QToolButton* lk_ProposePrefixButton_ = new QToolButton(this);
// 	lk_ProposePrefixButton_->setIcon(QIcon(":/icons/select-continuous-area.png"));
// 	lk_HLayout_->addWidget(lk_ProposePrefixButton_);
// 	connect(lk_ProposePrefixButton_, SIGNAL(clicked()), this, SLOT(proposePrefixButtonClicked()));

	QWidget* lk_Container_ = new QWidget(this);
	
	k_FoldedHeader* lk_FoldedHeader_ = new k_FoldedHeader("Output files", lk_Container_, this);
	
	lk_VLayout_->addWidget(lk_FoldedHeader_);
	lk_VLayout_->addWidget(lk_Container_);
	
	lk_FoldedHeader_->hideBuddy();
	
	lk_VLayout_ = new QVBoxLayout(lk_Container_);
	lk_VLayout_->setContentsMargins(0, 0, 0, 0);

	lk_HLayout_ = new QHBoxLayout();
	lk_VLayout_->addLayout(lk_HLayout_);

	mk_Prefix.setHint("output file prefix");
	lk_HLayout_->addWidget(&mk_Prefix);
	connect(&mk_Prefix, SIGNAL(textChanged(const QString&)), this, SLOT(updateOutputFilenames()));

	lk_HLayout_ = new QHBoxLayout();
	lk_VLayout_->addLayout(lk_HLayout_);

	mk_OutputDirectory.setHint("output directory");
	mk_OutputDirectory.setReadOnly(true);
	lk_HLayout_->addWidget(&mk_OutputDirectory);
	connect(&mk_OutputDirectory, SIGNAL(textChanged(const QString&)), this, SLOT(updateOutputFilenames()));

	QToolButton* lk_SelectOutputDirectory_ = new QToolButton(lk_Container_);
	lk_SelectOutputDirectory_->setIcon(QIcon(":/icons/folder.png"));
	lk_HLayout_->addWidget(lk_SelectOutputDirectory_);
	//connect(lk_ProposePrefixButton_, SIGNAL(clicked()), this, SLOT(proposePrefixButtonClicked()));
	
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
		QCheckBox* lk_CheckBox_ = new QCheckBox("Write " + lk_OutputFileDetails["label"], lk_Container_);
		lk_CheckBox_->setProperty("key", ls_Key);
		connect(lk_CheckBox_, SIGNAL(toggled(bool)), this, SLOT(outputFileActionToggled()));
		lk_VLayout_->addWidget(lk_CheckBox_);
		mk_Checkboxes[ls_Key] = lk_CheckBox_;
		if (lk_OutputFileDetails["default"] == "yes" || lk_OutputFileDetails["default"] == "true")
			lk_CheckBox_->setChecked(true);
	}
	
	mk_OutputBox.setReadOnly(true);
	mk_OutputBox.setFont(mk_Proteomatic.consoleFont());
	mk_OutputBox.setParent(&(mk_Desktop_->pipelineMainWindow()), Qt::Tool);
	mk_OutputBox.hide();
}
