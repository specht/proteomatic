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
	: QWidget(ak_Parent_)
	, mk_Desktop_(ak_Parent_)
	, mk_Background(QColor::fromRgb(255, 255, 255))
	, mk_Border(QColor::fromRgb(128, 128, 128))
	, md_OriginalFontSize(font().pointSizeF())
	, mi_OriginalIconSize(QPushButton().iconSize().width())
	, mk_Layout(this)
	, mk_Proteomatic(ak_Proteomatic)
{
	mk_Layout.getContentsMargins(&mi_OriginalMargin, &mi_OriginalMargin, &mi_OriginalMargin, &mi_OriginalMargin);
}


k_DesktopBox::~k_DesktopBox()
{
}


void k_DesktopBox::scale(double ad_Scale)
{
	hide();
	double ld_NewFontSize = md_OriginalFontSize * ad_Scale;
	QFont lk_Font(font());
	lk_Font.setPointSizeF(ld_NewFontSize);
	setFont(lk_Font);

	int li_NewMargin = (int)((double)mi_OriginalMargin * ad_Scale);
	mk_Layout.setContentsMargins(li_NewMargin, li_NewMargin, li_NewMargin, li_NewMargin);

	int li_NewIconSize = (int)((double)mi_OriginalIconSize * ad_Scale);
	QList<QPushButton*> lk_Buttons = findChildren<QPushButton*>();
	foreach (QPushButton* lk_Button_, lk_Buttons)
		lk_Button_->setIconSize(QSize(li_NewIconSize, li_NewIconSize));

	mk_Layout.invalidate();
	updateGeometry();
	show();
}


void k_DesktopBox::paintEvent(QPaintEvent* ak_Event_)
{
	QPainter lk_Painter(this);
	lk_Painter.fillRect(0, 0, width(), height(), mk_Background);
	lk_Painter.setPen(mk_Border);
	lk_Painter.drawRect(0, 0, width() - 1, height() - 1);
	if (mk_Desktop_->boxSelected(this))
		lk_Painter.drawRect(2, 2, width() - 5, height() - 5);
}


k_ScriptBox::k_ScriptBox(QString as_ScriptName, k_Desktop* ak_Parent_, k_Proteomatic& ak_Proteomatic)
	: k_DesktopBox(ak_Parent_, ak_Proteomatic)
	, mk_Script_(k_ScriptFactory::makeScript(as_ScriptName, ak_Proteomatic, false))
{
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

	QPushButton* lk_ConfigureButton_ = new QPushButton(this);
	lk_ConfigureButton_->setIcon(QIcon(":/icons/preferences-system.png"));
	lk_ButtonLayout_->addWidget(lk_ConfigureButton_);
	connect(lk_ConfigureButton_, SIGNAL(clicked()), mk_Script_->parameterWidget(), SLOT(show()));

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
	show();
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


k_FileBox::k_FileBox(k_Desktop* ak_Parent_, k_Proteomatic& ak_Proteomatic)
	: k_DesktopBox(ak_Parent_, ak_Proteomatic)
	, mk_Label("")
{
	mk_Background = QBrush(QColor::fromRgb(200, 240, 200));
	mk_Border = QPen(QColor::fromRgb(64, 160, 64));
	mk_Layout.addWidget(&mk_Label);
}


k_FileBox::~k_FileBox()
{
}


void k_FileBox::setFilename(const QString& as_Filename)
{
	ms_Filename = as_Filename;
	mk_Label.setText(QFileInfo(ms_Filename).fileName());
}


