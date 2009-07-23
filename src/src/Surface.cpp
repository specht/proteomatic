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

#include "Surface.h"
#include "Tango.h"
#include "RevelioMainWindow.h"

#ifdef WIN32
	#define FILE_URL_PREFIX "file:///"
#else
	#define FILE_URL_PREFIX "file://"
#endif

k_Surface::k_Surface(k_RevelioMainWindow& ak_RevelioMainWindow, QWidget* ak_Parent_)
	: QGraphicsView(ak_Parent_)
	, mk_RevelioMainWindow(ak_RevelioMainWindow)
	, mk_GraphicsScene(this)
	, mf_SceneWidth2(1.0)
	, mf_SceneHeight2(1.0)
	, mk_CentralNode_(NULL)

{	
	setRenderHint(QPainter::Antialiasing, true);
	setRenderHint(QPainter::TextAntialiasing, true);
	setRenderHint(QPainter::SmoothPixmapTransform, true);
	setScene(&mk_GraphicsScene);
	setBackgroundBrush(QBrush(QColor("#f8f8f8")));
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	createNodes();
	createConnection();
}


k_Surface::~k_Surface()
{
}


QGraphicsScene& k_Surface::graphicsScene()
{
	return mk_GraphicsScene;
}


void k_Surface::resizeEvent(QResizeEvent* event)
{
	QGraphicsView::resizeEvent(event);
	mf_SceneWidth2 = width() / 2.0;
	mf_SceneHeight2 = height() / 2.0;
	mk_GraphicsScene.setSceneRect(-mf_SceneWidth2, -mf_SceneHeight2, mf_SceneWidth2 * 2, mf_SceneHeight2 * 2);
	centerOn(0.0, 0.0);
	
	adjustNodes();
}


void k_Surface::createNodes()
{	
	if (!mr_FocusNode.mb_IsGood)
		return;

	bool lb_HaveResults = false;
	if (mr_FocusNode.me_Type == r_NodeType::File)
	{
		QSqlQuery ls_FileWithNameQueryCentralNode;
		QString ls_Query = QString("SELECT `filecontent_id` FROM `filewithname` WHERE `filecontent_id` = '%1' LIMIT 1").arg(mr_FocusNode.mi_Id);
		ls_FileWithNameQueryCentralNode.exec(ls_Query);
		if (ls_FileWithNameQueryCentralNode.next())
			lb_HaveResults = true;
	}
	else if (mr_FocusNode.me_Type == r_NodeType::Run)
	{
		QSqlQuery ls_RunsQuery;
		QString ls_RQuery = QString("SELECT `run_id` FROM `runs` WHERE `run_id`='%1' LIMIT 1").arg(mr_FocusNode.mi_Id);
		ls_RunsQuery.exec(ls_RQuery);
		if (ls_RunsQuery.next())
			lb_HaveResults = true;
	}
	
	if (!lb_HaveResults)
	{
		QMessageBox::critical(0, QObject::tr("Entry not found."), mk_Database.lastError().text());
		return;
	}
	
	mk_NodeInfoHash.clear();
	
	// clear all nodes
	mk_Nodes.clear();
	mk_CentralNode_ = NULL;
	mk_LeftNodes.clear();
	mk_RightNodes.clear();

	// create central node
	
	k_FileTrackerNode* lk_Node_ = new k_FileTrackerNode();
	mk_Nodes.append(RefPtr<k_FileTrackerNode>(lk_Node_));
	mk_CentralNode_= lk_Node_;
	mk_NodeInfoHash[mk_GraphicsScene.addWidget(lk_Node_)] = mr_FocusNode;
	
	if (mr_FocusNode.me_Type == r_NodeType::File)
	{
		//Query for CentralNode
		QSqlQuery ls_FileWithNameQueryCentralNode;
		QString ls_Query = QString("SELECT `code_basename`, `filecontent_id` FROM `filewithname` WHERE `filecontent_id` = '%1' LIMIT 1").arg(mr_FocusNode.mi_Id);
		ls_FileWithNameQueryCentralNode.exec(ls_Query);
		ls_FileWithNameQueryCentralNode.next();
		QString ls_CodeBasename = ls_FileWithNameQueryCentralNode.value(0).toString();
		int li_FileContentId = ls_FileWithNameQueryCentralNode.value(1).toInt();
		mr_FocusNode.mi_Id = li_FileContentId;
		mk_CentralNode_->setLabels(QStringList() << ls_CodeBasename);
		
		//Query for all files matching with inputfile
		QSqlQuery ls_FileWithNameQuery;
		QString ls_FWNIdQuery = QString("SELECT `filewithname_id` FROM `filewithname` WHERE `filecontent_id` = '%1'").arg(mr_FocusNode.mi_Id);
		ls_FileWithNameQuery.exec(ls_FWNIdQuery);
		QLinkedList<int> lk_FileWithNameIdList;
		while(ls_FileWithNameQuery.next())
		{
			int li_FileWithNameId	= ls_FileWithNameQuery.value(0).toInt();
			lk_FileWithNameIdList.append(li_FileWithNameId);
		}
		
		if (lk_FileWithNameIdList.size() != 0)
		{
			//searching for runs with file used as inputfile
			QSqlQuery ls_RunWithNameQueryIn;
			QString ls_RWNQuery = QString("SELECT `run_id` FROM `run_filewithname` WHERE `filewithname_id` IN (%1) AND `input_file` = 1").arg(listToString(lk_FileWithNameIdList));
			ls_RunWithNameQueryIn.exec(ls_RWNQuery);
			QLinkedList<int> lk_RunInList;
			while(ls_RunWithNameQueryIn.next())
			{
				int li_RunId = ls_RunWithNameQueryIn.value(0).toInt();
				lk_RunInList.append(li_RunId);
			}
			if (lk_RunInList.size() != 0)
			{
				QSqlQuery ls_RunsInQuery;
				QString ls_RInQuery = QString("SELECT `title`, `run_id` FROM `runs` WHERE `run_id` IN (%1)").arg(listToString(lk_RunInList));
				ls_RunsInQuery.exec(ls_RInQuery);
				QLinkedList<QString> lk_TitleInList;
				while(ls_RunsInQuery.next())
				{
					QString ls_Title = ls_RunsInQuery.value(0).toString();
					int li_RunId = ls_RunsInQuery.value(1).toInt();
					lk_Node_ = new k_FileTrackerNode();
					mk_Nodes.append(RefPtr<k_FileTrackerNode>(lk_Node_));
					mk_RightNodes.append(lk_Node_);
					lk_Node_->setLabels(QStringList() << ls_Title);
					lk_TitleInList.append(ls_Title);
					mk_NodeInfoHash[mk_GraphicsScene.addWidget(lk_Node_)] = r_NodeInfo(r_NodeType::Run, li_RunId);
				}
			}
			
			//searching for runs with file used as outputfile
			QSqlQuery ls_RunWithNameQueryOut;
			ls_RWNQuery = QString("SELECT `run_id` FROM `run_filewithname` WHERE `filewithname_id` IN (%1) AND `input_file` = 0").arg(listToString(lk_FileWithNameIdList));
			ls_RunWithNameQueryOut.exec(ls_RWNQuery);
			QLinkedList<int> lk_RunOutList;
			while(ls_RunWithNameQueryOut.next())
			{
				int li_RunId		= ls_RunWithNameQueryOut.value(0).toInt();
				lk_RunOutList.append(li_RunId);
			}			
			if (lk_RunOutList.size() != 0)
			{
				QSqlQuery ls_RunsOutQuery;
				QString ls_ROutQuery = QString("SELECT `title`, `run_id` FROM `runs` WHERE `run_id` IN (%1)").arg(listToString(lk_RunOutList));
				ls_RunsOutQuery.exec(ls_ROutQuery);
				QLinkedList<QString> lk_TitleOutList;
				while(ls_RunsOutQuery.next())
				{
					QString ls_Title = ls_RunsOutQuery.value(0).toString();
					int li_RunId = ls_RunsOutQuery.value(1).toInt();
					lk_Node_ = new k_FileTrackerNode();
					mk_Nodes.append(RefPtr<k_FileTrackerNode>(lk_Node_));
					mk_LeftNodes.append(lk_Node_);
					lk_Node_->setLabels(QStringList() << ls_Title);
					lk_TitleOutList.append(ls_Title);
					mk_NodeInfoHash[mk_GraphicsScene.addWidget(lk_Node_)] = r_NodeInfo(r_NodeType::Run, li_RunId);
				}
			}
		}
	}
	
	if (mr_FocusNode.me_Type == r_NodeType::Run)
	{
		QSqlQuery ls_RunsQuery;
		QString ls_RQuery = QString("SELECT `title`, `run_id` FROM `runs` WHERE `run_id`='%1'").arg(mr_FocusNode.mi_Id);
		ls_RunsQuery.exec(ls_RQuery);
		ls_RunsQuery.next();
		QString ls_Title		= ls_RunsQuery.value(0).toString();
		int li_RunId			= ls_RunsQuery.value(1).toInt();
		mr_FocusNode.mi_Id = li_RunId;
		mk_CentralNode_->setLabels(QStringList() << ls_Title);
		
		//searching for files used in run as inputfile
		QSqlQuery ls_RunWithNameQueryIn;
		QString ls_RWNQuery = QString("SELECT `filewithname_id` FROM `run_filewithname` WHERE `run_id`='%1' AND `input_file`= 1").arg(mr_FocusNode.mi_Id);
		ls_RunWithNameQueryIn.exec(ls_RWNQuery);
		QLinkedList<int> lk_FileInList;
		while (ls_RunWithNameQueryIn.next())
		{
			int li_FileWithNameId = ls_RunWithNameQueryIn.value(0).toInt();
			lk_FileInList.append(li_FileWithNameId);
		}
		if (lk_FileInList.size() != 0)
		{
			QSqlQuery ls_FilesInQuery;
			QString ls_FInQuery = QString("SELECT `code_basename`, `filewithname_id` FROM `filewithname` WHERE `filewithname_id` IN (%1)").arg(listToString(lk_FileInList));
			ls_FilesInQuery.exec(ls_FInQuery);
			while(ls_FilesInQuery.next())
			{
				QString ls_CodeBasename = ls_FilesInQuery.value(0).toString();
				int li_FileWithNameId = ls_FilesInQuery.value(1).toInt();
				//searching for filecontent_id
				QSqlQuery lk_FilecontentIdQuery;
				QString ls_FilecontentQuery = QString("SELECT `filecontent_id` FROM `filewithname` WHERE `filewithname_id`='%1'").arg(li_FileWithNameId);
				lk_FilecontentIdQuery.exec(ls_FilecontentQuery);
				lk_FilecontentIdQuery.next();
				int li_FileContentId = lk_FilecontentIdQuery.value(0).toInt();
				lk_Node_ = new k_FileTrackerNode();
				mk_Nodes.append(RefPtr<k_FileTrackerNode>(lk_Node_));
				mk_LeftNodes.append(lk_Node_);
				lk_Node_->setLabels(QStringList() << ls_CodeBasename);
				mk_NodeInfoHash[mk_GraphicsScene.addWidget(lk_Node_)] = r_NodeInfo(r_NodeType::File, li_FileContentId);
			}
		}
		//searching for files used in run as outputfile
		QSqlQuery ls_RunWithNameQueryOut;
		ls_RWNQuery = QString("SELECT `filewithname_id` FROM `run_filewithname` WHERE `run_id`='%1' AND `input_file`= 0").arg(mr_FocusNode.mi_Id);
		ls_RunWithNameQueryOut.exec(ls_RWNQuery);
		QLinkedList<int> lk_FileOutList;
		while(ls_RunWithNameQueryOut.next())
		{
			int li_FileWithNameId = ls_RunWithNameQueryOut.value(0).toInt();
			lk_FileOutList.append(li_FileWithNameId);
		}
		if (lk_FileOutList.size() != 0)
		{
			QSqlQuery ls_FilesOutQuery;
			QString ls_FOutQuery = QString("SELECT `code_basename`, `filewithname_id` FROM `filewithname` WHERE `filewithname_id` IN (%1)").arg(listToString(lk_FileOutList));
			ls_FilesOutQuery.exec(ls_FOutQuery);
				
			while(ls_FilesOutQuery.next())
			{
				QString ls_CodeBasename = ls_FilesOutQuery.value(0).toString();
				int li_FileWithNameId = ls_FilesOutQuery.value(1).toInt();
				//searching for filecontent_id
				QSqlQuery lk_FilecontentIdQuery;
				QString ls_FilecontentQuery = QString("SELECT `filecontent_id` FROM `filewithname` WHERE `filewithname_id`='%1'").arg(li_FileWithNameId);
				lk_FilecontentIdQuery.exec(ls_FilecontentQuery);
				lk_FilecontentIdQuery.next();
				int li_FileContentId = lk_FilecontentIdQuery.value(0).toInt();;
				lk_Node_ = new k_FileTrackerNode();
				mk_Nodes.append(RefPtr<k_FileTrackerNode>(lk_Node_));
				mk_RightNodes.append(lk_Node_);
				lk_Node_->setLabels(QStringList() << ls_CodeBasename);
				mk_NodeInfoHash[mk_GraphicsScene.addWidget(lk_Node_)] = r_NodeInfo(r_NodeType::File, li_FileContentId);
			}
		}
	}
/*	
	for (int i = 0; i< 2; ++i)
	{
		lk_Node_ = new k_FileTrackerNode();
		mk_Nodes.append(RefPtr<k_FileTrackerNode>(lk_Node_));
		mk_LeftNodes.append(lk_Node_);
	}
	
	for (int j = 0; j< 7; ++j)
	{
		lk_Node_ = new k_FileTrackerNode();
		mk_Nodes.append(RefPtr<k_FileTrackerNode>(lk_Node_));
		mk_RightNodes.append(lk_Node_);
		
	}
	
*/
	// insert nodes into graphics scene
	/*
	foreach(RefPtr<k_FileTrackerNode> lk_pNode, mk_Nodes)
		mk_GraphicsScene.addWidget(lk_pNode.get_Pointer());
	*/
	
	adjustNodes();
	updateInfoPane(mr_FocusNode);
}
/*
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
*/	
void k_Surface::updateInfoPane(r_NodeInfo ar_NodeInfo)
{
	if (mk_RevelioMainWindow.paneScrollArea().widget())
		delete mk_RevelioMainWindow.paneScrollArea().takeWidget();
	
	QWidget* lk_PaneWidget_ = new QWidget(NULL);
	QBoxLayout* lk_VLayout_ = new QVBoxLayout(lk_PaneWidget_);
	
	if (ar_NodeInfo.me_Type == r_NodeType::File)
	{
		//Query for FileInformation
		QSqlQuery ls_FileInformation;
		QString ls_Query = QString("SELECT `code_basename`,`directory`,`ctime`,`mtime`\
									FROM `filewithname` WHERE `filecontent_id` = '%1'").arg(ar_NodeInfo.mi_Id);
		ls_FileInformation.exec(ls_Query);
		
		//QTableWidget mit file name, directory, creation time, modification time
		QTableWidget* lk_FilenameTable_ = new QTableWidget(lk_PaneWidget_);
		lk_FilenameTable_->setColumnCount(4);
		lk_FilenameTable_->setRowCount(1);
		lk_FilenameTable_->setHorizontalHeaderLabels(QStringList() << "Title" << "Directory" << "Create Time" << "Modification Time");
		QTableWidgetItem lk_CodeBasenameItem;
		//QTableWidgetItem lk_DirectoryItem = new QTableWidgetItem(ls_Directory);
		//QTableWidgetItem lk_CTimeItem = new QTableWidgetItem(ls_CTime);
		//QTableWidgetItem lk_MTimeItem = new QTableWidgetItem(ls_MTime);
		lk_VLayout_->addWidget(lk_FilenameTable_);
		while (ls_FileInformation.next())
		{
			QString ls_CodeBasename 			= ls_FileInformation.value(0).toString();
			QString ls_Directory 				= ls_FileInformation.value(1).toString();
			QString ls_CTime					= ls_FileInformation.value(2).toString();
			QString ls_MTime					= ls_FileInformation.value(3).toString();
			
			lk_CodeBasenameItem.setText(ls_CodeBasename);
			
		}
		lk_FilenameTable_->setItem(1, 1, &lk_CodeBasenameItem);
		//QTableWidgetItem* lk_CodeBasenameItem = new QTableWidgetItem(QString("blabla"));
		//lk_FilenameTable_->setItem(1, 1, lk_CodeBasenameItem);
		//lk_FilenameTable_->setItem(1, 2, lk_DirectoryItem);
		//lk_FilenameTable_->setItem(1, 3, lk_CTimeItem);
		//lk_FilenameTable_->setItem(1, 4, lk_MTimeItem);

	} 
	else if (ar_NodeInfo.me_Type == r_NodeType::Run)
	{
		//Label for Parameters
		QLabel* lk_ParamLabel_ = new QLabel(lk_PaneWidget_);
		QLabel* lk_RunsInformation_ = new QLabel(lk_PaneWidget_);
		QGroupBox* lk_PramaGroup_ = new QGroupBox(lk_PaneWidget_);
		QBoxLayout* lk_VLayoutParam_ = new QVBoxLayout(lk_PramaGroup_);
		lk_VLayoutParam_->addWidget(lk_ParamLabel_);
		lk_VLayoutParam_->addWidget(lk_RunsInformation_);
		//standardout Information
		QTextEdit* lk_StdoutTextEdit_ = new QTextEdit(lk_PaneWidget_);
		lk_StdoutTextEdit_->setReadOnly(true);
		//lk_StdoutTextEdit_->setCurrentFont(mk_ConsoleFont);
		lk_StdoutTextEdit_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		lk_StdoutTextEdit_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
		lk_StdoutTextEdit_->setText("Further Information");
		
		QSqlQuery ls_RunsInformation;
		QString ls_RQuery = QString("SELECT `user`,`title`,`host`,`script_uri`,`version`,`start_time`,`end_time`\
									FROM `runs` WHERE `run_id` = '%1'").arg(ar_NodeInfo.mi_Id);
		ls_RunsInformation.exec(ls_RQuery);
		ls_RunsInformation.next();
		QString ls_User			= ls_RunsInformation.value(0).toString();
		QString ls_Title		= ls_RunsInformation.value(1).toString();
		QString ls_Host			= ls_RunsInformation.value(2).toString();
		QString ls_ScriptUri	= ls_RunsInformation.value(3).toString();
		int li_Version			= ls_RunsInformation.value(4).toInt();
		QString ls_StartTime	= ls_RunsInformation.value(5).toString();
		QString ls_EndTime		= ls_RunsInformation.value(6).toString();
		QString ls_RunInformation = QString("<b>User:</b> %1 <br> <b>Title:</b> %2 <br> <b>Host:</b> %3 <br> <b>ScriptUri:</b> %4 <br> <b>Version:</b> %5 <br> <b>Start Time:</b> %6 <br> <b>End Time:</b> %7 <br>").arg(ls_User).arg(ls_Title).arg(ls_Host).arg(ls_ScriptUri).arg(li_Version).arg(ls_StartTime).arg(ls_EndTime);

		lk_VLayout_->addWidget(new QLabel(QString("<b>%1</b>").arg(ls_Title), lk_PaneWidget_));
		lk_RunsInformation_->setText(ls_RunInformation);
		QSqlQuery ls_ParamQuery;
		QString ls_PQuery = QString("SELECT `code_key`,`code_value`\
									FROM `parameters` WHERE `run_id` ='%1'").arg(mr_FocusNode.mi_Id);
		ls_ParamQuery.exec(ls_PQuery);
		QString ls_ParameterInformation;
		while(ls_ParamQuery.next())
		{	
			QString ls_CodeKey		= ls_ParamQuery.value(0).toString();
			QString ls_CodeValue	= ls_ParamQuery.value(1).toString();
			QString ls_ParamString = QString("%1: %2<br />").arg(ls_CodeKey).arg(ls_CodeValue);
			ls_ParameterInformation.append(ls_ParamString);
		}
		lk_ParamLabel_->setText(ls_ParameterInformation);
		
		lk_VLayout_->addWidget(lk_PramaGroup_);
		lk_VLayout_->addWidget(lk_StdoutTextEdit_);
	}
	mk_RevelioMainWindow.paneScrollArea().setWidget(lk_PaneWidget_);	
}


void k_Surface::adjustNodes()
{	
	float lf_NodeSpacing = 50.0;
	if (mk_CentralNode_)
	{
		lf_NodeSpacing = mk_CentralNode_->height() + 20.0;
		mk_CentralNode_->setPosition(QPointF(0.0, 0.0));
		mk_CentralNode_->setAlignment(0.5, 0.5);
	}
	
	float lf_xLeft = -(mf_SceneWidth2 / 3.0);
	float lf_xRight = (mf_SceneWidth2 / 3.0);
		
	float y = -(float)(mk_LeftNodes.size() - 1) * lf_NodeSpacing * 0.5;
	for	(int i = 0; i < mk_LeftNodes.size(); ++i)
	{

		mk_LeftNodes[i]->setPosition(QPointF(lf_xLeft, y));
		y += lf_NodeSpacing;
		mk_LeftNodes[i]->setAlignment(1.0 , 0.5);
	};

	y = -(float)(mk_RightNodes.size() - 1) * lf_NodeSpacing * 0.5;
	for (int i = 0; i < mk_RightNodes.size(); ++i)
	{
		mk_RightNodes[i]->setPosition(QPointF(lf_xRight, y));
		y += lf_NodeSpacing;
		mk_RightNodes[i]->setAlignment(0.0 , 0.5);
	}
}

void k_Surface::mouseDoubleClickEvent(QMouseEvent* mouseEvent)
{	
	QGraphicsItem* lk_Item_ = NULL;
	if (lk_Item_ = itemAt(mouseEvent->pos()))
	{
		mr_FocusNode = mk_NodeInfoHash[lk_Item_];
		createNodes();
	}
}

void k_Surface::mousePressEvent(QMouseEvent* clickEvent)
{
	QGraphicsItem* lk_Item_ = NULL;
	if (lk_Item_ = itemAt(clickEvent->pos()))
	{
		mr_FocusNode = mk_NodeInfoHash[lk_Item_];
		//updateInfoPane(r_NodeInfo ar_NodeInfo);
	}
}

QString k_Surface::listToString(QLinkedList<int> ak_List)
{
	QStringList lk_List;
	foreach (int i, ak_List)
		lk_List << QString("%1").arg(i);
	return lk_List.join(",");
}


bool k_Surface::createConnection()
{

	mk_Database = QSqlDatabase::addDatabase("QMYSQL");
	mk_Database.setHostName("peaks.uni-muenster.de");
	mk_Database.setDatabaseName("filetracker");
	mk_Database.setUserName("testuser");
	mk_Database.setPassword("user");
	mk_Database.setPort(3306);

	
	if (!mk_Database.open())
	{
		QMessageBox::critical(0, QObject::tr("Database Error"), mk_Database.lastError().text());
		return false;
	}
	return true;
}


void k_Surface::focusFile(QString as_Path, QString as_Md5)
{	
	QSqlQuery ls_FilecontentQuery;
	QString ls_Query = QString("SELECT `filecontent_id`\
								FROM `filecontents` WHERE `identifier` = 'md5%1' AND `size` = '%2' LIMIT 1").arg(as_Md5).arg(QFileInfo(as_Path).size());
	ls_FilecontentQuery.exec(ls_Query);
	if (ls_FilecontentQuery.size() != 1)
	{
		QString ls_Query = QString("SELECT `filecontent_id`\
									FROM `filecontents` WHERE `identifier` = 'basename%1' AND `size` = '%2' LIMIT 1").arg(QFileInfo(as_Path).fileName()).arg(QFileInfo(as_Path).size());
		ls_FilecontentQuery.exec(ls_Query);
	}
	
	if (ls_FilecontentQuery.size() == 1)
	{
		ls_FilecontentQuery.next();
		int li_FileContentId = ls_FilecontentQuery.value(0).toInt();
		mr_FocusNode.me_Type = r_NodeType::File;
		mr_FocusNode.mi_Id = li_FileContentId;
		mr_FocusNode.mb_IsGood = true;
		createNodes();
	}
	else
	{
		QString ls_title("Critical");
		QString ls_text("File not found in database!");
		QMessageBox lk_msgBox(QMessageBox::Critical, ls_title ,ls_text, QMessageBox::Ok);
		lk_msgBox.exec();
	}

}

/*
QFont& k_Surface::consoleFont()
{
	return mk_ConsoleFont;
}
*/
