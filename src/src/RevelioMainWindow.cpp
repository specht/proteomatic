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
#include <md5.h>



k_RevelioMainWindow::k_RevelioMainWindow(QWidget* ak_Parent_)
	: mk_Surface(*this, this)
{
	resize(640, 480);
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
	//mk_pSurface = RefPtr<k_Surface>(new k_Surface(this));
	lk_VLayout_->addWidget(lk_Splitter);
	//lk_VLayout_->addWidget(&mk_PaneScrollArea);
}


k_RevelioMainWindow::~k_RevelioMainWindow()
{
}

QString k_RevelioMainWindow::md5ForFile(QString as_Path)
{	
	
	QFile lk_File(as_Path);
	lk_File.open(QIODevice::ReadOnly);
	// calculate MD5 of file content
	md5_state_s lk_Md5State;
	md5_init(&lk_Md5State);
	while (!lk_File.atEnd())
	{
		QByteArray lk_Bytes = lk_File.read(8 * 1024 * 1024);
		md5_append(&lk_Md5State, (md5_byte_t*)lk_Bytes.constData(), lk_Bytes.size());
	}
	lk_File.close();
	unsigned char lk_Md5[16];
	md5_finish(&lk_Md5State, (md5_byte_t*)(&lk_Md5));
	QString ls_HashString;
	for (int i = 0; i < 16; ++i)
		ls_HashString.append(QString("%1").arg(lk_Md5[i], 2, 16, QChar('0')));
	return ls_HashString;
	//	printf("%02x", lk_Md5[i]);
	//printf("\n");
}

void k_RevelioMainWindow::loadFile()
{
	QString ls_Path = QFileDialog::getOpenFileName(this, "Load file");
	if (!ls_Path.isEmpty())
	{
		QString ls_Md5 = md5ForFile(ls_Path);
		//mk_HashLabel.setText(ls_Md5);
		mk_Surface.focusFile(ls_Path, ls_Md5);
		//mk_Surface.mk_pNode->setLabels(QStringList() << QFileInfo(ls_Path).completeBaseName() << ls_SaveString);
	}
}

void k_RevelioMainWindow::adjustLayout()
{
	mk_Surface.adjustNodes();
}




QScrollArea& k_RevelioMainWindow::paneScrollArea()
{
	return mk_PaneScrollArea;
}
