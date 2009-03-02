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

#include "ClickableLabel.h"
#include "DesktopBox.h"
#include "Desktop.h"
#include "FileList.h"
#include "HintLineEdit.h"
#include "LocalScript.h"
#include "PipelineMainWindow.h"
#include "ScriptFactory.h"
#include "Tango.h"


k_DesktopBox::k_DesktopBox(k_Desktop* ak_Parent_, k_Proteomatic& ak_Proteomatic)
	: QWidget(NULL)
	, mk_Desktop_(ak_Parent_)
	, mk_Background(QColor(TANGO_ALUMINIUM_0))
	, mk_Border(QColor(TANGO_ALUMINIUM_3))
	, mk_Proteomatic(ak_Proteomatic)
	, mk_SizeGripLabel_(NULL)
	, mb_Moving(false)
	, mb_KeepSmall(true)
	, mb_SpecialFrame(false)
	, mi_GridSize(20)
	, me_Status(r_BoxStatus::Ready)
{
}


k_DesktopBox::~k_DesktopBox()
{
}


r_BoxStatus::Enumeration k_DesktopBox::status() const
{
	return me_Status;
}


void k_DesktopBox::updateStatus()
{
}


void k_DesktopBox::reportStatus()
{
}


void k_DesktopBox::snapToGrid()
{
	/*
	int li_X = this->pos().x() + this->width() / 2;
	int li_Y = this->pos().y() + this->height() / 2;
	if (((li_X % mi_GridSize) != 0) || ((li_Y % mi_GridSize) != 0))
	{
		li_X = (li_X / mi_GridSize) * mi_GridSize;
		li_Y = (li_Y / mi_GridSize) * mi_GridSize;
		this->move(li_X - this->width() / 2, li_Y - this->height() / 2);
	}
	*/
}


void k_DesktopBox::paintEvent(QPaintEvent* /*ak_Event_*/)
{
	// TODO: is this safe? this is a workaround to achieve a compact desktop box
	if (mb_KeepSmall)
		this->resize(1, 1);
	
	QPainter lk_Painter(this);
	QBrush lk_Brush(mk_Background);
	if (mb_SpecialFrame)
		lk_Brush.setColor(QColor(TANGO_BUTTER_0));
	lk_Painter.fillRect(0, 0, width(), height(), lk_Brush);
	QPen lk_Pen(mk_Border);
	float lf_PenWidth = 1.0;
	if (mk_Desktop_->boxSelected(this))
		lf_PenWidth = 2.5;
	lk_Pen.setWidthF(lf_PenWidth);
	lk_Painter.setPen(lk_Pen);
	lk_Painter.drawRect(QRectF(lf_PenWidth * 0.5, lf_PenWidth * 0.5, (qreal)width() - lf_PenWidth, (qreal)height() - lf_PenWidth));
	
	if (mb_SpecialFrame)
	{
		lf_PenWidth = 1.0;
		lk_Pen.setWidthF(lf_PenWidth);
		lk_Pen.setColor(QColor(TANGO_BUTTER_2));
		lk_Painter.setPen(lk_Pen);
		lk_Painter.drawRect(QRectF(lf_PenWidth * 0.5 + 1.0, lf_PenWidth * 0.5 + 1.0, (qreal)width() - lf_PenWidth - 2.0, (qreal)height() - lf_PenWidth - 2.0));
	}
}


void k_DesktopBox::mousePressEvent(QMouseEvent* ak_Event_)
{
	emit mousePressed(ak_Event_->modifiers());
	if (!mb_KeepSmall && this->cursorWithinSizeGrip(ak_Event_->pos()))
	{
		mb_Resizing = true;
		mk_OldSize = this->size();
	}
	else
	{
		mb_Moving = true;
		mk_OldPosition = this->pos();
		mk_OtherBoxesOldPosition.clear();
		foreach (k_DesktopBox* lk_Box_, mk_Desktop_->selectedBoxes())
			if (lk_Box_ != this)
				mk_OtherBoxesOldPosition[lk_Box_] = lk_Box_->pos();
	}
	mk_OldMousePosition = ak_Event_->globalPos();
}


void k_DesktopBox::mouseReleaseEvent(QMouseEvent* /*ak_Event_*/)
{
	mb_Moving = false;
	mb_Resizing = false;
	mk_OtherBoxesOldPosition.clear();
}


void k_DesktopBox::mouseMoveEvent(QMouseEvent* ak_Event_)
{
	if ((!mb_KeepSmall) && this->cursorWithinSizeGrip(ak_Event_->pos()))
		this->setCursor(Qt::SizeFDiagCursor);
	else
		this->unsetCursor();
	
	if (mb_Resizing)
	{
		QPoint lk_Delta = ak_Event_->globalPos() - mk_OldMousePosition;
		this->resize(mk_OldSize + QSize(lk_Delta.x(), lk_Delta.y()));
	}
	else if (mb_Moving)
	{
		this->move(mk_OldPosition + ak_Event_->globalPos() - mk_OldMousePosition);
		
		// move all other selected boxes as well
		foreach (k_DesktopBox* lk_Box_, mk_OtherBoxesOldPosition.keys())
			lk_Box_->move(mk_OtherBoxesOldPosition[lk_Box_] + ak_Event_->globalPos() - mk_OldMousePosition);
	}
}


void k_DesktopBox::moveEvent(QMoveEvent* ak_Event_)
{
	QWidget::moveEvent(ak_Event_);
	emit moved();
}


void k_DesktopBox::resizeEvent(QResizeEvent* ak_Event_)
{
	if (mk_SizeGripLabel_)
		mk_SizeGripLabel_->move(this->width() - 17, this->height() - 24);
	QWidget::resizeEvent(ak_Event_);
	emit resized();
}


void k_DesktopBox::setKeepSmall(bool ab_Flag)
{
	// one way only, sorry
	if (!mb_KeepSmall)
		return;
	
	mb_KeepSmall = ab_Flag;
	if (!mb_KeepSmall)
	{
		this->setMouseTracking(true);
		mk_SizeGripLabel_ = new QLabel(this);
		mk_SizeGripLabel_->setPixmap(QPixmap(":icons/size-grip.png"));
		mk_SizeGripLabel_->move(this->width() - 17, this->height() - 24);
	}
}


void k_DesktopBox::setSpecialFrame(bool ab_Flag)
{
	mb_SpecialFrame = ab_Flag;
	this->repaint();
}


bool k_DesktopBox::cursorWithinSizeGrip(QPoint ak_Position)
{
	QPoint lk_Point = (ak_Position - QPoint(this->width(), this->height()));
	return ((ak_Position - QPoint(this->width(), this->height())).manhattanLength() < 16);
}


k_ScriptBox::k_ScriptBox(RefPtr<IScript> ak_pScript, k_Desktop* ak_Parent_, k_Proteomatic& ak_Proteomatic)
	: k_DesktopBox(ak_Parent_, ak_Proteomatic)
	, mk_pScript(ak_pScript)
{
	k_LocalScript* lk_LocalScript_ = dynamic_cast<k_LocalScript*>(mk_pScript.get_Pointer());
	if (lk_LocalScript_)
	{
		connect(lk_LocalScript_, SIGNAL(started()), this, SLOT(scriptStarted()));
		connect(lk_LocalScript_, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(scriptFinished(int, QProcess::ExitStatus)));
		connect(lk_LocalScript_, SIGNAL(readyRead()), this, SLOT(scriptReadyRead()));
	}
	
	this->setLayout(&mk_Layout);
	
	QHBoxLayout* lk_HLayout_ = new QHBoxLayout(this);
	
    QLabel* lk_Label_ = new QLabel(mk_pScript->title(), this);
    QFont lk_Font(lk_Label_->font());
    lk_Font.setBold(true);
    lk_Label_->setFont(lk_Font);
	lk_HLayout_->addWidget(lk_Label_);
	lk_HLayout_->addStretch();
	
	lk_HLayout_->addWidget(&mk_StatusLabel);
	mk_StatusLabel.setIconSize(QSize(16, 16));
	connect(&mk_StatusLabel, SIGNAL(clicked()), this, SLOT(reportStatus()));
	
	mk_Layout.addLayout(lk_HLayout_);
	
	QFrame* lk_Frame_ = new QFrame(this);
	lk_Frame_->setFrameStyle(QFrame::HLine | QFrame::Plain);
	lk_Frame_->setLineWidth(1);
	lk_Frame_->setStyleSheet("color: #888a85;");
	mk_Layout.addWidget(lk_Frame_);

	QHBoxLayout* lk_ButtonLayout_ = new QHBoxLayout();
	
	QScrollArea* lk_ScrollArea_ = new QScrollArea();
	lk_ScrollArea_->setWidget(&mk_pScript->parameterWidget());
	lk_ScrollArea_->setWidgetResizable(true);
	
	QDialog* lk_ParameterWidget_ = new QDialog();
	
	mk_pParameterWidget = RefPtr<QWidget>(lk_ParameterWidget_);
	
	mk_pParameterWidget->resize(500, 600);
	mk_pParameterWidget->setWindowTitle(mk_pScript->title());
	mk_pParameterWidget->setWindowIcon(QIcon(":icons/proteomatic.png"));
	
	QBoxLayout* lk_VLayout_ = new QVBoxLayout(lk_ParameterWidget_);
	lk_VLayout_->setContentsMargins(0, 0, 0, 0);
	lk_VLayout_->setSpacing(0);
	
	QToolBar* lk_ToolBar_ = new QToolBar(lk_ParameterWidget_);
	lk_ToolBar_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

	lk_ToolBar_->addAction(QIcon(":/icons/document-properties.png"), "Profiles", this, SLOT(showProfileManager()));
	lk_ToolBar_->addAction(QIcon(":/icons/edit-clear.png"), "Reset", dynamic_cast<QObject*>(mk_pScript.get_Pointer()), SLOT(reset()));
	QWidget* lk_StretchLabel_ = new QWidget(lk_ParameterWidget_);
	lk_StretchLabel_->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum));
	lk_StretchLabel_->setContentsMargins(0, 0, 0, 0);
	lk_ToolBar_->addWidget(lk_StretchLabel_);
	lk_ToolBar_->addAction(QIcon(":/icons/dialog-ok.png"), "Close", lk_ParameterWidget_, SLOT(accept()));
	
	lk_VLayout_->addWidget(lk_ToolBar_);
	lk_VLayout_->addWidget(lk_ScrollArea_);
	
	QToolButton* lk_ConfigureButton_ = new QToolButton(this);
	lk_ConfigureButton_->setIcon(QIcon(":/icons/preferences-system.png"));
	lk_ButtonLayout_->addWidget(lk_ConfigureButton_);
	connect(lk_ConfigureButton_, SIGNAL(clicked()), this, SLOT(showParameterWidget()));

	QToolButton* lk_ShowOutputButton_ = new QToolButton(this);
	//lk_ShowOutputButton_->setCheckable(true);
	lk_ShowOutputButton_->setIcon(QIcon(":/icons/utilities-terminal.png"));
	lk_ButtonLayout_->addWidget(lk_ShowOutputButton_);
	mk_pOutputWidget = RefPtr<QWidget>(new QWidget());
	mk_pOutput = RefPtr<QTextEdit>(new QTextEdit(mk_pOutputWidget.get_Pointer()));
	mk_pOutput->setReadOnly(true);
	mk_pOutput->setFont(mk_Proteomatic.consoleFont());
	QBoxLayout* lk_SVLayout_ = new QVBoxLayout(mk_pOutputWidget.get_Pointer());
	lk_SVLayout_->addWidget(mk_pOutput.get_Pointer());
	QBoxLayout* lk_SHLayout_ = new QHBoxLayout(mk_pOutputWidget.get_Pointer());
	lk_SHLayout_->addStretch();
	QPushButton* lk_CloseButton_ = new QPushButton(QIcon(":icons/dialog-ok.png"), "Close", mk_pOutputWidget.get_Pointer());
	connect(lk_CloseButton_, SIGNAL(clicked()), mk_pOutputWidget.get_Pointer(), SLOT(hide()));
	lk_SHLayout_->addWidget(lk_CloseButton_);
	lk_SVLayout_->addLayout(lk_SHLayout_);
	mk_pOutputWidget->setWindowIcon(QIcon(":icons/proteomatic.png"));
	mk_pOutputWidget->setWindowTitle(mk_pScript->title());
	connect(lk_ShowOutputButton_, SIGNAL(clicked()), mk_pOutputWidget.get_Pointer(), SLOT(show()));
	connect(lk_ShowOutputButton_, SIGNAL(clicked()), mk_pOutputWidget.get_Pointer(), SLOT(raise()));
	
	lk_ButtonLayout_->addStretch();
	
	/*
	QToolButton* lk_ClearPrefixButton_ = new QToolButton(this);
	lk_ClearPrefixButton_->setIcon(QIcon(":/icons/dialog-cancel.png"));
	lk_ButtonLayout_->addWidget(lk_ClearPrefixButton_);
	connect(lk_ClearPrefixButton_, SIGNAL(clicked()), &mk_PrefixWidget, SLOT(clear()));
	*/

	QToolButton* lk_ProposePrefixButton_ = new QToolButton(this);
	lk_ProposePrefixButton_->setIcon(QIcon(":/icons/select-continuous-area.png"));
	lk_ButtonLayout_->addWidget(lk_ProposePrefixButton_);
	connect(lk_ProposePrefixButton_, SIGNAL(clicked()), this, SLOT(proposePrefixButtonClicked()));


	mk_Layout.addLayout(lk_ButtonLayout_);

	mk_Layout.addWidget(&mk_PrefixWidget);
	connect(&mk_PrefixWidget, SIGNAL(textChanged(const QString&)), this, SLOT(prefixChanged(const QString&)));

	QStringList lk_OutKeys = mk_pScript->outputFileKeys();
	foreach (QString ls_Key, lk_OutKeys)
	{
		QHash<QString, QString> lk_OutFile = mk_pScript->outputFileDetails(ls_Key);
		QCheckBox* lk_CheckBox_ = new QCheckBox(lk_OutFile["label"], this);
		mk_Layout.addWidget(lk_CheckBox_);
		lk_CheckBox_->setObjectName(ls_Key);
		connect(lk_CheckBox_, SIGNAL(toggled(bool)), this, SLOT(toggleOutput(bool)));
	}
	
	this->updateStatus();
}


k_ScriptBox::~k_ScriptBox() 
{ 
}


QList<k_OutputFileBox*> k_ScriptBox::outputFileBoxes()
{
	return mk_OutputFileBoxes.values();
}


RefPtr<IScript> k_ScriptBox::script()
{
	return mk_pScript;
}


bool k_ScriptBox::allInputFilesExist()
{
	foreach (tk_StringStringHash lk_Hash, mk_InputFileBoxes.values())
	{
		foreach (QString ls_Path, lk_Hash.keys())
		{
			if (!QFileInfo(ls_Path).exists())
				return false;
		}
	}
	return true;
}


void k_ScriptBox::toggleOutput(bool ab_Enabled)
{
	QCheckBox* lk_CheckBox_ = dynamic_cast<QCheckBox*>(sender());
	if (!lk_CheckBox_)
		return;
	
	QString ls_Key = lk_CheckBox_->objectName();
	this->toggleOutputFile(ls_Key, ab_Enabled, false);
}


void k_ScriptBox::removeOutputFileBox(k_OutputFileBox* ak_OutputFileBox_)
{
	if (mk_OutputFileBoxes.values().contains(ak_OutputFileBox_))
	{
		// don't perform checks on remove to avoid recursion
		mk_Desktop_->removeBox(ak_OutputFileBox_, false);
		mk_CheckBoxForOutputFileBox[ak_OutputFileBox_]->setChecked(false);
		mk_CheckBoxForOutputFileBox.remove(ak_OutputFileBox_);
		foreach (QString ls_Key, mk_OutputFileBoxes.keys())
		{
			if (mk_OutputFileBoxes[ls_Key] == ak_OutputFileBox_)
				mk_OutputFileBoxes.remove(ls_Key);
		}
	}
}


// this doesn't reset script parameters but clear the finished/failed state
void k_ScriptBox::resetScript()
{
	me_Status = r_BoxStatus::Ready;
	ms_Output.clear();
	mk_pOutput->setText(ms_Output.text());
	mk_pOutput->moveCursor(QTextCursor::End);
	mk_pOutput->ensureCursorVisible();
	
	this->updateStatus();
}


void k_ScriptBox::start()
{
	// collect input files
	QStringList lk_Arguments;
	foreach (tk_StringStringHash lk_Hash, mk_InputFileBoxes.values())
		lk_Arguments << lk_Hash.keys();

	lk_Arguments << "-outputDirectory" << mk_Desktop_->pipelineMainWindow().outputDirectory();
	lk_Arguments << "-outputPrefix" << mk_PrefixWidget.text();
	foreach (QString ls_Key, mk_OutputFileBoxes.keys())
		lk_Arguments << "-" + ls_Key << "yes";
	
	mk_pScript->start(lk_Arguments);
}


void k_ScriptBox::showParameterWidget()
{
	mk_pParameterWidget->show();
}


void k_ScriptBox::prefixChanged(const QString& as_Prefix)
{
	foreach (k_OutputFileBox* lk_Box_, mk_OutputFileBoxes.values())
		lk_Box_->setPrefix(as_Prefix);
}


void k_ScriptBox::proposePrefixButtonClicked()
{
	if (!mk_pScript)
		return;

	if (mk_pScript->status() == r_ScriptStatus::Running)
		return;

	QStringList lk_Arguments;

	foreach (tk_StringStringHash lk_BoxFiles, mk_InputFileBoxes.values())
		lk_Arguments << lk_BoxFiles.keys();

	if (mk_pScript->location() == r_ScriptLocation::Local)
	{
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
	}
}


void k_ScriptBox::fileBoxConnected(IFileBox* ak_FileBox_)
{
	// this is about input file boxes here

	// watch the file box for changes
	connect(dynamic_cast<k_DesktopBox*>(ak_FileBox_), SIGNAL(changed()), this, SLOT(fileBoxChanged()));
	
	mk_InputFileBoxes[ak_FileBox_] = QHash<QString, QString>();

	// for each file name, determine the input file group
	foreach (QString ls_Filename, ak_FileBox_->fileNames())
	{
		QString ls_Key = mk_pScript->inputGroupForFilename(ls_Filename);
		mk_InputFileBoxes[ak_FileBox_][ls_Filename] = ls_Key;
	}

	this->updateStatus();
}


void k_ScriptBox::fileBoxDisconnected(IFileBox* ak_FileBox_)
{
	// don't watch this file box for changes anymore
	dynamic_cast<k_DesktopBox*>(ak_FileBox_)->disconnect(SIGNAL(changed()), this, SLOT(fileBoxChanged()));
	mk_InputFileBoxes.remove(ak_FileBox_);
	this->updateStatus();
}


void k_ScriptBox::updateStatus()
{
	QHash<QString, QSet<QString> > lk_Files;
	
	foreach (tk_StringStringHash lk_BoxFiles, mk_InputFileBoxes.values())
	{
		foreach (QString ls_Filename, lk_BoxFiles.keys())
		{
			QString ls_Key = lk_BoxFiles[ls_Filename];
			if (!lk_Files.contains(ls_Key))
				lk_Files[ls_Key] = QSet<QString>();
			lk_Files[ls_Key].insert(ls_Filename);
		}
	}
	
	// update status unless finished or failed
	if ((me_Status != r_BoxStatus::Finished) && (me_Status != r_BoxStatus::Failed) && (me_Status != r_BoxStatus::Running))
	{
		if (mk_pScript->checkInputFiles(lk_Files, ms_InputFilesErrorMessage))
			me_Status = r_BoxStatus::Ready;
		else
			me_Status = r_BoxStatus::InputFilesMissing;
	}
	
	/*
	if (me_Status == r_BoxStatus::Ready)
		mk_StatusLabel.setPixmap(QPixmap(":icons/appointment.png").scaledToWidth(16, Qt::SmoothTransformation));
	else
		mk_StatusLabel.setPixmap(QPixmap(":icons/dialog-warning.png").scaledToWidth(16, Qt::SmoothTransformation));
	*/
	if (me_Status == r_BoxStatus::Ready)
		mk_StatusLabel.setIcon(QIcon(":icons/appointment.png"));
	else if (me_Status == r_BoxStatus::InputFilesMissing)
		mk_StatusLabel.setIcon(QIcon(":icons/dialog-warning.png"));
	else if (me_Status == r_BoxStatus::Running)
		mk_StatusLabel.setIcon(QIcon(":icons/applications-system.png"));
	else if (me_Status == r_BoxStatus::Finished)
		mk_StatusLabel.setIcon(QIcon(":icons/dialog-ok.png"));
	else if (me_Status == r_BoxStatus::Failed)
		mk_StatusLabel.setIcon(QIcon(":icons/process-stop.png"));
}


void k_ScriptBox::reportStatus()
{
	if (me_Status == r_BoxStatus::Ready)
		mk_Proteomatic.showMessageBox("Script status", "The script is ready to go.", ":icons/appointment.png");
	else if (me_Status == r_BoxStatus::InputFilesMissing)
		mk_Proteomatic.showMessageBox("Script status", QString("The script is not ready because of the following problems:<br />%1").arg(ms_InputFilesErrorMessage), ":icons/dialog-warning.png");
	else if (me_Status == r_BoxStatus::Running)
		mk_Proteomatic.showMessageBox("Script status", "The script is currently running.", ":icons/applications-system.png");
	else if (me_Status == r_BoxStatus::Finished)
		mk_Proteomatic.showMessageBox("Script status", "The script has finished successfully.", ":icons/dialog-ok.png");
	else if (me_Status == r_BoxStatus::Failed)
		mk_Proteomatic.showMessageBox("Script status", "The script has failed.", ":icons/process-stop.png");
}


void k_ScriptBox::toggleOutputFile(QString as_Key, bool ab_Enabled, bool ab_ToggleCheckBox)
{
	QHash<QString, QString> lk_OutFile = mk_pScript->outputFileDetails(as_Key);
	if (ab_Enabled)
	{
		if (!mk_OutputFileBoxes.contains(as_Key))
		{
			k_OutputFileBox* lk_FileBox_ = new k_OutputFileBox(mk_Desktop_, mk_Proteomatic, *this);
			lk_FileBox_->setFilename(lk_OutFile["filename"]);
			mk_Desktop_->addBox(lk_FileBox_, this);
			mk_Desktop_->connectBoxes(this, lk_FileBox_);
			mk_OutputFileBoxes[as_Key] = lk_FileBox_;
			QString ls_Prefix = mk_PrefixWidget.text();
			lk_FileBox_->setPrefix(ls_Prefix);
			mk_CheckBoxForOutputFileBox[lk_FileBox_] = this->findChild<QCheckBox*>(as_Key);
			if (ab_ToggleCheckBox)
				this->findChild<QCheckBox*>(as_Key)->setCheckState(Qt::Checked);
		}
	}
	else
	{
		if (mk_OutputFileBoxes.contains(as_Key))
		{
			mk_Desktop_->removeBox(mk_OutputFileBoxes[as_Key]);
			mk_CheckBoxForOutputFileBox.remove(mk_OutputFileBoxes[as_Key]);
			mk_OutputFileBoxes.remove(as_Key);
			if (ab_ToggleCheckBox)
				this->findChild<QCheckBox*>(as_Key)->setCheckState(Qt::Unchecked);
		}
	}
}


void k_ScriptBox::fileBoxChanged()
{
	IFileBox* lk_Box_ = dynamic_cast<IFileBox*>(sender());
	if (!lk_Box_)
		return;
	
	// clear and recreate filenames for this box
	mk_InputFileBoxes[lk_Box_] = QHash<QString, QString>();

	// for each file name, determine the input file group
	foreach (QString ls_Filename, lk_Box_->fileNames())
	{
		QString ls_Key = mk_pScript->inputGroupForFilename(ls_Filename);
		mk_InputFileBoxes[lk_Box_][ls_Filename] = ls_Key;
	}
	
	if (dynamic_cast<k_InputFileListBox*>(lk_Box_))
	{
		this->setSpecialFrame(dynamic_cast<k_InputFileListBox*>(lk_Box_)->isFileBatch());
	}
	this->updateStatus();
}


void k_ScriptBox::showProfileManager()
{
	RefPtr<k_ProfileManager> lk_pProfileManager(new k_ProfileManager(mk_Proteomatic, mk_pScript.get_Pointer()));
	lk_pProfileManager->reset();
	if (lk_pProfileManager->exec())
		mk_pScript->setConfiguration(lk_pProfileManager->getGoodProfileMix());
}


void k_ScriptBox::scriptStarted()
{
	me_Status = r_BoxStatus::Running;
	this->updateStatus();
}


void k_ScriptBox::scriptFinished(int ai_ExitCode, QProcess::ExitStatus ae_Status)
{
	if ((ae_Status == QProcess::CrashExit) || (ai_ExitCode != 0))
		me_Status = r_BoxStatus::Failed;
	else
		me_Status = r_BoxStatus::Finished;
	
	this->updateStatus();
	
	emit scriptFinished();
}


void k_ScriptBox::scriptReadyRead()
{
	addOutput(mk_pScript->readAll());
}


void k_ScriptBox::addOutput(QString as_Text)
{
	ms_Output.append(as_Text);
	mk_pOutput->setText(ms_Output.text());
	mk_pOutput->moveCursor(QTextCursor::End);
	mk_pOutput->ensureCursorVisible();
}


k_ConverterScriptBox::k_ConverterScriptBox(RefPtr<IScript> ak_pScript, k_Desktop* ak_Parent_, k_Proteomatic& ak_Proteomatic)
	: k_ScriptBox(ak_pScript, ak_Parent_, ak_Proteomatic)
{
	QBoxLayout* lk_HBoxLayout_ = new QHBoxLayout(this);
	lk_HBoxLayout_->addWidget(&mk_FileList);
	
	QBoxLayout* lk_VBoxLayout_ = new QVBoxLayout(this);
	
	k_ClickableLabel* lk_ArrowLabel_ = new k_ClickableLabel(this);
	lk_ArrowLabel_->setPixmap(QPixmap(":icons/arrow-semi-transparent.png").scaledToWidth(20, Qt::SmoothTransformation));
	lk_ArrowLabel_->setContentsMargins(0, 0, 4, 0);
	lk_VBoxLayout_->addStretch();
	lk_VBoxLayout_->addWidget(lk_ArrowLabel_);
	connect(lk_ArrowLabel_, SIGNAL(pressed()), this, SIGNAL(arrowPressed()));
	connect(lk_ArrowLabel_, SIGNAL(released()), this, SIGNAL(arrowReleased()));
	lk_HBoxLayout_->addLayout(lk_VBoxLayout_);
	
	mk_Layout.addLayout(lk_HBoxLayout_);

	this->setKeepSmall(false);
	this->resize(1, 1);
}


k_ConverterScriptBox::~k_ConverterScriptBox()
{
}


QStringList k_ConverterScriptBox::fileNames()
{
	return QStringList();
}


k_InputFileBox::k_InputFileBox(k_Desktop* ak_Parent_, k_Proteomatic& ak_Proteomatic)
	: k_DesktopBox(ak_Parent_, ak_Proteomatic)
	, mk_Label("")
{
	QHBoxLayout* lk_Layout_ = new QHBoxLayout(this);
	lk_Layout_->setContentsMargins(0, 0, 0, 0);
	lk_Layout_->addWidget(&mk_IconLabel);
	connect(&mk_IconLabel, SIGNAL(clicked()), this, SLOT(reportStatus()));
	mk_IconLabel.setIconSize(QSize(16, 16));
	mk_IconLabel.setVisible(false);
	lk_Layout_->addWidget(&mk_Label);
	lk_Layout_->setContentsMargins(8, 8, 8, 8);
	k_ClickableLabel* lk_ArrowLabel_ = new k_ClickableLabel(this);
	lk_ArrowLabel_->setPixmap(QPixmap(":icons/arrow-semi-transparent.png").scaledToWidth(20, Qt::SmoothTransformation));
	lk_ArrowLabel_->setContentsMargins(0, 0, 4, 0);
	lk_Layout_->addStretch();
	lk_Layout_->addWidget(lk_ArrowLabel_);
	connect(lk_ArrowLabel_, SIGNAL(pressed()), this, SIGNAL(arrowPressed()));
	connect(lk_ArrowLabel_, SIGNAL(released()), this, SIGNAL(arrowReleased()));
	this->updateStatus();
}


k_InputFileBox::~k_InputFileBox()
{
}


QStringList k_InputFileBox::fileNames()
{
	return QStringList() << ms_Filename;
}


void k_InputFileBox::setFilename(const QString& as_Filename)
{
	ms_Filename = as_Filename;
	this->updateStatus();
	emit changed();
}


QString k_InputFileBox::filename() const
{
	return ms_Filename;
}


void k_InputFileBox::updateStatus()
{
	mk_Label.setText(this->displayString());
	if (this->fileExists())
		mk_Label.setStyleSheet("color: #000");
	else
		mk_Label.setStyleSheet("color: #888");
}


void k_InputFileBox::reportStatus()
{
}


QString k_InputFileBox::displayString() const
{
	return QFileInfo(ms_Filename).fileName();
}


bool k_InputFileBox::fileExists()
{
	return QFileInfo(ms_Filename).exists();
}


k_InputFileListBox::k_InputFileListBox(k_Desktop* ak_Parent_, k_Proteomatic& ak_Proteomatic)
	: k_DesktopBox(ak_Parent_, ak_Proteomatic)
	, mk_FileList(this, true, true)
	, me_Type(r_InputFileListBoxType::List)
{
	this->setKeepSmall(false);
	QBoxLayout* lk_MainLayout_ = new QVBoxLayout(this);
	QBoxLayout* lk_HLayout_ = new QHBoxLayout(this);
	lk_HLayout_->addWidget(&mk_Label);
	lk_HLayout_->addStretch();
	mk_BatchModeButton.setIcon(QIcon(":icons/cycle.png"));
	mk_BatchModeButton.setCheckable(true);
	mk_BatchModeButton.setChecked(false);
	lk_HLayout_->addWidget(&mk_BatchModeButton);
	lk_MainLayout_->addLayout(lk_HLayout_);
	lk_HLayout_ = new QHBoxLayout(this);
	lk_MainLayout_->addLayout(lk_HLayout_);
	lk_HLayout_->addWidget(&mk_FileList);
	QBoxLayout* lk_VLayout_ = new QVBoxLayout(this);
	QToolButton* lk_AddFilesButton_ = new QToolButton(this);
	lk_AddFilesButton_->setIcon(QIcon(":/icons/document-open.png"));
	connect(lk_AddFilesButton_, SIGNAL(clicked()), this, SLOT(addFilesButtonClicked()));
	mk_RemoveFilesButton.setIcon(QIcon(":/icons/list-remove.png"));
	connect(&mk_RemoveFilesButton, SIGNAL(clicked()), &mk_FileList, SLOT(removeSelection()));
	connect(&mk_RemoveFilesButton, SIGNAL(clicked()), this, SIGNAL(changed()));
	lk_VLayout_->addWidget(lk_AddFilesButton_);
	lk_VLayout_->addWidget(&mk_RemoveFilesButton);
	lk_VLayout_->addStretch();

	k_ClickableLabel* lk_ArrowLabel_ = new k_ClickableLabel(this);
	lk_ArrowLabel_->setPixmap(QPixmap(":icons/arrow-semi-transparent.png").scaledToWidth(20, Qt::SmoothTransformation));
	//lk_ArrowLabel_->setContentsMargins(0, 0, 0, 0);
	lk_VLayout_->addWidget(lk_ArrowLabel_);
	connect(lk_ArrowLabel_, SIGNAL(pressed()), this, SIGNAL(arrowPressed()));
	connect(lk_ArrowLabel_, SIGNAL(released()), this, SIGNAL(arrowReleased()));
	
	connect(&mk_FileList, SIGNAL(changed()), this, SLOT(updateStatus()));
	connect(&mk_FileList, SIGNAL(changed()), this, SIGNAL(changed()));
	connect(&mk_FileList, SIGNAL(itemSelectionChanged()), this, SLOT(toggleUi()));
	connect(&mk_BatchModeButton, SIGNAL(toggled(bool)), this, SLOT(setSpecialFrame(bool)));
	connect(&mk_BatchModeButton, SIGNAL(toggled(bool)), this, SLOT(updateStatus()));
	connect(&mk_BatchModeButton, SIGNAL(toggled(bool)), this, SIGNAL(changed()));
	
	lk_HLayout_->addLayout(lk_VLayout_);
	this->resize(300, 1);
	this->setAcceptDrops(true);
	this->updateStatus();
	this->toggleUi();
}


k_InputFileListBox::~k_InputFileListBox()
{
}


QStringList k_InputFileListBox::fileNames()
{
	return mk_FileList.files();
}


void k_InputFileListBox::addFilename(const QString& as_Filename)
{
	QString ls_Path = as_Filename;
	mk_FileList.addInputFile(ls_Path);
	emit changed();
}


bool k_InputFileListBox::isFileBatch()
{
	return mk_BatchModeButton.isChecked();
}


void k_InputFileListBox::addFilesButtonClicked()
{
	QStringList lk_Files = QFileDialog::getOpenFileNames(mk_Proteomatic.messageBoxParent(), tr("Add files"), mk_Proteomatic.getConfiguration(CONFIG_REMEMBER_INPUT_FILES_PATH).toString(), tr("All files (*.*)"));
	QString ls_FirstPath = "";
	foreach (QString ls_Path, lk_Files)
	{
		if (ls_FirstPath.isEmpty())
			ls_FirstPath = ls_Path;
		addFilename(ls_Path);
	}
	if (!ls_FirstPath.isEmpty())
		mk_Proteomatic.getConfigurationRoot()[CONFIG_REMEMBER_INPUT_FILES_PATH] = QFileInfo(ls_FirstPath).absolutePath();
}


void k_InputFileListBox::updateStatus()
{
	QString ls_Count;
	int li_Count = mk_FileList.files().size();
	if (li_Count == 0)
		ls_Count = "no files";
	else if (li_Count == 1)
		ls_Count = "1 file";
	else
		ls_Count = QString("%1 files").arg(li_Count);
	
	mk_Label.setText(QString("<b>File %1</b> (%2)").arg(mk_BatchModeButton.isChecked() ? "batch" : "list").arg(ls_Count));
}


void k_InputFileListBox::toggleUi()
{
	mk_RemoveFilesButton.setEnabled(!mk_FileList.selectedItems().empty());
}


void k_InputFileListBox::dragEnterEvent(QDragEnterEvent* ak_Event_)
{
	ak_Event_->acceptProposedAction();
}


void k_InputFileListBox::dragMoveEvent(QDragMoveEvent* ak_Event_)
{
	ak_Event_->acceptProposedAction();
}


void k_InputFileListBox::dropEvent(QDropEvent* ak_Event_)
{
	ak_Event_->acceptProposedAction();
	foreach (QUrl lk_Url, ak_Event_->mimeData()->urls())
	{
		QString ls_Path = lk_Url.toLocalFile();
		if (ls_Path != "")
		{
			QFileInfo lk_FileInfo(ls_Path);
			if (!lk_FileInfo.isDir())
				this->addFilename(ls_Path);
		}
	}
}


k_OutputFileBox::k_OutputFileBox(k_Desktop* ak_Parent_, k_Proteomatic& ak_Proteomatic, k_ScriptBox& ak_ScriptBox)
	: k_InputFileBox(ak_Parent_, ak_Proteomatic)
	, mk_ScriptBox(ak_ScriptBox)
{
	mk_IconLabel.setIcon(QIcon(":icons/dialog-warning.png"));
}


k_OutputFileBox::~k_OutputFileBox()
{
}


QStringList k_OutputFileBox::fileNames()
{
	return QStringList() << this->assembledPath();
}


QString k_OutputFileBox::directory()
{
	return ms_Directory;
}


QString k_OutputFileBox::prefix()
{
	return ms_Prefix;
}


QString k_OutputFileBox::assembledPath()
{
	return QFileInfo(QDir(ms_Directory), ms_Prefix + ms_Filename).absoluteFilePath();
}


void k_OutputFileBox::setDirectory(const QString& as_Directory)
{
	ms_Directory = as_Directory;
	this->updateStatus();
	emit changed();
}


void k_OutputFileBox::setPrefix(const QString& as_Prefix)
{
	ms_Prefix = as_Prefix;
	this->updateStatus();
	emit changed();
}


bool k_OutputFileBox::fileExists()
{
	return QFileInfo(this->assembledPath()).exists();
}


QString k_OutputFileBox::displayString() const
{
	return ms_Prefix + QFileInfo(ms_Filename).fileName();
}


void k_OutputFileBox::updateStatus()
{
	mk_Label.setText(this->displayString());
	if (this->fileExists())
	{
		if (mk_ScriptBox.status() == r_BoxStatus::Finished)
			me_Status = r_BoxStatus::Ready;
		else
			me_Status = r_BoxStatus::OutputFileExists;
	}
	else
		me_Status = r_BoxStatus::Ready;

	if (me_Status == r_BoxStatus::OutputFileExists)
	{
		mk_Label.setStyleSheet("color: #888");
		mk_IconLabel.setVisible(true);
	}
	else
	{
		mk_IconLabel.setVisible(false);
		if ((mk_ScriptBox.status() == r_BoxStatus::Finished) && (this->fileExists()))
		{
			mk_Label.setStyleSheet("color: #000");
		}
		else
		{
			mk_Label.setStyleSheet("color: #888");
		}
	}
}


void k_OutputFileBox::reportStatus()
{
	if (me_Status == r_BoxStatus::OutputFileExists)
		mk_Proteomatic.showMessageBox("Output file status", "The output file already exists. Please specify a filename prefix or choose a different output directory. Alternatively, you can delete the file.", ":icons/dialog-warning.png");
}
