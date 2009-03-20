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
	, mk_GraphicsScene(ak_Parent_)
	, md_Scale(1.0)
	, mk_ArrowStartBox_(NULL)
	, mk_ArrowEndBox_(NULL)
	, mk_UserArrowPathItem_(NULL)
{
	setAcceptDrops(true);
	setScene(&mk_GraphicsScene);
	setRenderHint(QPainter::Antialiasing, true);
	setRenderHint(QPainter::TextAntialiasing, true);
	setRenderHint(QPainter::SmoothPixmapTransform, true);
	setBackgroundBrush(QBrush(QColor("#f8f8f8")));
	setSceneRect(0.0, 0.0, 20000.0, 20000.0);
	setDragMode(QGraphicsView::ScrollHandDrag);
	setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
	centerOn(10000.5, 10000.5);
	
	QPen lk_Pen(QColor(TANGO_ALUMINIUM_3));
	lk_Pen.setWidthF(1.5);
	lk_Pen.setStyle(Qt::DashLine);
	mk_SelectionGraphicsPathItem_ = mk_GraphicsScene.addPath(QPainterPath(), lk_Pen);
	mk_SelectionGraphicsPathItem_->setZValue(-2.0);

	lk_Pen = QPen(QColor(TANGO_BUTTER_2));
	lk_Pen.setWidthF(1.5);
	mk_BatchGraphicsPathItem_ = mk_GraphicsScene.addPath(QPainterPath(), lk_Pen, QBrush(QColor(TANGO_BUTTER_0)));
	mk_BatchGraphicsPathItem_->setZValue(-3.0);
}


k_Desktop::~k_Desktop()
{
	while (!mk_Boxes.empty())
		removeBox(mk_Boxes.toList().first());
}


k_PipelineMainWindow& k_Desktop::pipelineMainWindow() const
{
	return mk_PipelineMainWindow;
}


QGraphicsScene& k_Desktop::graphicsScene()
{
	return mk_GraphicsScene;
}

	
void k_Desktop::addInputFileListBox()
{
	IDesktopBox* lk_Box_ = k_DesktopBoxFactory::makeFileListBox(this, mk_Proteomatic);
	k_DesktopBox* lk_DesktopBox_ = dynamic_cast<k_DesktopBox*>(lk_Box_);
	addBox(lk_Box_);
	lk_DesktopBox_->resize(300, 10);
}


void k_Desktop::addScriptBox(const QString& as_ScriptUri)
{
	IDesktopBox* lk_Box_ = k_DesktopBoxFactory::makeScriptBox(as_ScriptUri, this, mk_Proteomatic);
	if (lk_Box_)
		addBox(lk_Box_);
}


void k_Desktop::addBox(IDesktopBox* ak_Box_)
{
	k_DesktopBox* lk_DesktopBox_ = dynamic_cast<k_DesktopBox*>(ak_Box_);
	QRectF lk_BoundingRect = mk_GraphicsScene.itemsBoundingRect();
	mk_GraphicsScene.addWidget(lk_DesktopBox_);
	IFileBox* lk_FileBox_ = dynamic_cast<IFileBox*>(ak_Box_);
	if (lk_FileBox_)
	{
		connect(dynamic_cast<QObject*>(lk_FileBox_), SIGNAL(arrowPressed()), this, SLOT(arrowPressed()));
		connect(dynamic_cast<QObject*>(lk_FileBox_), SIGNAL(arrowReleased()), this, SLOT(arrowReleased()));
	}
	connect(lk_DesktopBox_, SIGNAL(moved(QPoint)), this, SLOT(boxMovedOrResized(QPoint)));
	connect(lk_DesktopBox_, SIGNAL(resized()), this, SLOT(boxMovedOrResized()));
	connect(lk_DesktopBox_, SIGNAL(clicked(Qt::KeyboardModifiers)), this, SLOT(boxClicked(Qt::KeyboardModifiers)));
	connect(lk_DesktopBox_, SIGNAL(batchModeChanged(bool)), this, SLOT(boxBatchModeChanged(bool)));
	mk_Boxes.insert(ak_Box_);
	lk_DesktopBox_->resize(1, 1);
	QPointF lk_FreeSpace = findFreeSpace(lk_BoundingRect, mk_Boxes.size() - 1, ak_Box_);
	lk_DesktopBox_->move(QPoint((int)lk_FreeSpace.x(), (int)lk_FreeSpace.y()));
	redraw();
}


void k_Desktop::removeBox(IDesktopBox* ak_Box_)
{
	if (!mk_Boxes.contains(ak_Box_))
		return;
	
	foreach (IDesktopBox* lk_Box_, ak_Box_->incomingBoxes())
		disconnectBoxes(lk_Box_, ak_Box_);
	foreach (IDesktopBox* lk_Box_, ak_Box_->outgoingBoxes())
		disconnectBoxes(ak_Box_, lk_Box_);
	delete ak_Box_;
	mk_Boxes.remove(ak_Box_);
	mk_SelectedBoxes.remove(ak_Box_);
	mk_BatchBoxes.remove(ak_Box_);
	redrawSelection();
	redrawBatchFrame();
}


void k_Desktop::connectBoxes(IDesktopBox* ak_Source_, IDesktopBox* ak_Destination_)
{
	ak_Source_->connectOutgoingBox(ak_Destination_);
	QGraphicsPathItem* lk_GraphicsPathItem_ = 
		mk_GraphicsScene.addPath(QPainterPath(), QPen(QColor(TANGO_ALUMINIUM_3)), QBrush(QColor(TANGO_ALUMINIUM_3)));
	lk_GraphicsPathItem_->setZValue(-1.0);
	QPen lk_Pen("#f8f8f8");
	lk_Pen.setWidthF(10.0);
	QGraphicsLineItem* lk_ArrowProxy_ = 
		mk_GraphicsScene.addLine(QLineF(), lk_Pen);
	lk_ArrowProxy_->setZValue(-1000.0);
	mk_ArrowForProxy[lk_ArrowProxy_] = lk_GraphicsPathItem_;
	mk_Arrows[lk_GraphicsPathItem_] = tk_BoxPair(ak_Source_, ak_Destination_);
	mk_ArrowForBoxPair[tk_BoxPair(ak_Source_, ak_Destination_)] = lk_GraphicsPathItem_;
	mk_ArrowsForBox[ak_Source_].insert(lk_GraphicsPathItem_);
	mk_ArrowsForBox[ak_Destination_].insert(lk_GraphicsPathItem_);
	mk_ArrowProxy[lk_GraphicsPathItem_] = lk_ArrowProxy_;
	this->updateArrow(lk_GraphicsPathItem_);
}


void k_Desktop::disconnectBoxes(IDesktopBox* ak_Source_, IDesktopBox* ak_Destination_)
{
	ak_Source_->disconnectOutgoingBox(ak_Destination_);
	tk_BoxPair lk_BoxPair(ak_Source_, ak_Destination_);
	QGraphicsPathItem* lk_GraphicsPathItem_ = mk_ArrowForBoxPair[lk_BoxPair];
	mk_Arrows.remove(lk_GraphicsPathItem_);
	mk_ArrowForBoxPair.remove(lk_BoxPair);
	mk_ArrowsForBox[ak_Source_].remove(lk_GraphicsPathItem_);
	mk_ArrowsForBox[ak_Destination_].remove(lk_GraphicsPathItem_);
	mk_ArrowForProxy.remove(mk_ArrowProxy[lk_GraphicsPathItem_]);
	delete mk_ArrowProxy[lk_GraphicsPathItem_];
	mk_ArrowProxy.remove(lk_GraphicsPathItem_);
	delete lk_GraphicsPathItem_;
}


void k_Desktop::moveBox(IDesktopBox* ak_Box_, QPoint ak_Delta)
{
	k_DesktopBox* lk_Box_ = dynamic_cast<k_DesktopBox*>(ak_Box_);
	if (mk_SelectedBoxes.contains(ak_Box_))
	{
		foreach (IDesktopBox* lk_SetBox_, mk_SelectedBoxes)
		{
			k_DesktopBox* lk_SetDesktopBox_ = dynamic_cast<k_DesktopBox*>(lk_SetBox_);
			lk_SetDesktopBox_->move(lk_SetDesktopBox_->pos() + ak_Delta);
		}
	}
	else
		lk_Box_->move(lk_Box_->pos() + ak_Delta);
}


void k_Desktop::createFilenameTags(QStringList ak_Filenames, QHash<QString, QString>& ak_TagForFilename, QString& as_PrefixWithoutTags)
{
	QHash<QString, QString> lk_Result;
	QStringList lk_Files = ak_Filenames;
	QHash<QString, QSet<QString> > lk_TagInFilenames;
	QHash<QString, int> lk_MinTagCountInFilenames;
	QHash<QString, QStringList> lk_AllChopped;
	foreach (QString ls_Path, lk_Files)
	{
		QString ls_Basename = QFileInfo(ls_Path).baseName();
		// ls_Basename contains MT_Hyd_CPAN_040708_33-no-ms1
		QStringList lk_Chopped;
		// chop string
		int li_LastClass = -1;
		for (int i = 0; i < ls_Basename.length(); ++i)
		{
			int li_ThisClass = 0;
			QChar lk_Char = ls_Basename.at(i);
			if (lk_Char >= '0' && lk_Char <= '9')
				li_ThisClass = 1;
			else if ((lk_Char >= 'A' && lk_Char <= 'Z') || (lk_Char >= 'a' && lk_Char <= 'z'))
				li_ThisClass = 2;
			if (li_ThisClass != li_LastClass)
				lk_Chopped.append(lk_Char);
			else
				lk_Chopped.last().append(lk_Char);
			li_LastClass = li_ThisClass;
		}
		lk_AllChopped[ls_Path] = lk_Chopped;
		// lk_Chopped contains: MT _ Hyd _ CPAN _ 040708 _ 33 - no - ms 1
		QHash<QString, int> lk_TagCount;
		foreach (QString ls_Tag, lk_Chopped)
		{
			if (!lk_TagCount.contains(ls_Tag))
				lk_TagCount[ls_Tag] = 0;
			lk_TagCount[ls_Tag] += 1;
		}
		foreach (QString ls_Tag, lk_TagCount.keys())
		{
			if (!lk_TagInFilenames.contains(ls_Tag))
				lk_TagInFilenames[ls_Tag] = QSet<QString>();
			lk_TagInFilenames[ls_Tag].insert(ls_Path);
			if (!lk_MinTagCountInFilenames.contains(ls_Tag))
				lk_MinTagCountInFilenames[ls_Tag] = lk_TagCount[ls_Tag];
			lk_MinTagCountInFilenames[ls_Tag] = std::min<int>(lk_MinTagCountInFilenames[ls_Tag], lk_TagCount[ls_Tag]);
		}
	}
	foreach (QString ls_Path, lk_Files)
	{
		QStringList lk_Chopped = lk_AllChopped[ls_Path];
		foreach (QString ls_Tag, lk_TagInFilenames.keys())
		{
			if (lk_TagInFilenames[ls_Tag].size() != lk_Files.size())
				continue;
			for (int i = 0; i < lk_MinTagCountInFilenames[ls_Tag]; ++i)
				lk_Chopped.removeOne(ls_Tag);
		}
		QString ls_Short = lk_Chopped.join("");
		lk_Result[ls_Path] = ls_Short;
	}
	
	// determine common prefix without tags
	QString ls_PrefixWithoutTags;
	if (!lk_Files.empty())
	{
		QStringList lk_Chopped = lk_AllChopped[lk_Files.first()];
		foreach (QString ls_Tag, lk_Chopped)
		{
			if (lk_TagInFilenames[ls_Tag].size() != lk_Files.size())
				continue;
			if (lk_MinTagCountInFilenames[ls_Tag] > 0)
			{
				ls_PrefixWithoutTags += ls_Tag;
				lk_MinTagCountInFilenames[ls_Tag] -= 1;
			}
		}
	}
	
	ak_TagForFilename = lk_Result;
	as_PrefixWithoutTags = ls_PrefixWithoutTags;
}


void k_Desktop::redraw()
{
	redrawSelection();
	redrawBatchFrame();
	foreach (QGraphicsPathItem* lk_Arrow_, mk_Arrows.keys())
		updateArrow(lk_Arrow_);
}


void k_Desktop::boxMovedOrResized(QPoint ak_Delta)
{
	IDesktopBox* lk_Sender_ = dynamic_cast<IDesktopBox*>(sender());
	if (lk_Sender_ && mk_ArrowsForBox.contains(lk_Sender_))
		foreach (QGraphicsPathItem* lk_Arrow_, mk_ArrowsForBox[lk_Sender_])
			updateArrow(lk_Arrow_);
	foreach (IDesktopBox* lk_Box_, mk_SelectedBoxes)
		foreach (QGraphicsPathItem* lk_Arrow_, mk_ArrowsForBox[lk_Box_])
			updateArrow(lk_Arrow_);
	redraw();
}


void k_Desktop::boxClicked(Qt::KeyboardModifiers ae_Modifiers)
{
	IDesktopBox* lk_Box_ = dynamic_cast<IDesktopBox*>(sender());
	if (!lk_Box_)
		return;
	if ((ae_Modifiers & Qt::ControlModifier) == Qt::ControlModifier)
	{
		if (mk_SelectedBoxes.contains(lk_Box_))
			mk_SelectedBoxes.remove(lk_Box_);
		else
			mk_SelectedBoxes.insert(lk_Box_);
	}
	else
	{
		if (!mk_SelectedBoxes.contains(lk_Box_))
			clearSelection();
		mk_SelectedBoxes.insert(lk_Box_);
	}
	redrawSelection();
}


void k_Desktop::arrowPressed()
{
	mk_ArrowStartBox_ = dynamic_cast<IDesktopBox*>(sender());
	mk_ArrowEndBox_ = NULL;
	mk_UserArrowPathItem_ = mk_GraphicsScene.
		addPath(QPainterPath(), QPen(QColor(TANGO_ALUMINIUM_3)), QBrush(QColor(TANGO_ALUMINIUM_3)));
	mk_UserArrowPathItem_->setZValue(1000.0);
}


void k_Desktop::arrowReleased()
{
	if (mk_ArrowStartBox_ && mk_ArrowEndBox_)
		connectBoxes(mk_ArrowStartBox_, mk_ArrowEndBox_);
	
	mk_ArrowStartBox_ = NULL;
	mk_ArrowEndBox_ = NULL;
	if (mk_UserArrowPathItem_)
	{
		delete mk_UserArrowPathItem_;
		mk_UserArrowPathItem_ = NULL;
	}
}


void k_Desktop::boxBatchModeChanged(bool ab_Enabled)
{
	IDesktopBox* lk_Box_ = dynamic_cast<IDesktopBox*>(sender());
	if (!lk_Box_)
		return;
	
	if (ab_Enabled)
		mk_BatchBoxes.insert(lk_Box_);
	else
		mk_BatchBoxes.remove(lk_Box_);
	
	redrawBatchFrame();
	foreach (QGraphicsPathItem* lk_Arrow_, mk_ArrowsForBox[lk_Box_])
		updateArrow(lk_Arrow_);
}


void k_Desktop::updateArrow(QGraphicsPathItem* ak_Arrow_)
{
	IDesktopBox* lk_ArrowStartBox_ = mk_Arrows[ak_Arrow_].first;
	IDesktopBox* lk_ArrowEndBox_ = mk_Arrows[ak_Arrow_].second;
	
	QPointF lk_Start = boxLocation(lk_ArrowStartBox_);
	QPointF lk_End = boxLocation(lk_ArrowEndBox_);
	
	/*
	QPainterPath lk_Path;
	lk_Path.moveTo(lk_Start);
	lk_Path.lineTo(lk_End);

	QPainterPath lk_Outline;
	lk_Outline = grownPathForBox(lk_ArrowStartBox_, 0 + (mk_SelectedBoxes.contains(lk_ArrowStartBox_) ? 3 : 0) + (lk_ArrowStartBox_->batchMode() ? 4 : 0));
	lk_Path = lk_Path.subtracted(lk_Outline);
	lk_Outline = grownPathForBox(lk_ArrowEndBox_, 0 + (mk_SelectedBoxes.contains(lk_ArrowEndBox_) ? 3 : 0) + (lk_ArrowEndBox_->batchMode() ? 4 : 0));
	lk_Path = lk_Path.subtracted(lk_Outline);

	lk_Start = lk_Path.pointAtPercent(0.0);
	lk_End = lk_Path.pointAtPercent(1.0);
	*/
	

	lk_Start = intersectLineWithBox(lk_End, lk_Start, lk_ArrowStartBox_);
	lk_End = intersectLineWithBox(lk_Start, lk_End, lk_ArrowEndBox_);
	
	QPen lk_Pen(TANGO_ALUMINIUM_3);
	if ((dynamic_cast<IFileBox*>(lk_ArrowStartBox_)) && lk_ArrowStartBox_->batchMode())
		lk_Pen = QPen(TANGO_BUTTER_2);
	ak_Arrow_->setPen(lk_Pen);
	
	updateArrowInternal(ak_Arrow_, lk_Start, lk_End);
	mk_ArrowProxy[ak_Arrow_]->setLine(QLineF(QPointF(lk_Start), QPointF(lk_End)));
}


void k_Desktop::redrawSelection()
{
	QPainterPath lk_Path;
	
	foreach (IDesktopBox* lk_Box_, mk_SelectedBoxes)
		lk_Path = lk_Path.united(grownPathForBox(lk_Box_, 3));
	
	foreach (QGraphicsPathItem* lk_Arrow_, mk_SelectedArrows)
		lk_Path = lk_Path.united(grownPathForArrow(lk_Arrow_, 3));
	
	mk_SelectionGraphicsPathItem_->setPath(lk_Path);
	redrawBatchFrame();
}


void k_Desktop::deleteSelected()
{
	QList<IDesktopBox*> lk_BoxDeleteList = mk_SelectedBoxes.toList();
	QList<QGraphicsPathItem*> lk_ArrowDeleteList = mk_SelectedArrows.toList();

	foreach (QGraphicsPathItem* lk_Arrow_, lk_ArrowDeleteList)
		disconnectBoxes(mk_Arrows[lk_Arrow_].first, mk_Arrows[lk_Arrow_].second);
	
	while (!mk_SelectedBoxes.empty())
		removeBox(mk_SelectedBoxes.toList().first());
	
	redrawSelection();
	redrawBatchFrame();
}


void k_Desktop::redrawBatchFrame()
{
	QPainterPath lk_Path;
	foreach (IDesktopBox* lk_Box_, mk_BatchBoxes)
	{
		lk_Path = lk_Path.united(grownPathForBox(lk_Box_, mk_SelectedBoxes.contains(lk_Box_) ? 7 : 3));
		IFileBox* lk_FileBox_ = dynamic_cast<IFileBox*>(lk_Box_);
		if (lk_FileBox_)
		{
			foreach (IDesktopBox* lk_PeerBox_, lk_Box_->outgoingBoxes())
			{
				if (!dynamic_cast<IScriptBox*>(lk_PeerBox_))
					continue;
				QPointF p0 = boxLocation(lk_Box_);
				QPointF p1 = boxLocation(lk_PeerBox_);
				QPointF p0c = p0;
				QPointF p1c = p1;
				intersectLineWithBox(p0c, p1c, lk_Box_);
				intersectLineWithBox(p0c, p1c, lk_PeerBox_);
				QPointF m0 = p0c + (p1c - p0c) * 0.1;
				QPointF m1 = p0c + (p1c - p0c) * 0.9;
				
				QSize s0 = dynamic_cast<k_DesktopBox*>(lk_Box_)->size();
				QSize s1 = dynamic_cast<k_DesktopBox*>(lk_PeerBox_)->size();
				double r0 = s0.width();
				r0 = std::min<double>(r0, s0.height());
				double r1 = s1.width();
				r1 = std::min<double>(r1, s1.height());
				
				r0 *= 0.5;
				r1 *= 0.5;
				
				QPointF lk_Dir = p1 - p0;
				double lf_Length = lk_Dir.x() * lk_Dir.x() + lk_Dir.y() * lk_Dir.y();
				if (lf_Length > 1.0)
				{
					lf_Length = sqrt(lf_Length);
					QPointF lk_Normal = QPointF(-lk_Dir.y(), lk_Dir.x()) / lf_Length;
					QPainterPath lk_SubPath;
					lk_SubPath.moveTo(p0 + lk_Normal * r0);
					lk_SubPath.cubicTo(m0, m1, p1 + lk_Normal * r1);
					lk_SubPath.lineTo(p1 - lk_Normal * r1);
					lk_SubPath.cubicTo(m1, m0, p0 - lk_Normal * r0);
					lk_SubPath.lineTo(p0 + lk_Normal * r0);
					lk_Path = lk_Path.united(lk_SubPath);
				}
			}
		}
	}
	mk_BatchGraphicsPathItem_->setPath(lk_Path);
}


void k_Desktop::clearSelection()
{
	mk_SelectedBoxes.clear();
	mk_SelectedArrows.clear();
}


void k_Desktop::keyPressEvent(QKeyEvent* event)
{
	QGraphicsView::keyPressEvent(event);
	if (!event->isAccepted())
	{
		if (event->matches(QKeySequence::Delete))
			deleteSelected();
	}
}


void k_Desktop::mousePressEvent(QMouseEvent* event)
{
	QPointF lk_Position = this->mapToScene(event->pos());
	if (mk_GraphicsScene.items(lk_Position).empty())
	{
		if ((event->modifiers() & Qt::ControlModifier) == 0)
		{
			clearSelection();
			redrawSelection();
		}
	}
	else
	{
		QList<QGraphicsItem*> lk_ItemList = mk_GraphicsScene.items(lk_Position);
		if (!lk_ItemList.empty())
		{
			QGraphicsLineItem* lk_ProxyLine_ = NULL;
			for (int i = lk_ItemList.size() - 1; i >= 0; --i)
			{
				lk_ProxyLine_ = dynamic_cast<QGraphicsLineItem*>(lk_ItemList[i]);
				if (lk_ProxyLine_)
					break;
			}
			if (lk_ProxyLine_)
			{
				if ((event->modifiers() & Qt::ControlModifier) == 0)
				{
					clearSelection();
					redrawSelection();
				}
				QGraphicsPathItem* lk_Arrow_ = mk_ArrowForProxy[lk_ProxyLine_];
				if (mk_SelectedArrows.contains(lk_Arrow_))
					mk_SelectedArrows.remove(lk_Arrow_);
				else
					mk_SelectedArrows.insert(lk_Arrow_);
				redrawSelection();
			}
		}
	}
	QGraphicsView::mousePressEvent(event);
}


void k_Desktop::mouseMoveEvent(QMouseEvent* event)
{
	QGraphicsView::mouseMoveEvent(event);
	if (mk_ArrowStartBox_)
	{
		mk_ArrowEndBox_ = boxAt(mapToScene(event->pos()));
		updateUserArrow(mapToScene(event->pos()));
	}
}


void k_Desktop::wheelEvent(QWheelEvent* event)
{
	if ((event->modifiers() & Qt::ControlModifier) != 0)
	{
		double ld_ScaleDelta = pow(1.1, fabs(event->delta() / 100.0));
		if (event->delta() < 0)
			ld_ScaleDelta = 1.0 / ld_ScaleDelta;
		md_Scale *= ld_ScaleDelta;
		md_Scale = std::max<double>(md_Scale, 0.3);
		md_Scale = std::min<double>(md_Scale, 1.0);
		QMatrix lk_Matrix = this->matrix();
		lk_Matrix.setMatrix(md_Scale, lk_Matrix.m12(), lk_Matrix.m21(), md_Scale, lk_Matrix.dx(), lk_Matrix.dy());
		this->setMatrix(lk_Matrix);
	}
}


void k_Desktop::updateUserArrow(QPointF ak_MousePosition)
{
	if (!(mk_UserArrowPathItem_ && mk_ArrowStartBox_))
		return;
	
	QPointF lk_Start = boxLocation(mk_ArrowStartBox_);
	QPointF lk_End;
	if (mk_ArrowEndBox_)
		lk_End = boxLocation(mk_ArrowEndBox_);
	else
		lk_End = ak_MousePosition;
	
	lk_Start = intersectLineWithBox(lk_End, lk_Start, mk_ArrowStartBox_);
	if (mk_ArrowEndBox_)
		lk_End = intersectLineWithBox(lk_Start, lk_End, mk_ArrowEndBox_);
	
	updateArrowInternal(mk_UserArrowPathItem_, lk_Start, lk_End);
}


QPoint k_Desktop::boxLocation(IDesktopBox* ak_Box_) const
{
	k_DesktopBox* lk_Box_ = dynamic_cast<k_DesktopBox*>(ak_Box_);
	return lk_Box_->pos() + QPoint(lk_Box_->width(), lk_Box_->height()) / 2;
}


IDesktopBox* k_Desktop::boxAt(QPointF ak_Point) const
{
	QGraphicsItem* lk_GraphicsItem_ = mk_GraphicsScene.itemAt(ak_Point);
	QGraphicsWidget* lk_GraphicsWidget_ = dynamic_cast<QGraphicsWidget*>(lk_GraphicsItem_);
	if (lk_GraphicsWidget_ == NULL)
		return NULL;
	QGraphicsProxyWidget* lk_GraphicsProxyWidget_ = dynamic_cast<QGraphicsProxyWidget*>(lk_GraphicsWidget_);
	if (lk_GraphicsProxyWidget_ == NULL)
		return NULL;
	return dynamic_cast<IDesktopBox*>(lk_GraphicsProxyWidget_->widget());
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


void k_Desktop::updateArrowInternal(QGraphicsPathItem* ak_Arrow_, QPointF ak_Start, QPointF ak_End)
{
	QPainterPath lk_Path;
	lk_Path.moveTo(ak_Start);
	lk_Path.lineTo(ak_End);
	
	QPointF lk_Dir = ak_End - ak_Start;
	double ld_Length = sqrt(lk_Dir.x() * lk_Dir.x() + lk_Dir.y() * lk_Dir.y());
	if (ld_Length > 1.0)
	{
		lk_Dir /= ld_Length;
		QPointF lk_Normal = QPointF(-lk_Dir.y(), lk_Dir.x());
		QPolygonF lk_Arrow;
		lk_Arrow << ak_End;
		lk_Arrow << ak_End - lk_Dir * 10.0 + lk_Normal * 3.5;
		lk_Arrow << ak_End - lk_Dir * 7.0;
		lk_Arrow << ak_End - lk_Dir * 10.0 - lk_Normal * 3.5;
		lk_Arrow << ak_End;
		lk_Path.addPolygon(lk_Arrow);
	}
	
	ak_Arrow_->setPath(lk_Path);
}


QPainterPath k_Desktop::grownPathForBox(IDesktopBox* ak_Box_, int ai_Grow)
{
	QWidget* lk_Widget_ = dynamic_cast<QWidget*>(ak_Box_);
	lk_Widget_->ensurePolished();
	QPainterPath lk_Path;
	lk_Path.addRoundedRect(QRectF(
		lk_Widget_->x() - ai_Grow, lk_Widget_->y() - ai_Grow, 
		lk_Widget_->width() + ai_Grow * 2, lk_Widget_->height() + ai_Grow * 2), 
		8.0 + ai_Grow, 8.0 + ai_Grow);
	return lk_Path;
}

QPainterPath k_Desktop::grownPathForArrow(QGraphicsPathItem* ak_Arrow_, int ai_Grow)
{
	if (!mk_Arrows.contains(ak_Arrow_))
		return QPainterPath();
	
	QPainterPath lk_Path;
	IDesktopBox* lk_StartBox_ = mk_Arrows[ak_Arrow_].first;
	IDesktopBox* lk_EndBox_ = mk_Arrows[ak_Arrow_].second;
	
	QPointF p0 = boxLocation(lk_StartBox_);
	QPointF p1 = boxLocation(lk_EndBox_);
	
	QPointF lk_Dir = p1 - p0;
	double lf_Length = lk_Dir.x() * lk_Dir.x() + lk_Dir.y() * lk_Dir.y();
	if (lf_Length > 1.0)
	{
		lf_Length = sqrt(lf_Length);
		QPointF lk_Normal = QPointF(-lk_Dir.y(), lk_Dir.x()) / lf_Length;
		lk_Path.moveTo(p0 + lk_Normal * ai_Grow);
		lk_Path.lineTo(p1 + lk_Normal * ai_Grow);
		lk_Path.lineTo(p1 - lk_Normal * ai_Grow);
		lk_Path.lineTo(p0 - lk_Normal * ai_Grow);
		lk_Path.lineTo(p0 + lk_Normal * ai_Grow);
	}
	return lk_Path;
}


QPointF k_Desktop::findFreeSpace(QRectF ak_BoundRect, int ai_BoxCount, IDesktopBox* ak_Box_)
{
	QPointF lk_HalfSize = 
		QPointF((double)(dynamic_cast<k_DesktopBox*>(ak_Box_)->width()) * 0.5,
				 (double)(dynamic_cast<k_DesktopBox*>(ak_Box_)->height()) * 0.5);
				 
	if (ai_BoxCount == 0)
		return QPointF(10000.0, 10000.0) - lk_HalfSize;

	QRectF lk_Rect = ak_BoundRect.adjusted(-8.0, -8.0, 8.0, 8.0);
	double p[4] = {lk_Rect.top(), lk_Rect.right(), lk_Rect.bottom(), lk_Rect.left()};
	int best = 0;
	double bestd = fabs(p[0] - 10000.0);
	for (int i = 1; i < 4; ++i)
	{
		double d = fabs(p[i] - 10000.0);
		if (d < bestd)
		{
			bestd = d;
			best = i;
		}
	}
	if (best == 0)
		return QPointF(10000.0, lk_Rect.top() - lk_HalfSize.y()) - lk_HalfSize;
	if (best == 1)
		return QPointF(lk_Rect.right() + lk_HalfSize.x(), 10000.0) - lk_HalfSize;
	if (best == 2)
		return QPointF(10000.0, lk_Rect.bottom() + lk_HalfSize.y()) - lk_HalfSize;
	if (best == 3)
		return QPointF(lk_Rect.left() - lk_HalfSize.x(), 10000.0) - lk_HalfSize;
	return QPointF() - lk_HalfSize;
}
