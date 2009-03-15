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

#include "Desktop.h"
#include "DesktopBoxFactory.h"
#include "DesktopBox.h"
#include "IFileBox.h"
#include "IScriptBox.h"
#include "PipelineMainWindow.h"
#include "Tango.h"


k_Desktop::k_Desktop(QWidget* ak_Parent_, k_Proteomatic& ak_Proteomatic, k_PipelineMainWindow& ak_PipelineMainWindow)
	: QGraphicsView(ak_Parent_)
	, mk_Proteomatic(ak_Proteomatic)
	, mk_PipelineMainWindow(ak_PipelineMainWindow)
	, mk_ArrowStartBox_(NULL)
	, mk_ArrowEndBox_(NULL)
	, mk_ArrowPathItem_(NULL)
{
	setAcceptDrops(true);
	mk_pGraphicsScene = RefPtr<QGraphicsScene>(new QGraphicsScene(ak_Parent_));
	setScene(mk_pGraphicsScene.get_Pointer());
	setRenderHint(QPainter::Antialiasing, true);
	setRenderHint(QPainter::TextAntialiasing, true);
	setRenderHint(QPainter::SmoothPixmapTransform, true);
	setBackgroundBrush(QBrush(QColor("#f8f8f8")));
	setSceneRect(-10000.0, -10000.0, 20000.0, 20000.0);
	setDragMode(QGraphicsView::ScrollHandDrag);
	translate(0.5, 0.5);
}


k_Desktop::~k_Desktop()
{
	foreach (QGraphicsProxyWidget* lk_Widget_, mk_ProxyWidgetForDesktopBox.values())
		mk_pGraphicsScene->removeItem(lk_Widget_);
}


void k_Desktop::addInputFileBox(const QString& as_Path)
{
}


void k_Desktop::addInputFileListBox()
{
	RefPtr<IDesktopBox> lk_pBox = k_DesktopBoxFactory::makeFileListBox(this, mk_Proteomatic);
	addBox(lk_pBox);
}


void k_Desktop::addScriptBox(const QString& as_ScriptUri)
{
	RefPtr<IDesktopBox> lk_pBox = k_DesktopBoxFactory::makeScriptBox(as_ScriptUri, this, mk_Proteomatic);
	if (lk_pBox)
		addBox(lk_pBox);
}


void k_Desktop::arrowPressed()
{
	mk_ArrowStartBox_ = dynamic_cast<IDesktopBox*>(sender());
	mk_ArrowEndBox_ = NULL;
	mk_ArrowPathItem_ = mk_pGraphicsScene->
		addPath(QPainterPath(), QPen(QColor(TANGO_ALUMINIUM_3)), QBrush(QColor(TANGO_ALUMINIUM_3)));
	mk_ArrowPathItem_->setZValue(1000.0);
}


void k_Desktop::arrowReleased()
{
	mk_ArrowStartBox_ = NULL;
	mk_ArrowEndBox_ = NULL;
	if (mk_ArrowPathItem_)
		delete mk_ArrowPathItem_;
}


void k_Desktop::mouseMoveEvent(QMouseEvent* event)
{
	QGraphicsView::mouseMoveEvent(event);
	if (mk_ArrowStartBox_)
		updateUserArrow(mapToScene(event->pos()));
}


void k_Desktop::addBox(RefPtr<IDesktopBox> ak_pBox)
{
	mk_Boxes.append(ak_pBox);
	k_DesktopBox* lk_DesktopBox_ = dynamic_cast<k_DesktopBox*>(ak_pBox.get_Pointer());
	mk_ProxyWidgetForDesktopBox[ak_pBox.get_Pointer()] = mk_pGraphicsScene->addWidget(lk_DesktopBox_);
	IFileBox* lk_FileBox_ = dynamic_cast<IFileBox*>(ak_pBox.get_Pointer());
	if (lk_FileBox_)
	{
		connect(dynamic_cast<QObject*>(lk_FileBox_), SIGNAL(arrowPressed()), this, SLOT(arrowPressed()));
		connect(dynamic_cast<QObject*>(lk_FileBox_), SIGNAL(arrowReleased()), this, SLOT(arrowReleased()));
	}
}


void k_Desktop::updateUserArrow(QPointF ak_MousePosition)
{
	if (mk_ArrowPathItem_ == NULL || mk_ArrowStartBox_ == NULL)
		return;
	
	QGraphicsPathItem* lk_PathItem_ = mk_ArrowPathItem_;
	QPainterPath lk_Path;
	
	QPointF lk_Start = boxLocation(mk_ArrowStartBox_);
	QPointF lk_End = (mk_ArrowEndBox_ == NULL) ? ak_MousePosition : boxLocation(mk_ArrowEndBox_);
	lk_Start = intersectLineWithBox(lk_End, lk_Start, mk_ArrowStartBox_);
	if (mk_ArrowEndBox_ != NULL)
 		lk_End = intersectLineWithBox(lk_Start, lk_End, mk_ArrowEndBox_);
	
	lk_Path.moveTo(lk_Start);
	lk_Path.lineTo(lk_End);
	
	QPointF lk_Dir = lk_End - lk_Start;
	double ld_Length = sqrt(lk_Dir.x() * lk_Dir.x() + lk_Dir.y() * lk_Dir.y());
	if (ld_Length > 1.0)
	{
		lk_Dir /= ld_Length;
		QPointF lk_Normal = QPointF(-lk_Dir.y(), lk_Dir.x());
		QPolygonF lk_Arrow;
		lk_Arrow << lk_End;
		lk_Arrow << lk_End - lk_Dir * 10.0 + lk_Normal * 3.5;
		lk_Arrow << lk_End - lk_Dir * 7.0;
		lk_Arrow << lk_End - lk_Dir * 10.0 - lk_Normal * 3.5;
		lk_Arrow << lk_End;
		lk_Path.addPolygon(lk_Arrow);
	}
	
	lk_PathItem_->setPath(lk_Path);
}


QPoint k_Desktop::boxLocation(IDesktopBox* ak_Box_) const
{
	k_DesktopBox* lk_Box_ = dynamic_cast<k_DesktopBox*>(ak_Box_);
	return lk_Box_->pos() + QPoint(lk_Box_->width(), lk_Box_->height()) / 2;
}


double k_Desktop::intersect(QPointF p0, QPointF d0, QPointF p1, QPointF d1)
{
	double d = d0.x() * d1.y() - d0.y() * d1.x();
	if (fabs(d) < 0.000001)
		return 0.0;
	return (d1.x() * (p0.y() - p1.y()) + d1.y() * (p1.x() - p0.x())) / d;
}


void k_Desktop::boxConnector(IDesktopBox* ak_Box0_, IDesktopBox* ak_Box1_, QPointF& ak_Point0, QPointF& ak_Point1)
{
	QPointF lk_Point0 = boxLocation(ak_Box0_);
	QPointF lk_Point1 = boxLocation(ak_Box1_);
	ak_Point0 = intersectLineWithBox(lk_Point1, lk_Point0, ak_Box0_);
	ak_Point1 = intersectLineWithBox(lk_Point0, lk_Point1, ak_Box1_);
}


QPointF k_Desktop::intersectLineWithBox(const QPointF& ak_Point0, const QPointF& ak_Point1, IDesktopBox* ak_Box_)
{
	QPointF lk_Dir = ak_Point1 - ak_Point0;
	QPointF lk_Quadrant = QPointF(fabs(lk_Dir.x()), fabs(lk_Dir.y()));
	k_DesktopBox* lk_Box_ = dynamic_cast<k_DesktopBox*>(ak_Box_);
	QPointF lk_Diagonal = QPointF(lk_Box_->width(), lk_Box_->height());
	double t = 1.0;
	if (lk_Quadrant.x() * lk_Diagonal.y() > lk_Quadrant.y() * lk_Diagonal.x())
	{
		QPointF lk_Quadrant = ak_Point0 - ak_Point1;
		if (lk_Quadrant.x() < 0)
			t = intersect(ak_Point0, lk_Dir, boxLocation(ak_Box_) - QPointF(lk_Box_->width(), lk_Box_->height()) * 0.5, QPointF(0, lk_Box_->height()));
		else
			t = intersect(ak_Point0, lk_Dir, boxLocation(ak_Box_) - QPointF(lk_Box_->width(), lk_Box_->height()) * 0.5 + QPointF(lk_Box_->width(), 0), QPointF(0, lk_Box_->height()));
	}
	else
	{
		QPointF lk_Quadrant = ak_Point0 - ak_Point1;
		if (lk_Quadrant.y() < 0)
			t = intersect(ak_Point0, lk_Dir, boxLocation(ak_Box_) - QPointF(lk_Box_->width(), lk_Box_->height()) * 0.5, QPointF(lk_Box_->width(), 0));
		else
			t = intersect(ak_Point0, lk_Dir, boxLocation(ak_Box_) - QPointF(lk_Box_->width(), lk_Box_->height()) * 0.5 + QPointF(0, lk_Box_->height()), QPointF(lk_Box_->width(), 0));
	}
	
	return ak_Point0 + lk_Dir * t;
}
