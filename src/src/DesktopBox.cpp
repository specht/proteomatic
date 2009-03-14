/*
Copyright (c) 2007-2008 Michael Specht

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

#include "DesktopBox.h"
#include "Desktop.h"
#include "Tango.h"


k_DesktopBox::k_DesktopBox(k_Desktop* ak_Parent_, k_Proteomatic& ak_Proteomatic)
	: QWidget(NULL)
	, mk_Desktop_(ak_Parent_)
	, mk_Proteomatic(ak_Proteomatic)
	, mb_BatchMode(false)
{
}


k_DesktopBox::~k_DesktopBox()
{
}


bool k_DesktopBox::batchMode() const
{
	return mb_BatchMode;
}


QList<IDesktopBox*> k_DesktopBox::incomingBoxes() const
{
	QSet<IDesktopBox*> lk_Set = this->mk_ConnectedIncomingBoxes;
	return lk_Set.toList();
}


QList<IDesktopBox*> k_DesktopBox::outgoingBoxes() const
{
	QSet<IDesktopBox*> lk_Set = this->mk_ConnectedOutgoingBoxes;
	return lk_Set.toList();
}


void k_DesktopBox::setBatchMode(bool ab_Enabled)
{
	bool lb_OldBatchMode = mb_BatchMode;
	mb_BatchMode = ab_Enabled;
	if (lb_OldBatchMode != mb_BatchMode)
		emit batchModeChanged(mb_BatchMode);
}


void k_DesktopBox::connectIncomingBox(IDesktopBox* ak_Other_)
{
	mk_ConnectedIncomingBoxes.insert(ak_Other_);
}


void k_DesktopBox::connectOutgoingBox(IDesktopBox* ak_Other_)
{
	mk_ConnectedOutgoingBoxes.insert(ak_Other_);
}


void k_DesktopBox::disconnectBox(IDesktopBox* ak_Other_)
{
	mk_ConnectedIncomingBoxes.remove(ak_Other_);
	mk_ConnectedOutgoingBoxes.remove(ak_Other_);
}


void k_DesktopBox::paintEvent(QPaintEvent* /*event*/)
{
	//if (mb_KeepSmall)
		this->resize(1, 1);
	
	QPainter lk_Painter(this);
	QBrush lk_Brush(TANGO_ALUMINIUM_0);
	lk_Painter.fillRect(0, 0, width(), height(), lk_Brush);
	QPen lk_Pen(TANGO_ALUMINIUM_3);
	float lf_PenWidth = 1.0;
	/*
	if (mk_Desktop_->boxSelected(this))
		lf_PenWidth = 2.5;
	*/
	lk_Pen.setWidthF(lf_PenWidth);
	lk_Painter.setPen(lk_Pen);
	lk_Painter.drawRect(QRectF(lf_PenWidth * 0.5, lf_PenWidth * 0.5, (qreal)width() - lf_PenWidth, (qreal)height() - lf_PenWidth));

	/*
	if (mb_SpecialFrame)
	{
		lf_PenWidth = 1.0;
		lk_Pen.setWidthF(lf_PenWidth);
		lk_Pen.setColor(QColor(TANGO_BUTTER_2));
		lk_Painter.setPen(lk_Pen);
		lk_Painter.drawRect(QRectF(lf_PenWidth * 0.5 + 1.0, lf_PenWidth * 0.5 + 1.0, (qreal)width() - lf_PenWidth - 2.0, (qreal)height() - lf_PenWidth - 2.0));
	}
	*/
}
