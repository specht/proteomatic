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
	if (!mk_FocusNode.mb_IsGood)
		return;
	
	// clear all nodes
	mk_Nodes.clear();
	mk_CentralNode_ = NULL;
	mk_LeftNodes.clear();
	mk_RightNodes.clear();

	// if (mk_FocusNode.me_Type == r_NodeType::File)
		// `filewithname`
		// SELECT filewithname_id FROM `filewithname` WHERE filecontent_id = 'x' (Liste!)
		// (ex. 2, 7, 189, 509)
		
		// `run_filewithname`
		// SELECT `run_id`, `input_file` FROM `run_filewithname` WHERE filewithname_id = '2' OR  ...
		// run_id list: 1, 4, 10, 11, 1,2 40
		// run titles are: 
		// Filter by mass accuracy
		// Write HTML report
		// SimQuant
		// Compare PSM lists (you want some SimQuant with tha...
		// Compare PSM lists (you want some SimQuant with tha...
		// Write HTML report

	// create central node
	
	k_FileTrackerNode* lk_Node_ = new k_FileTrackerNode();
	mk_Nodes.append(RefPtr<k_FileTrackerNode>(lk_Node_));
	mk_CentralNode_= lk_Node_;
	
	if (mk_FocusNode.me_Type == r_NodeType::File)
	{
		QSqlQuery ls_FileWithNameQuery;
		QString ls_Query = QString("SELECT `filewithname_id`,`code_basename`,`directory`,`ctime`,`mtime`\
									FROM `filewithname` WHERE `filecontent_id` = '%1'").arg(mk_FocusNode.mi_Id);
		ls_FileWithNameQuery.exec(ls_Query);
		
		while(ls_FileWithNameQuery.next())
		{
			int li_FileWithNameId 	= ls_FileWithNameQuery.value(0).toInt();
			QString ls_CodeBasename = ls_FileWithNameQuery.value(1).toString();
			QString ls_Directory 	= ls_FileWithNameQuery.value(2).toString();
			QTime li_CTime			= ls_FileWithNameQuery.value(3).toTime();
			QTime li_MTime			= ls_FileWithNameQuery.value(4).toTime();
		}
		
		mk_CentralNode_->setLabels(QStringList() << ls_CodeBasename);
	}
	
	if (mk_FocusNode.me_Type == r_NodeType::Run)
	{
		QSqlQuery ls_RunsQuery;
		QString ls_RQuery = QString("SELECT `run_id`,`user`,`title`,`host`,`script_uri`,`version`,`start_time`,`end_time`\
									FROM `runs` WHERE 1");
		ls_RunsQuery.exec(ls_RQuery);
		
		ls_RunsQuery.next();
		int li_RunId			= ls_RunsQuery.value(0).toInt();
		QString ls_User			= ls_RunsQuery.value(1).toString();
		QString ls_Title		= ls_RunsQuery.value(2).toString();
		QString ls_Host			= ls_RunsQuery.value(3).toString();
		QString ls_ScriptUri	= ls_RunsQuery.value(4).toString();
		int li_Version			= ls_RunsQuery.value(5).toInt();
		QTime li_StartTime		= ls_RunsQuery.value(6).toTime();
		QTime li_EndTime		= ls_RunsQuery.value(7).toTime();
		
		mk_FocusNode.mi_Id = li_RunId;
		mk_FocusNode.mb_IsGood = true;
		
		QSqlQuery ls_ParamQuery;
		QString ls_PQuery = QString("SELECT `code_key`,`code_value`\
									FROM `parameters` WHERE `run_id` ='%1'").arg(mk_FocusNode.mi_Id);
		ls_ParamQuery.exec(ls_PQuery);
		
		while(ls_ParamQuery.next())
		{	
			QString ls_CodeKey		= ls_ParamQuery.value(0).toString();
			QString ls_CodeValue	= ls_ParamQuery.value(1).toString();
		}	
		
		//QSqlQuery ls_FileInRunQuery;
		//QString ls_FIRQuery  = QString("").arg();
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
	
	foreach(RefPtr<k_FileTrackerNode> lk_pNode, mk_Nodes)
	{
		mk_GraphicsScene.addWidget(lk_pNode.get_Pointer());
		lk_pNode->setLabels(QStringList() << "hello" << "fellow" << "how are you");
	}
*/
	adjustNodes();
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

	if (itemAt(mouseEvent->pos()))
	{
		if (pos() != QPoint(0,0))
		{
			QString title("Information");
			QString text("Move Node to centre!");
		
			QMessageBox msgBox(QMessageBox::Information, title ,text, QMessageBox::Ok);
			msgBox.exec();
		}
	}

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
								FROM `filecontents` WHERE `identifier` = 'md5%1' AND `size` = '%2'LIMIT 1").arg(as_Md5).arg(QFileInfo(as_Path).size());
	ls_FilecontentQuery.exec(ls_Query);
	if (ls_FilecontentQuery.size() != 1)
	{
		QString ls_Query = QString("SELECT `filecontent_id`\
									FROM `filecontents` WHERE `identifier` = 'basename%1' AND `size` = '%2'LIMIT 1").arg(QFileInfo(as_Path).fileName()).arg(QFileInfo(as_Path).size());
		ls_FilecontentQuery.exec(ls_Query);
	}
	
	if (ls_FilecontentQuery.size() == 1)
	{
		ls_FilecontentQuery.next()
		int li_FileContentId = ls_FilecontentQuery.value(0).toInt();
		mk_FocusNode.me_Type = r_NodeType::File;
		mk_FocusNode.mi_Id = li_FileContentId;
		mk_FocusNode.mb_IsGood = true;
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


