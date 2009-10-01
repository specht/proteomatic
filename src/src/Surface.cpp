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
#include "Proteomatic.h"
#include "RevelioMainWindow.h"

#ifdef WIN32
	#define FILE_URL_PREFIX "file:///"
#else
	#define FILE_URL_PREFIX "file://"
#endif

k_Surface::k_Surface(k_RevelioMainWindow& ak_RevelioMainWindow, 
					  k_Proteomatic& ak_Proteomatic, 
					  QWidget* ak_Parent_)
	: QGraphicsView(ak_Parent_)
	, mk_RevelioMainWindow(ak_RevelioMainWindow)
	, mk_Proteomatic(ak_Proteomatic)
	, mk_GraphicsScene(this)
	, mf_SceneWidth2(1.0)
	, mf_SceneHeight2(1.0)
	, mk_CentralNode_(NULL)
	, md_LeftScrollOffset(0.0)
	, md_RightScrollOffset(0.0)
	, md_NodeHeight(40.0)
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
	mk_ScrollLinesPathItem_ = mk_GraphicsScene.addPath(QPainterPath(), QPen(TANGO_ALUMINIUM_2));
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
	
	mk_CurvePath = QPainterPath();
	float d = mf_SceneWidth2 * 2.0 * 0.15;
	float d2 = mf_SceneWidth2 * 2.0 * 0.2;
	mk_CurvePath.moveTo(d - pow(mf_SceneHeight2 / 60.0, 2.0), -mf_SceneHeight2);
	mk_CurvePath.quadTo(d2, 0.0, d - pow(mf_SceneHeight2 / 60.0, 2.0), mf_SceneHeight2);
	
	QPainterPath lk_Path;
	lk_Path.addPath(mk_CurvePath);
	lk_Path.addPath(mk_CurvePath * QMatrix().scale(-1.0, 1.0));
	
	mk_ScrollLinesPathItem_->setPath(lk_Path);
	
	updateNodesMaxWidth();
	adjustNodes();
}


void k_Surface::createNodes()
{	
	if (!mr_FocusNode.mb_IsGood)
		return;
	
	md_LeftScrollOffset = 0.0;
	md_RightScrollOffset = 0.0;

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
				QString ls_RInQuery = QString("SELECT `title`, `run_id`, `start_time` FROM `runs` WHERE `run_id` IN (%1) ORDER BY `start_time`").arg(listToString(lk_RunInList));
				ls_RunsInQuery.exec(ls_RInQuery);
				QLinkedList<QString> lk_TitleInList;
				while(ls_RunsInQuery.next())
				{
					QString ls_Title = ls_RunsInQuery.value(0).toString();
					int li_RunId = ls_RunsInQuery.value(1).toInt();
					QString ls_Time = ls_RunsInQuery.value(2).toDateTime().toString(Qt::SystemLocaleShortDate);
					lk_Node_ = new k_FileTrackerNode();
					mk_Nodes.append(RefPtr<k_FileTrackerNode>(lk_Node_));
					mk_RightNodes.append(lk_Node_);
					lk_Node_->setLabels(QStringList() << ls_Title << ls_Time);
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
				int li_RunId = ls_RunWithNameQueryOut.value(0).toInt();
				lk_RunOutList.append(li_RunId);
			}			
			if (lk_RunOutList.size() != 0)
			{
				QSqlQuery ls_RunsOutQuery;
				QString ls_ROutQuery = QString("SELECT `title`, `run_id`, `start_time` FROM `runs` WHERE `run_id` IN (%1) ORDER BY `start_time`").arg(listToString(lk_RunOutList));
				ls_RunsOutQuery.exec(ls_ROutQuery);
				QLinkedList<QString> lk_TitleOutList;
				while(ls_RunsOutQuery.next())
				{
					QString ls_Title = ls_RunsOutQuery.value(0).toString();
					int li_RunId = ls_RunsOutQuery.value(1).toInt();
					QString ls_Time = ls_RunsOutQuery.value(2).toDateTime().toString(Qt::SystemLocaleShortDate);
					lk_Node_ = new k_FileTrackerNode();
					mk_Nodes.append(RefPtr<k_FileTrackerNode>(lk_Node_));
					mk_LeftNodes.append(lk_Node_);
					lk_Node_->setLabels(QStringList() << ls_Title << ls_Time);
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
	
	md_LeftScrollOffset = (mk_LeftNodes.size() - 1) * 0.5;
	md_RightScrollOffset = (mk_RightNodes.size() - 1) * 0.5;

	foreach (k_FileTrackerNode* lk_Node_, mk_LeftNodes)
		lk_Node_->setAlignment(1.0 , 0.5);
	foreach (k_FileTrackerNode* lk_Node_, mk_RightNodes)
		lk_Node_->setAlignment(0.0 , 0.5);
	if (mk_CentralNode_)
		mk_CentralNode_->setAlignment(0.5, 0.5);
	
	foreach (RefPtr<k_FileTrackerNode> lk_pNode, mk_Nodes)
	{
		k_FileTrackerNode* lk_Node_ = lk_pNode.get_Pointer();
		if (lk_Node_ == mk_CentralNode_)
			lk_Node_->setFrameColor(mr_FocusNode.me_Type == r_NodeType::File? TANGO_ALUMINIUM_3 : TANGO_SKY_BLUE_2);
		else
			lk_Node_->setFrameColor(mr_FocusNode.me_Type == r_NodeType::File? TANGO_SKY_BLUE_2 : TANGO_ALUMINIUM_3);
	}
	
	updateNodesMaxWidth();
	adjustNodes();
	updateInfoPane(mr_FocusNode);
}


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
		QLabel* lk_FilenameLabel_ = new QLabel(lk_PaneWidget_);
		lk_FilenameLabel_->setWordWrap(true);
		lk_VLayout_->addWidget(lk_FilenameLabel_);
		QString ls_FilenameLabelString;
		ls_FilenameLabelString += "<b>Seen at:</b><br /><ul>";
		while (ls_FileInformation.next())
			ls_FilenameLabelString += QString("<li>%1/%2</li>").arg(ls_FileInformation.value(1).toString()).arg(ls_FileInformation.value(0).toString());
		ls_FilenameLabelString += "</ul>";
		lk_FilenameLabel_->setText(ls_FilenameLabelString);
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
		lk_StdoutTextEdit_->setCurrentFont(mk_Proteomatic.consoleFont());
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
									FROM `parameters` WHERE `run_id` ='%1'").arg(ar_NodeInfo.mi_Id);
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
	float lf_NodeSpacing = md_NodeHeight + 4.0;
	if (mk_CentralNode_)
		mk_CentralNode_->setPosition(QPointF(0.0, 0.0));
/*		printf("central node height is %d.\n", mk_CentralNode_->height());
		lf_NodeSpacing = mk_CentralNode_->height() + 8.0;*/
	
	float y = -md_LeftScrollOffset * lf_NodeSpacing;
	for	(int i = 0; i < mk_LeftNodes.size(); ++i)
	{
		if (fabs(y) <= mf_SceneHeight2 + lf_NodeSpacing)
		{
			mk_LeftNodes[i]->setPosition(QPointF(-12.0 - curveXForY(y), y));
			mk_LeftNodes[i]->show();
		}
		else
			mk_LeftNodes[i]->hide();
			
		y += lf_NodeSpacing;
	};

	y = -md_RightScrollOffset * lf_NodeSpacing;
	for (int i = 0; i < mk_RightNodes.size(); ++i)
	{
		if (fabs(y) <= mf_SceneHeight2 + lf_NodeSpacing)
		{
			mk_RightNodes[i]->setPosition(QPointF(12.0 + curveXForY(y), y));
			mk_RightNodes[i]->show();
		}
		else
			mk_RightNodes[i]->hide();
		
		y += lf_NodeSpacing;
	}
}


void k_Surface::mouseDoubleClickEvent(QMouseEvent* mouseEvent)
{	
	QGraphicsItem* lk_Item_ = NULL;
	if ((lk_Item_ = itemAt(mouseEvent->pos())) != NULL)
	{
		mr_FocusNode = mk_NodeInfoHash[lk_Item_];
		createNodes();
	}
}


void k_Surface::mousePressEvent(QMouseEvent* clickEvent)
{
	QGraphicsItem* lk_Item_ = NULL;
	if ((lk_Item_ = itemAt(clickEvent->pos())) != NULL)
	{
		updateInfoPane(mk_NodeInfoHash[lk_Item_]);
	}
}


void k_Surface::wheelEvent(QWheelEvent* wheelEvent)
{
	bool lb_LeftSide = (float)wheelEvent->x() < mf_SceneWidth2;
	if (fabs((float)wheelEvent->x() - mf_SceneWidth2) > curveXForY(wheelEvent->x() - mf_SceneHeight2))
	{
		if (lb_LeftSide)
		{
			md_LeftScrollOffset -= (double)wheelEvent->delta() / 20.0 / 8.0;
			md_LeftScrollOffset = qBound<double>(0.0, md_LeftScrollOffset, (double)(mk_LeftNodes.size() - 1));
		}
		else
		{
			md_RightScrollOffset -= (double)wheelEvent->delta() / 20.0 / 8.0;
			md_RightScrollOffset = qBound<double>(0.0, md_RightScrollOffset, (double)(mk_RightNodes.size() - 1));
		}
	}
	adjustNodes();
}


void k_Surface::dragEnterEvent(QDragEnterEvent* event)
{
	event->acceptProposedAction();
}


void k_Surface::dragMoveEvent(QDragMoveEvent* event)
{
	event->acceptProposedAction();
}


void k_Surface::dropEvent(QDropEvent* event)
{
	event->accept();
	QList<QUrl> lk_Urls = event->mimeData()->urls();
	if (!lk_Urls.empty())
	{
		QString ls_Path = lk_Urls.first().toLocalFile();
		if (!ls_Path.isEmpty())
			if (QFileInfo(ls_Path).isFile())
				focusFile(ls_Path);
	}
}


QString k_Surface::listToString(QLinkedList<int> ak_List)
{
	QStringList lk_List;
	foreach (int i, ak_List)
		lk_List << QString("%1").arg(i);
	return lk_List.join(",");
}


void k_Surface::updateNodesMaxWidth()
{
	if (mk_CentralNode_)
		mk_CentralNode_->setMaximumWidth(curveXForY(0.0) * 2.0 - 16.0);
	foreach(k_FileTrackerNode* lk_Node_, mk_LeftNodes)
		lk_Node_->setMaximumWidth(mf_SceneWidth2 - curveXForY(0.0) - 16.0);
	foreach(k_FileTrackerNode* lk_Node_, mk_RightNodes)
		lk_Node_->setMaximumWidth(mf_SceneWidth2 - curveXForY(0.0) - 16.0);
}


double k_Surface::curveXForY(double y) const
{
	double d = qBound<double>(0.0, (y + mf_SceneHeight2) / (mf_SceneHeight2 * 2.0), 1.0);
	return mk_CurvePath.pointAtPercent(d).x();
}


bool k_Surface::createConnection()
{

	mk_Database = QSqlDatabase::addDatabase("QMYSQL");
	mk_Database.setHostName("localhost");
	mk_Database.setDatabaseName("filetracker");
	mk_Database.setUserName("test");
	mk_Database.setPassword("test");
	mk_Database.setPort(3306);

	
	if (!mk_Database.open())
	{
		QMessageBox::critical(0, QObject::tr("Database Error"), mk_Database.lastError().text());
		return false;
	}
	return true;
}


void k_Surface::focusFile(QString as_Path)
{	
	QString ls_Md5 = mk_Proteomatic.md5ForFile(as_Path);
	QSqlQuery ls_FilecontentQuery;
	QString ls_Query = QString("SELECT `filecontent_id`\
								FROM `filecontents` WHERE `identifier` = 'md5%1' AND `size` = '%2' LIMIT 1").arg(ls_Md5).arg(QFileInfo(as_Path).size());
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
