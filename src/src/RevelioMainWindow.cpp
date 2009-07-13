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

#ifdef WIN32
	#define FILE_URL_PREFIX "file:///"
#else
	#define FILE_URL_PREFIX "file://"
#endif

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
	
	
	QFontDatabase lk_FontDatabase;
	QStringList lk_Fonts = QStringList() << "Consolas" << "Bitstream Vera Sans Mono" << "Lucida Console" << "Liberation Mono" << "Courier New" << "Courier" << "Fixed" << "System";
	while (!lk_Fonts.empty())
	{
		QString ls_Font = lk_Fonts.takeFirst();
		if (lk_FontDatabase.families().contains(ls_Font))
		{
			mk_ConsoleFont = QFont(ls_Font);
			mk_ConsoleFont.setPointSizeF(mk_ConsoleFont.pointSizeF() * 0.8);
#ifdef WIN32			
			mk_ConsoleFont.setPointSizeF(mk_ConsoleFont.pointSizeF() * 0.9);
#endif
			break;
		}
	}
	
	
	
	QWidget* lk_MainWidget_ = new QWidget(this);
	setCentralWidget(lk_MainWidget_);
	
	mk_ParamLabel.setText("Hier werden demn�chst Parameter erscheinen");
	
	QBoxLayout* lk_HLayoutParam_ = new QHBoxLayout(this);
	lk_HLayoutParam_->addWidget(&mk_ParamLabel);
	
	QGroupBox* lk_ParamGroup_ = new QGroupBox("Parameter");
	lk_ParamGroup_->setLayout(lk_HLayoutParam_);
	
	mk_StdoutTextEdit.setReadOnly(true);
	mk_StdoutTextEdit.setCurrentFont(mk_ConsoleFont);
	mk_StdoutTextEdit.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	//mk_StdoutTextEdit.setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	mk_StdoutTextEdit.setText("Und hier sollen weitere Infos rein?!");
	//mk_StdoutTextEdit.resize(320, 240);
	
	QBoxLayout* lk_VLayoutInfo_ = new QVBoxLayout(this);
	lk_VLayoutInfo_->addWidget(lk_ParamGroup_);
	lk_VLayoutInfo_->addWidget(&mk_StdoutTextEdit);
	
	//QGroupBox* lk_InfoGruoup_ = new QGroupBox(this);
	//lk_InfoGruoup_->setLayout(lk_VLayoutInfo_);
	
	QBoxLayout* lk_HLayout_ = new QHBoxLayout(this);
	lk_HLayout_->addWidget(&mk_Surface);
	lk_HLayout_->addLayout(lk_VLayoutInfo_); //(lk_InfoGruoup_);
	
	//QGroupBox* lk_MainGruoup_ = new QGroupBox(this);
	//lk_MainGruoup_->setLayout(lk_HLayout_);
	
	QBoxLayout* lk_VLayout_ = new QVBoxLayout(lk_MainWidget_);
	lk_VLayout_->addWidget(lk_LoadFileButton_);
	lk_VLayout_->addLayout(lk_HLayout_);
	//lk_VLayout_->addWidget(lk_MainGruoup_);
	//lk_VLayout_->addWidget(&mk_Surface);
	lk_VLayout_->addWidget(&mk_HashLabel);
	//mk_Surface_ = new k_Surface(this);
	//mk_pSurface = RefPtr<k_Surface>(new k_Surface(this));

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
		mk_HashLabel.setText(ls_Md5);
		mk_Surface.focusFile(ls_Path, ls_Md5);
		//mk_Surface.mk_pNode->setLabels(QStringList() << QFileInfo(ls_Path).completeBaseName() << ls_SaveString);
	}
}

void k_RevelioMainWindow::adjustLayout()
{
	mk_Surface.adjustNodes();
}

QFont& k_RevelioMainWindow::consoleFont()
{
	return mk_ConsoleFont;
}
/*void k_RevelioMainWindow::showParam()
{

}
*/

