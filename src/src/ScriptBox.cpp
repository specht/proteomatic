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
#include "ScriptFactory.h"
#include "Tango.h"
#include "UnclickableLabel.h"


k_ScriptBox::k_ScriptBox(const QString& as_ScriptUri, k_Desktop* ak_Parent_, k_Proteomatic& ak_Proteomatic)
	: k_DesktopBox(ak_Parent_, ak_Proteomatic)
	, mk_Proteomatic(ak_Proteomatic) // GCC wants this, although it already happens one line above
	, mk_pScript(k_ScriptFactory::makeScript(as_ScriptUri, ak_Proteomatic, false, false))
{
	setupLayout();
}


k_ScriptBox::~k_ScriptBox()
{
	// remove mk_pScript's parameter widget from parameter widget proxy
	mk_pScript->parameterWidget()->setParent(NULL);
}


IScript* k_ScriptBox::script()
{
	return mk_pScript.get_Pointer();
}


void k_ScriptBox::handleIncomingBoxesChanged()
{
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
	lk_HLayout_ = new QHBoxLayout(this);
	lk_VLayout_->addLayout(lk_HLayout_);
	
	QToolButton* lk_ParametersToolButton_ = new QToolButton();
	lk_ParametersToolButton_->setIcon(QIcon(":/icons/preferences-system.png"));
	lk_HLayout_->addWidget(lk_ParametersToolButton_);
	connect(lk_ParametersToolButton_, SIGNAL(clicked()), mk_pParameterProxyWidget.get_Pointer(), SLOT(show()));
	lk_HLayout_->addStretch();
	
	// horizontal rule
	lk_Frame_ = new QFrame(this);
	lk_Frame_->setFrameStyle(QFrame::HLine | QFrame::Plain);
	lk_Frame_->setLineWidth(1);
	lk_Frame_->setStyleSheet("color: " + QString(TANGO_ALUMINIUM_3) + ";");
	lk_VLayout_->addWidget(lk_Frame_);
	
	// output file checkboxes
	foreach (QString ls_Key, mk_pScript->outputFileKeys())
	{
		QHash<QString, QString> lk_OutputFileDetails = mk_pScript->outputFileDetails(ls_Key);
		QCheckBox* lk_CheckBox_ = new QCheckBox(lk_OutputFileDetails["label"]);
		lk_VLayout_->addWidget(lk_CheckBox_);
	}
}
