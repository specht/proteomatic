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
#include "Tango.h"


k_FileTrackerNode::k_FileTrackerNode()
	: mf_HorizontalAlignment(0.0)
	, mf_VerticalAlignment(0.0)
	, mk_Position(0.0, 0.0)
{	
	setAttribute(Qt::WA_OpaquePaintEvent, true);	
	QBoxLayout* lk_Layout_ = new QHBoxLayout(this);

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
	mk_LabelWidgets.clear();
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


void k_FileTrackerNode::setLabels(QStringList ak_Labels)
{
	QLayout* lk_Layout_ = layout();
		
	mk_LabelWidgets.clear();
	for (int i = 0; i < ak_Labels.size(); ++i)
	{
		if (i > 0)
		{
			QFrame* lk_Separator_ = new QFrame(this);
			lk_Separator_->setFrameStyle(QFrame::VLine | QFrame::Plain);
			lk_Separator_->setLineWidth(1);
			lk_Separator_->setStyleSheet("color: " + QString(TANGO_ALUMINIUM_3) + ";");
			lk_Layout_->addWidget(lk_Separator_);
			mk_LabelWidgets.append(RefPtr<QWidget>(lk_Separator_));
		}
		QString ls_Text = ak_Labels[i];
		QLabel* lk_Label_ = new QLabel(ls_Text, this);
		lk_Layout_->addWidget(lk_Label_);
		mk_LabelWidgets.append(RefPtr<QWidget>(lk_Label_));
	}
	resize(1, 1);
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
