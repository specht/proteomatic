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

#pragma once
#include <QtGui>
#include <QtSql>
#include "RefPtr.h"
#include "FileTrackerNode.h"


class k_RevelioMainWindow;


struct r_NodeType
{
	enum Enumeration
	{
		Run,
		File
	};
};


struct r_NodeInfo
{
	r_NodeInfo()
		: mb_IsGood(false)
		, me_Type(r_NodeType::File)
		, mi_Id(-1)
	{
	}
	
	bool mb_IsGood;
	r_NodeType::Enumeration me_Type;
	int mi_Id;
	// mi_Id is either a run_id or a filecontent_id
};


class k_Surface: public QGraphicsView
{
	Q_OBJECT
public:
	k_Surface(k_RevelioMainWindow& ak_RevelioMainWindow, QWidget* ak_Parent_ = NULL);
	virtual ~k_Surface();
	
	virtual QGraphicsScene& graphicsScene();
	virtual void adjustNodes();
	virtual bool createConnection();
	virtual void focusFile(QString as_Path, QString as_Md5);

public slots:

signals:
	

protected:
	virtual void resizeEvent(QResizeEvent* event);
	virtual void createNodes();
	virtual void mouseDoubleClickEvent(QMouseEvent* mouseEvent);
	virtual QString listToString(QLinkedList<int> ak_List);
	
	k_RevelioMainWindow& mk_RevelioMainWindow;
	QGraphicsScene mk_GraphicsScene;
	
	float mf_SceneWidth2; 
	float mf_SceneHeight2;
	r_NodeInfo mk_FocusNode;
	
	QList<RefPtr<k_FileTrackerNode> > mk_Nodes;
	QList<k_FileTrackerNode*> mk_LeftNodes;
	QList<k_FileTrackerNode*> mk_RightNodes;
	k_FileTrackerNode* mk_CentralNode_;
	QSqlDatabase mk_Database;
};


