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

#pragma once

#include <QtGui>
#include "IDesktopBox.h"


class k_PipelineMainWindow;
class k_Proteomatic;


typedef QPair<IDesktopBox*, IDesktopBox*> tk_BoxPair;


class k_Desktop: public QGraphicsView
{
	Q_OBJECT
public:
	k_Desktop(QWidget* ak_Parent_, k_Proteomatic& ak_Proteomatic, k_PipelineMainWindow& ak_PipelineMainWindow);
	virtual ~k_Desktop();

	virtual k_PipelineMainWindow& pipelineMainWindow() const;
	virtual QGraphicsScene& graphicsScene();
	virtual void addInputFileListBox();
	virtual void addScriptBox(const QString& as_ScriptUri);
	virtual void addBox(IDesktopBox* ak_Box_);
	virtual void removeBox(IDesktopBox* ak_Box_);
	virtual void connectBoxes(IDesktopBox* ak_Source_, IDesktopBox* ak_Destination_);
	virtual void disconnectBoxes(IDesktopBox* ak_Source_, IDesktopBox* ak_Destination_);
	virtual void moveBox(IDesktopBox* ak_Box_, QPoint ak_Delta);
	
	virtual void createFilenameTags(QStringList ak_Filenames, QHash<QString, QString>& ak_TagForFilename, QString& as_PrefixWithoutTags);
	
public slots:
	virtual void redraw();
	
protected slots:
	virtual void boxMovedOrResized(QPoint ak_Delta = QPoint());
	virtual void boxClicked(Qt::KeyboardModifiers ae_Modifiers);
	virtual void arrowPressed();
	virtual void arrowReleased();
	virtual void boxBatchModeChanged(bool ab_Enabled);
	virtual void updateArrow(QGraphicsPathItem* ak_Arrow_);
	virtual void redrawSelection();
	virtual void deleteSelected();
	virtual void redrawBatchFrame();
	virtual void clearSelection();
	
protected:
	virtual void keyPressEvent(QKeyEvent* event);
	virtual void mousePressEvent(QMouseEvent* event);
	virtual void mouseMoveEvent(QMouseEvent* event);
	virtual void wheelEvent(QWheelEvent* event);
	virtual void updateUserArrow(QPointF ak_MousePosition);
	virtual QPoint boxLocation(IDesktopBox* ak_Box_) const;
	virtual IDesktopBox* boxAt(QPointF ak_Point) const;
	virtual double intersect(QPointF p0, QPointF d0, QPointF p1, QPointF d1);
	virtual void boxConnector(IDesktopBox* ak_Box0_, IDesktopBox* ak_Box1_, QPointF& ak_Point0, QPointF& ak_Point1);
	virtual QPointF intersectLineWithBox(const QPointF& ak_Point0, const QPointF& ak_Point1, IDesktopBox* ak_Box_);
	virtual void updateArrowInternal(QGraphicsPathItem* ak_Arrow_, QPointF ak_Start, QPointF ak_End);
	virtual QPainterPath grownPathForBox(IDesktopBox* ak_Box_, int ai_Grow);
	virtual QPainterPath grownPathForArrow(QGraphicsPathItem* ak_Arrow_, int ai_Grow);
	virtual QPointF findFreeSpace(QRectF ak_BoundRect, int ai_BoxCount, IDesktopBox* ak_Box_);
	
	k_Proteomatic& mk_Proteomatic;
	k_PipelineMainWindow& mk_PipelineMainWindow;
	QGraphicsScene mk_GraphicsScene;
	
	double md_Scale;
	
	QSet<IDesktopBox*> mk_Boxes;

	// an arrow in the making!
	IDesktopBox* mk_ArrowStartBox_;
	IDesktopBox* mk_ArrowEndBox_;
	QGraphicsPathItem* mk_UserArrowPathItem_;
	
	// all arrows are kept in this hash
	QHash<QGraphicsPathItem*, tk_BoxPair> mk_Arrows;
	// secondary hash, reverse
	QHash<tk_BoxPair, QGraphicsPathItem*> mk_ArrowForBoxPair;
	// secondary hash, all arrows for each box
	QHash<IDesktopBox*, QSet<QGraphicsPathItem* > > mk_ArrowsForBox;
	// and a think invisible line under each arrow for picking!
	QHash<QGraphicsPathItem*, QGraphicsLineItem*> mk_ArrowProxy;
	QHash<QGraphicsLineItem*, QGraphicsPathItem*> mk_ArrowForProxy;
	
	QSet<IDesktopBox*> mk_SelectedBoxes;
	QSet<QGraphicsPathItem*> mk_SelectedArrows;
	QGraphicsPathItem* mk_SelectionGraphicsPathItem_;

	QSet<IDesktopBox*> mk_BatchBoxes;
	QGraphicsPathItem* mk_BatchGraphicsPathItem_;
};
