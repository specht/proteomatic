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
#include "FileTrackerNode.h"


class k_RevelioMainWindow;
class k_Proteomatic;


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
    
    r_NodeInfo(r_NodeType::Enumeration ae_Type, int ai_Id)
        : mb_IsGood(true)
        , me_Type(ae_Type)
        , mi_Id(ai_Id)
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
    k_Surface(k_RevelioMainWindow& ak_RevelioMainWindow, k_Proteomatic& ak_Proteomatic, QWidget* ak_Parent_ = NULL);
    virtual ~k_Surface();
    
    virtual QGraphicsScene& graphicsScene();
    virtual void adjustNodes();
    virtual bool createConnection();
    virtual void focusFile(QString as_Path);
    QFont& consoleFont();
    
public slots:

signals:
    

protected:
    virtual void resizeEvent(QResizeEvent* event);
    virtual void createNodes();
    virtual void updateInfoPane(r_NodeInfo ar_NodeInfo);
    virtual void mouseDoubleClickEvent(QMouseEvent* mouseEvent);
    virtual void mousePressEvent(QMouseEvent* clickEvent);
    virtual void wheelEvent(QWheelEvent* wheelEvent);
    virtual void dragEnterEvent(QDragEnterEvent* event);
    virtual void dragMoveEvent(QDragMoveEvent* event);
    virtual void dropEvent(QDropEvent* event);
    virtual QString listToString(QLinkedList<int> ak_List);
    virtual void updateNodesMaxWidth();
    virtual double curveXForY(double y) const;
    
    k_RevelioMainWindow& mk_RevelioMainWindow;
    k_Proteomatic& mk_Proteomatic;
    QGraphicsScene mk_GraphicsScene;
    
    float mf_SceneWidth2; 
    float mf_SceneHeight2;
    r_NodeInfo mr_FocusNode;
    double md_LeftScrollOffset;
    double md_RightScrollOffset;
    QGraphicsPathItem* mk_ScrollLinesPathItem_;
    
    QList<QSharedPointer<k_FileTrackerNode> > mk_Nodes;
    QList<k_FileTrackerNode*> mk_LeftNodes;
    QList<k_FileTrackerNode*> mk_RightNodes;
    k_FileTrackerNode* mk_CentralNode_;
    
    QSqlDatabase mk_Database;
    QHash<QGraphicsItem*, r_NodeInfo> mk_NodeInfoHash;
    QPainterPath mk_CurvePath;
    double md_NodeHeight;
};
