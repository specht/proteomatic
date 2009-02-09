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

#include "PipelineMainWindow.h"
#include <QtGui>
#include "Proteomatic.h"


k_PipelineMainWindow::k_PipelineMainWindow(QWidget* ak_Parent_, k_Proteomatic& ak_Proteomatic)
	: QMainWindow(ak_Parent_)
	, mk_Desktop(this, ak_Proteomatic)
	, mk_Proteomatic(ak_Proteomatic)
{
	setWindowIcon(QIcon(":/icons/gpf-48.png"));
	setWindowTitle("Proteomatic Pipeline");
	resize(800, 600);
	setCentralWidget(&mk_Desktop);
	statusBar()->show();

	QToolBar* lk_AddToolBar_ = new QToolBar(this);

	QToolButton* lk_AddScriptButton_ = new QToolButton(lk_AddToolBar_);
	lk_AddScriptButton_->setIcon(QIcon(":/icons/folder.png"));
	lk_AddScriptButton_->setText("Add script");
	lk_AddScriptButton_->setMenu(mk_Proteomatic.proteomaticScriptsMenu());
	lk_AddScriptButton_->setPopupMode(QToolButton::InstantPopup);
	lk_AddScriptButton_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	lk_AddToolBar_->addWidget(lk_AddScriptButton_);
	connect(mk_Proteomatic.proteomaticScriptsMenu(), SIGNAL(triggered(QAction*)), &mk_Desktop, SLOT(addScriptBox(QAction*)));

	QToolButton* lk_StartButton_ = new QToolButton(lk_AddToolBar_);
	lk_StartButton_->setText("Start");
	lk_StartButton_->setIcon(QIcon(":/icons/dialog-ok.png"));
	lk_StartButton_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	lk_AddToolBar_->addWidget(lk_StartButton_);

	lk_AddToolBar_->addSeparator();

	QToolButton* lk_BeautifyButton_ = new QToolButton(lk_AddToolBar_);
	lk_BeautifyButton_->setCheckable(true);
	lk_BeautifyButton_->setChecked(false);
	lk_BeautifyButton_->setText("Self-arrangement");
	lk_BeautifyButton_->setIcon(QIcon(":/icons/face-monkey.png"));
	lk_BeautifyButton_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	lk_AddToolBar_->addWidget(lk_BeautifyButton_);

	addToolBar(Qt::TopToolBarArea, lk_AddToolBar_);

	QToolBar* lk_MouseToolBar_ = new QToolBar(this);

	mk_MouseMoveButton_ = new QToolButton(lk_MouseToolBar_);
	mk_MouseMoveButton_->setIcon(QIcon(":/icons/cursor.png"));
	mk_MouseMoveButton_->setToolButtonStyle(Qt::ToolButtonIconOnly);
	mk_MouseMoveButton_->setCheckable(true);
	mk_MouseMoveButton_->setChecked(true);
	mk_MouseMoveButton_->setAutoExclusive(true);
	lk_MouseToolBar_->addWidget(mk_MouseMoveButton_);
	connect(mk_MouseMoveButton_, SIGNAL(clicked()), this, SLOT(mouseModeButtonClicked()));

	mk_MouseArrowButton_ = new QToolButton(lk_MouseToolBar_);
	mk_MouseArrowButton_->setIcon(QIcon(":/icons/arrow.png"));
	mk_MouseArrowButton_->setToolButtonStyle(Qt::ToolButtonIconOnly);
	mk_MouseArrowButton_->setCheckable(true);
	mk_MouseArrowButton_->setChecked(false);
	mk_MouseArrowButton_->setAutoExclusive(true);
	lk_MouseToolBar_->addWidget(mk_MouseArrowButton_);
	connect(mk_MouseArrowButton_, SIGNAL(clicked()), this, SLOT(mouseModeButtonClicked()));

	lk_MouseToolBar_->addSeparator();

	QToolButton* lk_MagPlusButton_ = new QToolButton(lk_MouseToolBar_);
	lk_MagPlusButton_->setIcon(QIcon(":/icons/viewmag+.png"));
	lk_MagPlusButton_->setToolButtonStyle(Qt::ToolButtonIconOnly);
	lk_MouseToolBar_->addWidget(lk_MagPlusButton_);
	//connect(lk_MagPlusButton_, SIGNAL(clicked()), this, SLOT(mouseModeButtonClicked()));

	QToolButton* lk_MagMinusButton_ = new QToolButton(lk_MouseToolBar_);
	lk_MagMinusButton_->setIcon(QIcon(":/icons/viewmag-.png"));
	lk_MagMinusButton_->setToolButtonStyle(Qt::ToolButtonIconOnly);
	lk_MouseToolBar_->addWidget(lk_MagMinusButton_);
	//connect(lk_MagPlusButton_, SIGNAL(clicked()), this, SLOT(mouseModeButtonClicked()));

	addToolBar(Qt::LeftToolBarArea, lk_MouseToolBar_);

	//mk_Desktop.enableAnimation(true);
	show();
}


k_PipelineMainWindow::~k_PipelineMainWindow()
{
}


void k_PipelineMainWindow::mouseModeButtonClicked()
{
	if (sender() == mk_MouseMoveButton_)
		mk_Desktop.setMouseMode(r_MouseMode::Move);
	else if (sender() == mk_MouseArrowButton_)
		mk_Desktop.setMouseMode(r_MouseMode::Arrow);
}
