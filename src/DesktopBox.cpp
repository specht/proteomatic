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
#include "RefPtr.h"


k_DesktopBox::k_DesktopBox(k_Desktop* ak_Parent_, k_Proteomatic& ak_Proteomatic, 
						   bool ab_ResizableX, bool ab_ResizableY)
	: QWidget(NULL)
	, mk_Desktop_(ak_Parent_)
	, mk_Proteomatic(ak_Proteomatic)
	, mb_ResizableX(ab_ResizableX)
	, mb_ResizableY(ab_ResizableY)
	, mk_ResizeGripPixmap(QPixmap(":icons/size-grip.png"))
	, mb_BatchMode(false)
	, mb_ProtectedFromUserDeletion(false)
	, mb_Moving(false)
	, mb_Resizing(false)
{
	setAttribute(Qt::WA_OpaquePaintEvent, true);
	setFocusPolicy(Qt::StrongFocus);
	setContentsMargins(0, 0, 0, 0);
}


k_DesktopBox::~k_DesktopBox()
{
}


bool k_DesktopBox::batchMode() const
{
	return mb_BatchMode;
}


bool k_DesktopBox::protectedFromUserDeletion() const
{
	return mb_ProtectedFromUserDeletion;
}


void k_DesktopBox::setProtectedFromUserDeletion(bool ab_Flag)
{
	mb_ProtectedFromUserDeletion = ab_Flag;
}


QSet<IDesktopBox*> k_DesktopBox::incomingBoxes() const
{
	return mk_ConnectedIncomingBoxes;
}


QSet<IDesktopBox*> k_DesktopBox::outgoingBoxes() const
{
	return mk_ConnectedOutgoingBoxes;
}


void k_DesktopBox::setBatchMode(bool ab_Enabled)
{
	bool lb_OldBatchMode = mb_BatchMode;
	mb_BatchMode = ab_Enabled;
	if (lb_OldBatchMode != mb_BatchMode)
		emit batchModeChanged(mb_BatchMode);
	repaint();
}


void k_DesktopBox::connectIncomingBox(IDesktopBox* ak_Other_)
{
	if (mk_ConnectedIncomingBoxes.contains(ak_Other_))
		return;

	mk_ConnectedIncomingBoxes.insert(ak_Other_);
	ak_Other_->connectOutgoingBox(this);
    emit boxConnected(ak_Other_, true);
    connect(dynamic_cast<QObject*>(ak_Other_), SIGNAL(changed()), this, SLOT(updateSlot()));
    updateSlot();
}


void k_DesktopBox::connectOutgoingBox(IDesktopBox* ak_Other_)
{
	if (mk_ConnectedOutgoingBoxes.contains(ak_Other_))
		return;
	
	mk_ConnectedOutgoingBoxes.insert(ak_Other_);
	ak_Other_->connectIncomingBox(this);
	emit boxConnected(ak_Other_, false);
}


void k_DesktopBox::disconnectIncomingBox(IDesktopBox* ak_Other_)
{
	if (!mk_ConnectedIncomingBoxes.contains(ak_Other_))
		return;
	
	mk_ConnectedIncomingBoxes.remove(ak_Other_);
	ak_Other_->disconnectOutgoingBox(this);
	emit boxDisconnected(ak_Other_, true);
    disconnect(dynamic_cast<QObject*>(ak_Other_), SIGNAL(changed()), this, SLOT(updateSlot()));
    updateSlot();
}


void k_DesktopBox::disconnectOutgoingBox(IDesktopBox* ak_Other_)
{
	if (!mk_ConnectedOutgoingBoxes.contains(ak_Other_))
		return;

	mk_ConnectedOutgoingBoxes.remove(ak_Other_);
	ak_Other_->disconnectIncomingBox(this);
	emit boxDisconnected(ak_Other_, false);
}


void k_DesktopBox::disconnectAll()
{
	foreach (IDesktopBox* lk_Box_, mk_ConnectedIncomingBoxes)
		disconnectIncomingBox(lk_Box_);
	foreach (IDesktopBox* lk_Box_, mk_ConnectedOutgoingBoxes)
		disconnectOutgoingBox(lk_Box_);
}


void k_DesktopBox::setResizable(bool ab_EnabledX, bool ab_EnabledY)
{
	mb_ResizableX = ab_EnabledX;
	mb_ResizableY = ab_EnabledY;
	resize(mb_ResizableX ? width() : 1, mb_ResizableY ? height() : 1);
	repaint();
}


void k_DesktopBox::toggleUi()
{
}


QRectF k_DesktopBox::rect()
{
	return frameGeometry();
}


void k_DesktopBox::resizeEvent(QResizeEvent* /*event*/)
{
	emit resized();
}


void k_DesktopBox::moveEvent(QMoveEvent* event)
{
	emit moved(event->pos() - event->oldPos());
}


void k_DesktopBox::paintEvent(QPaintEvent* /*event*/)
{
	resize(mb_ResizableX ? width() : 1, mb_ResizableY ? height() : 1);
	
	QPainter lk_Painter(this);
	
	QPen lk_Pen(TANGO_ALUMINIUM_3);
	float lf_PenWidth = 1.5;
	lk_Pen.setWidthF(lf_PenWidth);
	lk_Painter.setPen(lk_Pen);
	QBrush lk_Brush(TANGO_ALUMINIUM_0);
	lk_Painter.setBrush(lk_Brush);
	lk_Painter.drawRoundedRect(QRectF(lf_PenWidth * 0.5, lf_PenWidth * 0.5, (qreal)width() - lf_PenWidth, (qreal)height() - lf_PenWidth), 8.0, 8.0);

	// draw resize grip
	if (mb_ResizableX || mb_ResizableY)
		lk_Painter.drawPixmap(width() - mk_ResizeGripPixmap.width() - 2, 
							  height() - mk_ResizeGripPixmap.height() - 2, 
							  mk_ResizeGripPixmap);
}


void k_DesktopBox::mousePressEvent(QMouseEvent* event)
{
	event->ignore();
	mk_MousePressPosition = event->globalPos();
	if ((mb_ResizableX || mb_ResizableY) && (event->pos() - QPoint(width(), height())).manhattanLength() <= 16)
	{
		mb_Resizing = true;
		mk_OldSize = this->size();
		event->accept();
	}
/*	else
	{
		mk_Desktop_->moveSelectedBoxesStart(this);
		mb_Moving = true;
		mk_OldPosition = this->pos();
	}*/
	else
		emit clicked(event);
	event->accept();
}


void k_DesktopBox::mouseReleaseEvent(QMouseEvent* event)
{
	mb_Moving = false;
	mb_Resizing = false;
	event->accept();
}


void k_DesktopBox::mouseMoveEvent(QMouseEvent* event)
{
/*	if (mb_Moving)
	{
		QPoint lk_GlobalPos = event->globalPos();
		QPoint lk_Delta = lk_GlobalPos - mk_MousePressPosition;
		mk_Desktop_->moveSelectedBoxes(lk_Delta);
		event->accept();
	}*/
	if (mb_Resizing)
	{
		QPoint lk_Delta = event->globalPos() - mk_MousePressPosition;
		this->resize(mk_OldSize + QSize(mb_ResizableX ? lk_Delta.x() : 0, mb_ResizableY ? lk_Delta.y() : 0));
		event->accept();
	}
}


void k_DesktopBox::update()
{
}


void k_DesktopBox::updateSlot()
{
    this->update();
}
