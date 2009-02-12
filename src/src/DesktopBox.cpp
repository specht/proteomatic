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
#include "ScriptFactory.h"


k_DesktopBox::k_DesktopBox(k_Desktop* ak_Parent_, k_Proteomatic& ak_Proteomatic)
	: QWidget(NULL)
	, mk_Desktop_(ak_Parent_)
	, mk_Background(QColor("#eeeeec"))
	, mk_Border(QColor("#888a85"))
	, mk_Proteomatic(ak_Proteomatic)
	, mb_Moving(false)
	, mb_KeepSmall(true)
	, mk_SizeGripLabel_(NULL)
{
}


k_DesktopBox::~k_DesktopBox()
{
}


void k_DesktopBox::paintEvent(QPaintEvent* ak_Event_)
{
	// TODO: is this safe? this is a workaround to achieve a compact desktop box
	if (mb_KeepSmall)
		this->resize(1, 1);
	
	QPainter lk_Painter(this);
	lk_Painter.fillRect(0, 0, width(), height(), mk_Background);
	QPen lk_Pen(mk_Border);
	float lf_PenWidth = 1.0;
	if (mk_Desktop_->boxSelected(this))
		lf_PenWidth = 2.5;
	lk_Pen.setWidthF(lf_PenWidth);
	lk_Painter.setPen(lk_Pen);
	lk_Painter.drawRect(QRectF(lf_PenWidth * 0.5, lf_PenWidth * 0.5, (qreal)width() - lf_PenWidth, (qreal)height() - lf_PenWidth));
	/*
	if (mk_Desktop_->boxSelected(this))
		lk_Painter.drawRect(2, 2, width() - 5, height() - 5);
	*/
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
	}
	mk_OldMousePosition = ak_Event_->globalPos();
}


void k_DesktopBox::mouseReleaseEvent(QMouseEvent* ak_Event_)
{
	mb_Moving = false;
	mb_Resizing = false;
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
		this->move(mk_OldPosition + ak_Event_->globalPos() - mk_OldMousePosition);
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


bool k_DesktopBox::cursorWithinSizeGrip(QPoint ak_Position)
{
	QPoint lk_Point = (ak_Position - QPoint(this->width(), this->height()));
	return ((ak_Position - QPoint(this->width(), this->height())).manhattanLength() < 16);
}


k_ScriptBox::k_ScriptBox(QString as_ScriptName, k_Desktop* ak_Parent_, k_Proteomatic& ak_Proteomatic)
	: k_DesktopBox(ak_Parent_, ak_Proteomatic)
	, mk_Script_(k_ScriptFactory::makeScript(as_ScriptName, ak_Proteomatic, false))
{
	this->setLayout(&mk_Layout);
	
	QHBoxLayout* lk_HLayout_ = new QHBoxLayout(this);
	
    QLabel* lk_Label_ = new QLabel(mk_Script_->title(), this);
    QFont lk_Font(lk_Label_->font());
    lk_Font.setBold(true);
    lk_Label_->setFont(lk_Font);
	lk_HLayout_->addWidget(lk_Label_);
	lk_HLayout_->addStretch();
	
	lk_HLayout_->addWidget(&mk_StatusLabel);
	
	mk_Layout.addLayout(lk_HLayout_);
	
	QFrame* lk_Frame_ = new QFrame(this);
    lk_Frame_->setFrameStyle(QFrame::HLine | QFrame::Plain);
	lk_Frame_->setLineWidth(1);
	lk_Frame_->setStyleSheet("color: #888a85;");
    mk_Layout.addWidget(lk_Frame_);

	QHBoxLayout* lk_ButtonLayout_ = new QHBoxLayout();
	
	QScrollArea* lk_ScrollArea_ = new QScrollArea();
	lk_ScrollArea_->setWidget(mk_Script_->parameterWidget());
	lk_ScrollArea_->setWidgetResizable(true);
	
	QDialog* lk_ParameterWidget_ = new QDialog();
	
	mk_pParameterWidget = RefPtr<QWidget>(lk_ParameterWidget_);
	
	mk_pParameterWidget->resize(500, 600);
	mk_pParameterWidget->setWindowTitle(mk_Script_->title());
	mk_pParameterWidget->setWindowIcon(QIcon(":icons/proteomatic.png"));
	
	QBoxLayout* lk_VLayout_ = new QVBoxLayout(lk_ParameterWidget_);
	lk_VLayout_->setContentsMargins(0, 0, 0, 0);
	lk_VLayout_->setSpacing(0);
	
	QToolBar* lk_ToolBar_ = new QToolBar(lk_ParameterWidget_);
	lk_ToolBar_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

	lk_ToolBar_->addAction(QIcon(":/icons/document-properties.png"), "Profiles", this, SLOT(showProfileManager()));
	lk_ToolBar_->addAction(QIcon(":/icons/edit-clear.png"), "Reset", mk_Script_, SLOT(reset()));
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
	lk_ShowOutputButton_->setIcon(QIcon(":/icons/utilities-terminal.png"));
	lk_ButtonLayout_->addWidget(lk_ShowOutputButton_);
	//connect(lk_ShowOutputButton_, SIGNAL(clicked()), mk_Script.parameterWidget(), SLOT(show()));
	
	lk_ButtonLayout_->addStretch();
	
	QToolButton* lk_ClearPrefixButton_ = new QToolButton(this);
	lk_ClearPrefixButton_->setIcon(QIcon(":/icons/dialog-cancel.png"));
	lk_ButtonLayout_->addWidget(lk_ClearPrefixButton_);
	connect(lk_ClearPrefixButton_, SIGNAL(clicked()), &mk_PrefixWidget, SLOT(clear()));

	QToolButton* lk_ProposePrefixButton_ = new QToolButton(this);
	lk_ProposePrefixButton_->setIcon(QIcon(":/icons/select-continuous-area.png"));
	lk_ButtonLayout_->addWidget(lk_ProposePrefixButton_);
	connect(lk_ProposePrefixButton_, SIGNAL(clicked()), this, SLOT(proposePrefixButtonClicked()));


/*
	QPushButton* lk_ShowInfoButton_ = new QPushButton(this);
	lk_ShowInfoButton_->setIcon(QIcon(":/icons/info.png"));
	lk_ButtonLayout_->addWidget(lk_ShowInfoButton_);
	*/
	//connect(lk_ShowInfoButton_, SIGNAL(clicked()), mk_Script.parameterWidget(), SLOT(show()));

	mk_Layout.addLayout(lk_ButtonLayout_);

	mk_Layout.addWidget(&mk_PrefixWidget);
	connect(&mk_PrefixWidget, SIGNAL(textChanged(const QString&)), this, SLOT(prefixChanged(const QString&)));

	QStringList lk_OutKeys = mk_Script_->outFiles();
	foreach (QString ls_Key, lk_OutKeys)
	{
		QHash<QString, QString> lk_OutFile = mk_Script_->outFileDetails(ls_Key);
		QCheckBox* lk_CheckBox_ = new QCheckBox(lk_OutFile["label"], this);
		lk_CheckBox_->setObjectName(ls_Key);
		connect(lk_CheckBox_, SIGNAL(toggled(bool)), this, SLOT(toggleOutput(bool)));
		if (lk_OutFile.contains("force"))
		{
			lk_CheckBox_->setChecked((lk_OutFile["force"] == "true" || lk_OutFile["force"] == "yes") ? true : false);
			lk_CheckBox_->setEnabled(false);
		}
		mk_Layout.addWidget(lk_CheckBox_);
	}
	this->updateStatus();
}


k_ScriptBox::~k_ScriptBox() 
{ 
	delete mk_Script_;
}


QList<k_OutputFileBox*> k_ScriptBox::outputFileBoxes()
{
	return mk_OutputFileBoxes.values();
}


void k_ScriptBox::toggleOutput(bool ab_Enabled)
{
	QCheckBox* lk_CheckBox_ = dynamic_cast<QCheckBox*>(sender());
	if (!lk_CheckBox_)
		return;
	
	QString ls_Key = lk_CheckBox_->objectName();
	QHash<QString, QString> lk_OutFile = mk_Script_->outFileDetails(ls_Key);
	if (ab_Enabled)
	{
		k_OutputFileBox* lk_FileBox_ = new k_OutputFileBox(mk_Desktop_, mk_Proteomatic);
		lk_FileBox_->setFilename(lk_OutFile["filename"]);
		mk_Desktop_->addBox(lk_FileBox_, this);
		mk_Desktop_->connectBoxes(this, lk_FileBox_);
		mk_OutputFileBoxes[ls_Key] = lk_FileBox_;
		QString ls_Prefix = mk_PrefixWidget.text();
		lk_FileBox_->setPrefix(ls_Prefix);
		mk_CheckBoxForOutputFileBox[lk_FileBox_] = lk_CheckBox_;
	}
	else
	{
		if (mk_OutputFileBoxes.contains(ls_Key))
		{
			mk_Desktop_->removeBox(mk_OutputFileBoxes[ls_Key]);
			mk_CheckBoxForOutputFileBox.remove(mk_OutputFileBoxes[ls_Key]);
			mk_OutputFileBoxes.remove(ls_Key);
		}
	}
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
	if (!mk_Script_)
		return;

	if (mk_Script_->running())
		return;

	QStringList lk_Arguments;

	foreach (tk_StringStringHash lk_BoxFiles, mk_InputFileBoxes.values())
		lk_Arguments << lk_BoxFiles.keys();

	if (mk_Script_->type() == r_ScriptType::Local)
	{
		QString ls_Result = (dynamic_cast<k_LocalScript*>(mk_Script_))->proposePrefix(lk_Arguments);
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


void k_ScriptBox::fileBoxConnected(k_FileBox* ak_FileBox_)
{
	// this is about input file boxes here
	// ak_FileBox_ may be a k_InputFileBox, a k_InputFileListBox, or a k_OutputFileBox
	k_InputFileBox* lk_InputFileBox_ = dynamic_cast<k_InputFileBox*>(ak_FileBox_);
	k_InputFileListBox* lk_InputFileListBox_ = dynamic_cast<k_InputFileListBox*>(ak_FileBox_);
	k_OutputFileBox* lk_OutputFileBox_ = dynamic_cast<k_OutputFileBox*>(ak_FileBox_);

	// watch the file box for changes
	connect(ak_FileBox_, SIGNAL(changed()), this, SLOT(fileBoxChanged()));
	
	mk_InputFileBoxes[ak_FileBox_] = QHash<QString, QString>();

	// for each file name, determine the input file group
	foreach (QString ls_Filename, ak_FileBox_->fileNames())
	{
		QString ls_Key = mk_Script_->inputKeyForFilename(ls_Filename);
		mk_InputFileBoxes[ak_FileBox_][ls_Filename] = ls_Key;
	}

	this->updateStatus();
}


void k_ScriptBox::fileBoxDisconnected(k_FileBox* ak_FileBox_)
{
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
	
	bool lb_StatusOk = mk_Script_->checkInputFiles(lk_Files);
	
	if (lb_StatusOk)
		mk_StatusLabel.setPixmap(QPixmap(":icons/appointment.png").scaledToWidth(16, Qt::SmoothTransformation));
	else
		mk_StatusLabel.setPixmap(QPixmap(":icons/dialog-warning.png").scaledToWidth(16, Qt::SmoothTransformation));
}


void k_ScriptBox::fileBoxChanged()
{
	k_FileBox* lk_Box_ = dynamic_cast<k_FileBox*>(sender());
	if (!lk_Box_)
		return;
	
	// clear and recreate filenames for this box
	mk_InputFileBoxes[lk_Box_] = QHash<QString, QString>();

	// for each file name, determine the input file group
	foreach (QString ls_Filename, lk_Box_->fileNames())
	{
		QString ls_Key = mk_Script_->inputKeyForFilename(ls_Filename);
		mk_InputFileBoxes[lk_Box_][ls_Filename] = ls_Key;
	}
	this->updateStatus();
}


void k_ScriptBox::showProfileManager()
{
	RefPtr<k_ProfileManager> lk_pProfileManager(new k_ProfileManager(mk_Proteomatic, mk_Script_));
	lk_pProfileManager->reset();
	if (lk_pProfileManager->exec())
		mk_Script_->setConfiguration(lk_pProfileManager->getGoodProfileMix());
}



k_FileBox::k_FileBox(k_Desktop* ak_Parent_, k_Proteomatic& ak_Proteomatic)
	: k_DesktopBox(ak_Parent_, ak_Proteomatic)
{
}


k_FileBox::~k_FileBox()
{
}


k_InputFileBox::k_InputFileBox(k_Desktop* ak_Parent_, k_Proteomatic& ak_Proteomatic)
	: k_FileBox(ak_Parent_, ak_Proteomatic)
	, mk_Label("")
{
	QHBoxLayout* lk_Layout_ = new QHBoxLayout(this);
	lk_Layout_->setContentsMargins(0, 0, 0, 0);
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


QString k_InputFileBox::displayString() const
{
	return QFileInfo(ms_Filename).fileName();
}


bool k_InputFileBox::fileExists()
{
	return QFileInfo(ms_Filename).exists();
}


k_InputFileListBox::k_InputFileListBox(k_Desktop* ak_Parent_, k_Proteomatic& ak_Proteomatic)
	: k_FileBox(ak_Parent_, ak_Proteomatic)
	, mk_FileList(this, true, true)
{
	this->setKeepSmall(false);
	QBoxLayout* lk_MainLayout_ = new QVBoxLayout(this);
	lk_MainLayout_->addWidget(&mk_Label);
	QBoxLayout* lk_HLayout_ = new QHBoxLayout(this);
	lk_MainLayout_->addLayout(lk_HLayout_);
	lk_HLayout_->addWidget(&mk_FileList);
	QBoxLayout* lk_VLayout_ = new QVBoxLayout(this);
	QToolButton* lk_AddFilesButton_ = new QToolButton(this);
	lk_AddFilesButton_->setIcon(QIcon(":/icons/document-open.png"));
	connect(lk_AddFilesButton_, SIGNAL(clicked()), this, SLOT(addFilesButtonClicked()));
	QToolButton* lk_RemoveFilesButton_ = new QToolButton(this);
	lk_RemoveFilesButton_->setIcon(QIcon(":/icons/list-remove.png"));
	connect(lk_RemoveFilesButton_, SIGNAL(clicked()), &mk_FileList, SLOT(removeSelection()));
	connect(lk_RemoveFilesButton_, SIGNAL(clicked()), this, SIGNAL(changed()));
	lk_VLayout_->addWidget(lk_AddFilesButton_);
	lk_VLayout_->addWidget(lk_RemoveFilesButton_);
	lk_VLayout_->addStretch();

	k_ClickableLabel* lk_ArrowLabel_ = new k_ClickableLabel(this);
	lk_ArrowLabel_->setPixmap(QPixmap(":icons/arrow-semi-transparent.png").scaledToWidth(20, Qt::SmoothTransformation));
	//lk_ArrowLabel_->setContentsMargins(0, 0, 0, 0);
	lk_VLayout_->addWidget(lk_ArrowLabel_);
	connect(lk_ArrowLabel_, SIGNAL(pressed()), this, SIGNAL(arrowPressed()));
	connect(lk_ArrowLabel_, SIGNAL(released()), this, SIGNAL(arrowReleased()));
	
	connect(&mk_FileList, SIGNAL(changed()), this, SLOT(updateStatus()));
	connect(&mk_FileList, SIGNAL(changed()), this, SIGNAL(changed()));
	
	lk_HLayout_->addLayout(lk_VLayout_);
	this->resize(250, 150);
	this->setAcceptDrops(true);
	this->updateStatus();
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
	
	mk_Label.setText(QString("<b>File list</b> (%1)").arg(ls_Count));
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


k_OutputFileBox::k_OutputFileBox(k_Desktop* ak_Parent_, k_Proteomatic& ak_Proteomatic)
	: k_InputFileBox(ak_Parent_, ak_Proteomatic)
{
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
