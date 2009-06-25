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
#include "FileTrackerNode.h"

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
/*	mk_pNode = RefPtr<k_FileTrackerNode>(new k_FileTrackerNode());
	mk_GraphicsScene.addWidget(mk_pNode.get_Pointer());
	mk_pNode->setAlignment(0.5, 0.5);
	mk_pNode->setPosition(QPoint(0, 0));
	mk_pNode->setLabels(QStringList() << "hello" << "fellow" << "how are you");
	k_FileTrackerNode* mk_NodeOutput = new k_FileTrackerNode();
	k_FileTrackerNode* mk_NodeGlobal = new k_FileTrackerNode();
	k_FileTrackerNode* mk_NodeInput = new k_FileTrackerNode();
	mk_GraphicsScene.addWidget(mk_NodeOutput);
	mk_NodeOutput->setAlignment(1.0,0.5);
	mk_NodeOutput->setPosition(QPoint(300, 0));
	mk_NodeOutput->setLabels(QStringList() << "Filename" << "Filesize" << "User");
	*/
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
	mk_Nodes.clear();
	mk_CentralNode_ = NULL;
	mk_LeftNodes.clear();
	mk_RightNodes.clear();

	k_FileTrackerNode* lk_Node_ = new k_FileTrackerNode();
	mk_Nodes.append(RefPtr<k_FileTrackerNode>(lk_Node_));
	mk_CentralNode_= lk_Node_;
	
	
	for (int i = 0; i< 4; ++i)
	{
		lk_Node_ = new k_FileTrackerNode();
		mk_Nodes.append(RefPtr<k_FileTrackerNode>(lk_Node_));
		mk_LeftNodes.append(lk_Node_);
	}
	
	for (int j = 0; j< 4; ++j)
	{
		lk_Node_ = new k_FileTrackerNode();
		mk_Nodes.append(RefPtr<k_FileTrackerNode>(lk_Node_));
		mk_RightNodes.append(lk_Node_);
	}
	
	foreach(RefPtr<k_FileTrackerNode> lk_pNode, mk_Nodes)
		mk_GraphicsScene.addWidget(lk_pNode.get_Pointer());
		
	adjustNodes();
 }
 
 void k_Surface::adjustNodes()
 {
 
 }
 
