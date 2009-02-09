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

#include "Desktop.h"
#include "DesktopBox.h"
#include "ScriptFactory.h"


k_DesktopBox::k_DesktopBox(k_Desktop* ak_Parent_, k_Proteomatic& ak_Proteomatic)
	: QFrame(NULL)
	, mk_Desktop_(ak_Parent_)
	, mk_Background(QColor::fromRgb(255, 255, 255))
	, mk_Border(QColor::fromRgb(128, 128, 128))
	, mk_Proteomatic(ak_Proteomatic)
	, mb_Moving(false)
{
	this->setFrameShadow(QFrame::Raised);
	this->setFrameShape(QFrame::StyledPanel);
}


k_DesktopBox::~k_DesktopBox()
{
}


void k_DesktopBox::paintEvent(QPaintEvent* ak_Event_)
{
	this->setLineWidth(mk_Desktop_->boxSelected(this) ? 2 : 1);
	QFrame::paintEvent(ak_Event_);
	return;
	/*
	QPainter lk_Painter(this);
	lk_Painter.fillRect(0, 0, width(), height(), mk_Background);
	lk_Painter.setPen(mk_Border);
	lk_Painter.drawRect(0, 0, width() - 1, height() - 1);
	if (mk_Desktop_->boxSelected(this))
		lk_Painter.drawRect(2, 2, width() - 5, height() - 5);
	*/
	
}


void k_DesktopBox::mousePressEvent(QMouseEvent* ak_Event_)
{
	mb_Moving = true;
	mk_OldPosition = this->pos();
	mk_OldMousePosition = ak_Event_->globalPos();
}


void k_DesktopBox::mouseReleaseEvent(QMouseEvent* ak_Event_)
{
	mb_Moving = false;
}


void k_DesktopBox::mouseMoveEvent(QMouseEvent* ak_Event_)
{
	if (mb_Moving)
	{
		this->move(mk_OldPosition + ak_Event_->globalPos() - mk_OldMousePosition);
		emit moved();
	}
}


k_ScriptBox::k_ScriptBox(QString as_ScriptName, k_Desktop* ak_Parent_, k_Proteomatic& ak_Proteomatic)
	: k_DesktopBox(ak_Parent_, ak_Proteomatic)
	, mk_Script_(k_ScriptFactory::makeScript(as_ScriptName, ak_Proteomatic, false))
{
	this->setLayout(&mk_Layout);
	mk_Background = QBrush(QColor::fromRgb(255, 255, 255));
	mk_Border = QPen(QColor::fromRgb(128, 128, 128));
	
    QLabel* lk_Label_ = new QLabel(mk_Script_->title(), this);
    QFont lk_Font(lk_Label_->font());
    lk_Font.setBold(true);
    lk_Label_->setFont(lk_Font);
	mk_Layout.addWidget(lk_Label_);
    
	QFrame* lk_Frame_ = new QFrame(this);
    lk_Frame_->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    mk_Layout.addWidget(lk_Frame_);

	QHBoxLayout* lk_ButtonLayout_ = new QHBoxLayout();
	
	QScrollArea* lk_ScrollArea_ = new QScrollArea();
	lk_ScrollArea_->setWidget(mk_Script_->parameterWidget());
	lk_ScrollArea_->setWidgetResizable(true);
	lk_ScrollArea_->resize(400, 600);
	lk_ScrollArea_->setWindowTitle(mk_Script_->title());
	lk_ScrollArea_->setWindowIcon(QIcon(":icons/proteomatic.png"));
	
	mk_pParameterWidget = RefPtr<QWidget>(lk_ScrollArea_);

	QPushButton* lk_ConfigureButton_ = new QPushButton(this);
	lk_ConfigureButton_->setIcon(QIcon(":/icons/preferences-system.png"));
	lk_ButtonLayout_->addWidget(lk_ConfigureButton_);
	connect(lk_ConfigureButton_, SIGNAL(clicked()), this, SLOT(showParameterWidget()));

	QPushButton* lk_ShowOutputButton_ = new QPushButton(this);
	lk_ShowOutputButton_->setIcon(QIcon(":/icons/utilities-terminal.png"));
	lk_ButtonLayout_->addWidget(lk_ShowOutputButton_);
	//connect(lk_ShowOutputButton_, SIGNAL(clicked()), mk_Script.parameterWidget(), SLOT(show()));

/*
	QPushButton* lk_ShowInfoButton_ = new QPushButton(this);
	lk_ShowInfoButton_->setIcon(QIcon(":/icons/info.png"));
	lk_ButtonLayout_->addWidget(lk_ShowInfoButton_);
	*/
	//connect(lk_ShowInfoButton_, SIGNAL(clicked()), mk_Script.parameterWidget(), SLOT(show()));

	lk_ButtonLayout_->addStretch();

	mk_Layout.addLayout(lk_ButtonLayout_);

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
}


k_ScriptBox::~k_ScriptBox() 
{ 
	delete mk_Script_;
}


void k_ScriptBox::toggleOutput(bool ab_Enabled)
{
	QCheckBox* lk_CheckBox_ = dynamic_cast<QCheckBox*>(sender());
	QString ls_Key = lk_CheckBox_->objectName();
	QHash<QString, QString> lk_OutFile = mk_Script_->outFileDetails(ls_Key);
	if (ab_Enabled)
	{
		k_FileBox* lk_FileBox_ = new k_FileBox(mk_Desktop_, mk_Proteomatic);
		lk_FileBox_->setFilename(lk_OutFile["filename"]);
		mk_Desktop_->addBox(lk_FileBox_, this);
		mk_Desktop_->connectBoxes(this, lk_FileBox_);
		mk_OutputFileBoxes[ls_Key] = lk_FileBox_;
	}
	else
	{
		if (mk_OutputFileBoxes.contains(ls_Key))
			mk_Desktop_->removeBox(mk_OutputFileBoxes[ls_Key]);
	}
}


void k_ScriptBox::showParameterWidget()
{
	mk_pParameterWidget->show();
}


k_FileBox::k_FileBox(k_Desktop* ak_Parent_, k_Proteomatic& ak_Proteomatic)
	: k_DesktopBox(ak_Parent_, ak_Proteomatic)
	, mk_Label("")
{
	this->setLayout(&mk_Layout);
	mk_Layout.setContentsMargins(0, 0, 0, 0);
	mk_Background = QBrush(QColor::fromRgb(200, 240, 200));
	mk_Border = QPen(QColor::fromRgb(64, 160, 64));
	mk_Background = QBrush(QColor("#8ae234"));
	mk_Border = QPen(QColor("#4e9a06"));
	mk_Layout.addWidget(&mk_Label);
	mk_Label.setContentsMargins(8, 8, 8, 8);
	QPushButton* lk_ArrowLabel_ = new QPushButton(this);
	lk_ArrowLabel_->setFlat(true);
	lk_ArrowLabel_->setIcon(QIcon(":icons/arrow.png"));
	lk_ArrowLabel_->setIconSize(QSize(20, 20));
	mk_Layout.addWidget(lk_ArrowLabel_);
	connect(lk_ArrowLabel_, SIGNAL(pressed()), this, SIGNAL(arrowPressed()));
	connect(lk_ArrowLabel_, SIGNAL(released()), this, SIGNAL(arrowReleased()));
}


k_FileBox::~k_FileBox()
{
}


void k_FileBox::setFilename(const QString& as_Filename)
{
	ms_Filename = as_Filename;
	mk_Label.setText(QFileInfo(ms_Filename).fileName());
}
