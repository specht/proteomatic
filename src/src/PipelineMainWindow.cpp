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
	setWindowIcon(QIcon(":/icons/proteomatic.png"));
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
	connect(lk_StartButton_, SIGNAL(clicked()), this, SLOT(start()));

	lk_AddToolBar_->addSeparator();

	QToolButton* lk_AddFileListButton_ = new QToolButton(lk_AddToolBar_);
	lk_AddFileListButton_->setText("Add file list");
	lk_AddFileListButton_->setIcon(QIcon(":/icons/document-open.png"));
	lk_AddFileListButton_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	lk_AddToolBar_->addWidget(lk_AddFileListButton_);
	connect(lk_AddFileListButton_, SIGNAL(clicked()), this, SLOT(addFileListBox()));

	lk_AddToolBar_->addSeparator();

	addToolBar(Qt::TopToolBarArea, lk_AddToolBar_);

	show();
}


k_PipelineMainWindow::~k_PipelineMainWindow()
{
}


void k_PipelineMainWindow::start()
{
	
}


void k_PipelineMainWindow::addFileListBox()
{
	k_FileListBox* lk_FileListBox_ = new k_FileListBox(&mk_Desktop, mk_Proteomatic);
	mk_Desktop.addBox(lk_FileListBox_);
}
