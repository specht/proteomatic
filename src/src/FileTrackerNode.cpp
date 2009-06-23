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

#include "FileTrackerNode.h"

k_FileTrackerNode::k_FileTrackerNode()
	: mk_NameLabel_(new QLabel(this))
	, mk_RunUserLabel_(new QLabel(this))
	, mk_SizeLabel_(new QLabel(this))
	, mk_FurtherInfoLabel_(new QLabel(this))
	, mf_HorizontalAlignment(0.0)
	, mf_VerticalAlignment(0.0)
	, mk_Position(0.0, 0.0)
{	
	setAttribute(Qt::WA_OpaquePaintEvent, true);	
	
	//Horizontal Label
	QBoxLayout* lk_HLayout_ = new QHBoxLayout();
	lk_HLayout_->addWidget(mk_NameLabel_);
	//lk_MainLabelLayout->addWidget(lk_InsertVerticalLine1);
	lk_HLayout_->addWidget(mk_RunUserLabel_);
	//lk_MainLabelLayout->addWidget(lk_InsertVerticalLine2);
	lk_HLayout_->addWidget(mk_SizeLabel_);
	
	//Main Label
	QBoxLayout* lk_VLayout_ = new QVBoxLayout(this);
	lk_VLayout_->addLayout(lk_HLayout_);
	//lk_MainFurtherInfoLabel->addWidget(lk_InsertHorizontalLine);
	lk_VLayout_->addWidget(mk_FurtherInfoLabel_);
	
	mk_NameLabel_->setText("mk_Position");
	mk_RunUserLabel_->setText("es");
	mk_SizeLabel_->setText("funzt");
	mk_FurtherInfoLabel_->setText("nicht?");
	
	/*
	QSpinBox* lk_SpinHorizontal = new QSpinBox;
	QSpinBox* lk_SpinVertical = new QSpinBox;
	lk_SpinHorizontal->setMinimum(0);
	lk_SpinHorizontal->setMaximum(100);
	lk_SpinVertical->setMinimum(0);
	lk_SpinVertical->setMaximum(100);
	*/
	
}


k_FileTrackerNode::~k_FileTrackerNode()
{
}


float k_FileTrackerNode::horizontalAlignment() const
{
	return mf_HorizontalAlignment;
}


float k_FileTrackerNode::verticalAlignment() const
{
	return mf_VerticalAlignment;
}
	

const QPointF k_FileTrackerNode::position() const
{
	return mk_Position;
}


void k_FileTrackerNode::setHorizontalAlignment(float af_HorizontalAlignment)
{	
	//this->mf_HorizontalAlignment = af_HorizontalAlignment;
	
	mf_HorizontalAlignment = qBound(0.0f, af_HorizontalAlignment, 1.0f);	
	
	adjustPosition();
}


void k_FileTrackerNode::setVerticalAlignment(float af_VerticalAlignment)
{
	
	mf_VerticalAlignment = qBound(0.0f, af_VerticalAlignment, 1.0f);
	
	adjustPosition();
	
}


void k_FileTrackerNode::setAlignment(float af_HorizontalAlignment, float af_VerticalAlignment)
{	
	mf_HorizontalAlignment = qBound(0.0f, af_HorizontalAlignment, 1.0f);
	mf_VerticalAlignment = qBound(0.0f, af_VerticalAlignment, 1.0f);
	
	adjustPosition();
}


void k_FileTrackerNode::setPosition(const QPointF ak_Position)
{
	mk_Position = ak_Position;
	adjustPosition();
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


void k_FileTrackerNode::resizeEvent(QResizeEvent* event)
{	
	QWidget::resizeEvent(event);
	adjustPosition();
}


void k_FileTrackerNode::adjustPosition()
{
	float x = (float)width() * mf_HorizontalAlignment;
	float y = (float)height() * mf_VerticalAlignment;
	
	
	move((mk_Position - QPointF(x, y)).toPoint());
	// take mf_HorizontalAlignment, mf_VerticalAlignment and mk_Position, width() and height()
	// determine correct position and move there
}
