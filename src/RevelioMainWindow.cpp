/*
Copyright (c) 2007-2008 Thaddaeus Slawicki

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

#include "RevelioMainWindow.h"
#include "Proteomatic.h"


k_RevelioMainWindow::k_RevelioMainWindow(k_Proteomatic& ak_Proteomatic, QWidget* ak_Parent_)
    : mk_Proteomatic(ak_Proteomatic)
    , mk_Surface(*this, ak_Proteomatic, this)
{
    mk_Proteomatic.setMessageBoxParent(this);
    resize(800, 480);
    setWindowTitle("Revelio");
    setWindowIcon(QIcon(":/icons/revelio.png"));
    
    QToolButton* lk_LoadFileButton_ = new QToolButton(this);
    lk_LoadFileButton_->setText("Load file");
    lk_LoadFileButton_->setIcon(QIcon(":/icons/document-open.png"));
    lk_LoadFileButton_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    connect(lk_LoadFileButton_, SIGNAL(pressed()), this, SLOT(loadFile()));
    
    QWidget* lk_MainWidget_ = new QWidget(this);
    setCentralWidget(lk_MainWidget_);
    
    //mk_ParamLabel.setText("Parameters will be shown!");
    
    //QBoxLayout* lk_HLayoutParam_ = new QHBoxLayout(this);
    //lk_HLayoutParam_->addWidget(&mk_ParamLabel);
    
    //QGroupBox* lk_ParamGroup_ = new QGroupBox("Parameters");
    //lk_ParamGroup_->setLayout(lk_HLayoutParam_);
    
    //mk_StdoutTextEdit.setReadOnly(true);
    //mk_StdoutTextEdit.setCurrentFont(mk_ConsoleFont);
    //mk_StdoutTextEdit.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    //mk_StdoutTextEdit.setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    //mk_StdoutTextEdit.setText("Further Information");

    QSplitter* lk_Splitter = new QSplitter(this);
    lk_Splitter->setChildrenCollapsible(false);
    lk_Splitter->setOrientation(Qt::Horizontal);
    lk_Splitter->addWidget(&mk_Surface);
    lk_Splitter->addWidget(&mk_PaneScrollArea);
    lk_Splitter->setSizes(QList<int>() << width() * 3 / 4 << width() * 1 / 4);
    mk_PaneScrollArea.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    
    //QBoxLayout* lk_VLayoutInfo_ = new QVBoxLayout(this);
    //lk_VLayoutInfo_->addWidget(lk_ParamGroup_);
    //lk_VLayoutInfo_->addWidget(&mk_StdoutTextEdit);
    
    //QBoxLayout* lk_HLayout_ = new QHBoxLayout(this);
    //lk_HLayout_->addWidget(&mk_Surface);
    //lk_HLayout_->addLayout(lk_VLayoutInfo_); //(lk_InfoGruoup_);
    
    QBoxLayout* lk_VLayout_ = new QVBoxLayout(lk_MainWidget_);
    lk_VLayout_->addWidget(lk_LoadFileButton_);
    //lk_VLayout_->addLayout(lk_HLayout_);
    //lk_VLayout_->addWidget(&mk_HashLabel);
    //mk_Surface_ = new k_Surface(this);
    //mk_pSurface = QSharedPointer<k_Surface>(new k_Surface(this));
    lk_VLayout_->addWidget(lk_Splitter);
    //lk_VLayout_->addWidget(&mk_PaneScrollArea);
    
    QStatusBar* lk_StatusBar_ = new QStatusBar(this);
    setStatusBar(lk_StatusBar_);
}


k_RevelioMainWindow::~k_RevelioMainWindow()
{
}


void k_RevelioMainWindow::loadFile()
{
    QString ls_Path = QFileDialog::getOpenFileName(this, "Load file");
    if (!ls_Path.isEmpty())
        mk_Surface.focusFile(ls_Path);
}


void k_RevelioMainWindow::adjustLayout()
{
    mk_Surface.adjustNodes();
}


QScrollArea& k_RevelioMainWindow::paneScrollArea()
{
    return mk_PaneScrollArea;
}
