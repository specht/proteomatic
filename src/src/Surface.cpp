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
	createNodes();
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
	
	for (int j = 0; j< 7; ++j)
	{
		lk_Node_ = new k_FileTrackerNode();
		mk_Nodes.append(RefPtr<k_FileTrackerNode>(lk_Node_));
		mk_RightNodes.append(lk_Node_);
		
	}
	/*
	foreach(RefPtr<k_FileTrackerNode> lk_pNode, mk_Nodes)
		mk_GraphicsScene.addWidget(lk_pNode.get_Pointer());
		lk_Node_->setLabels(QStringList() << "hello" << "fellow" << "how are you");
		lk_Node_->setAlignment(0.5, 0.5);
		lk_Node_->setPosition(QPoint(0, 0));
	*/
	adjustNodes();
 }
 
void k_Surface::adjustNodes()
{	
	float lf_NodeHeight = (float)height();
	float lf_NodePosition = lf_NodeHeight + 100.0;
	
	float lf_xLeft = -(mf_SceneWidth2 / 3.0);
	float lf_xRight = (mf_SceneWidth2 / 3.0);
		
	mk_GraphicsScene.addWidget(mk_CentralNode_);
	mk_CentralNode_->setLabels(QStringList() << "No" << "Name" << "Set");
	mk_CentralNode_->setAlignment(0.5, 0.5);
	mk_CentralNode_->setPosition(QPoint(0, 0));

	
	for	(int i = 0; i < mk_LeftNodes.size(); ++i)
	{
	float lf_y = -(((i - 1)* lf_NodePosition) / 2);
	mk_GraphicsScene.addWidget(mk_LeftNodes[i]);
	mk_LeftNodes[i]->setAlignment(1.0 , 0.5);
	mk_LeftNodes[i]->setPosition(QPointF(lf_xLeft, lf_y));
	};
	
	for (int j = 0; j < mk_RightNodes.size(); ++j)
	{
	float lf_y = -(((j - 1)* lf_NodePosition) / 2);
	mk_GraphicsScene.addWidget(mk_RightNodes[j]);
	mk_RightNodes[j]->setAlignment(0.0 , 0.5);
	mk_RightNodes[j]->setPosition(QPointF(lf_xRight,lf_y));
	};
}
 
