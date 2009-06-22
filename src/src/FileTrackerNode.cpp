/*
Copyright (c) 2007-2008 Thaddäus Slawicki

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

#include "FileTrackerNode.h"

k_FileTrackerNode::k_FileTrackerNode()
{	
	setAttribute(Qt::WA_OpaquePaintEvent, true);	
	//

/*	
	QPainter lk_Painter(this);
	QPen lk_Pen(TANGO_ALUMINIUM_3);
	float lf_PenWidth = 1.5;
	lk_Pen.setWidthF(lf_PenWidth);
	lk_Painter.setPen(lk_Pen);
	QBrush lk_Brush(TANGO_ALUMINIUM_0);
	lk_Painter.setBrush(lk_Brush);
	lk_Painter.drawRoundedRect(QRectF(lf_PenWidth * 0.5, lf_PenWidth * 0.5, (qreal)width() - lf_PenWidth, (qreal)height() - lf_PenWidth), 8.0, 8.0);
	*/

	lk_NameLabel = new QLabel(this);
	lk_RunUserLabel = new QLabel(this);
	lk_SizeLabel = new QLabel(this);
	lk_FurtherInfoLabel = new QLabel(this);
	
	//Horizontal Label
	QBoxLayout* lk_HLayout_ = new QHBoxLayout();
	lk_HLayout_->addWidget(lk_NameLabel);
	//lk_MainLabelLayout->addWidget(lk_InsertVerticalLine1);
	lk_HLayout_->addWidget(lk_RunUserLabel);
	//lk_MainLabelLayout->addWidget(lk_InsertVerticalLine2);
	lk_HLayout_->addWidget(lk_SizeLabel);
	
	//grouping Horizontal Label
	
	//Main Label
	QBoxLayout* lk_VLayout_ = new QVBoxLayout(this);
	lk_VLayout_->addLayout(lk_HLayout_);
	//lk_MainFurtherInfoLabel->addWidget(lk_InsertHorizontalLine);
	lk_VLayout_->addWidget(lk_FurtherInfoLabel);
	
	//Grouping Main Label
	
	lk_NameLabel->setText("Hallo");
	lk_RunUserLabel->setText("es");
	lk_SizeLabel->setText("funzt");
	lk_FurtherInfoLabel->setText("nicht?");
	
	
	
}
k_FileTrackerNode::~k_FileTrackerNode()
{
}


void k_FileTrackerNode::paintEvent(QPaintEvent* event)
{
	QPainter lk_Painter(this);
	QPen lk_Pen(TANGO_ALUMINIUM_3);
	float lf_PenWidth = 1.5;
	lk_Pen.setWidthF(lf_PenWidth);
	lk_Painter.setPen(lk_Pen);
	QBrush lk_Brush(TANGO_ALUMINIUM_0);
	lk_Painter.setBrush(lk_Brush);
	lk_Painter.drawRoundedRect(QRectF(lf_PenWidth * 0.5, lf_PenWidth * 0.5, (qreal)width() - lf_PenWidth, (qreal)height() - lf_PenWidth), 8.0, 8.0);
	 
}


