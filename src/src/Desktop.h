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
#include "StopWatch.h"
#include <math.h>
#include <stdlib.h>
#include "DesktopBox.h"
#include "Proteomatic.h"


struct r_MouseMode
{
	enum Enumeration
	{
		Move, Arrow
	};
};


class k_Desktop: public QWidget
{
	Q_OBJECT
public:
	k_Desktop(QWidget* ak_Parent_, k_Proteomatic& ak_Proteomatic);
	virtual ~k_Desktop();

	void addBox(k_DesktopBox* ak_Box_, k_DesktopBox* ak_CloseTo_ = NULL);
	void removeBox(k_DesktopBox* ak_Box_);
	void connectBoxes(k_DesktopBox* ak_Box0_, k_DesktopBox* ak_Box1_);
	void disconnectBoxes(k_DesktopBox* ak_Box0_, k_DesktopBox* ak_Box1_);
	void arrowClick(k_DesktopBox* ak_Box_);
	bool boxSelected(k_DesktopBox* ak_Box_) const;

	typedef QPair<k_DesktopBox*, k_DesktopBox*> tk_BoxPair;

public slots:
	void setMouseMode(r_MouseMode::Enumeration ae_MouseMode);
	void animate();
	void addScriptBox(QAction* ak_Action_);
	void enableAnimation(bool ab_Enable);

protected:

	virtual void mousePressEvent(QMouseEvent* ak_Event_);
	virtual void mouseReleaseEvent(QMouseEvent* ak_Event_);
	virtual void mouseMoveEvent(QMouseEvent* ak_Event_);
	virtual void dragEnterEvent(QDragEnterEvent* ak_Event_);
	virtual void dropEvent(QDropEvent* ak_Event_);
	virtual void paintEvent(QPaintEvent* ak_Event_);
	virtual void wheelEvent(QWheelEvent* ak_Event_);
	virtual void drawArrow(QPainter* ak_Painter_, k_DesktopBox* ak_Box0_, k_DesktopBox* ak_Box1_);
	virtual void drawArrow(QPainter* ak_Painter_, k_DesktopBox* ak_Box0_, QPoint ak_Point);
	double intersect(QPointF p0, QPointF d0, QPointF p1, QPointF d1);
	virtual void boxConnector(k_DesktopBox* ak_Box0_, k_DesktopBox* ak_Box1_, QPointF& ak_Point0, QPointF& ak_Point1);
	QPointF intersectLineWithBox(const QPointF& ak_Point0, const QPointF& ak_Point1, k_DesktopBox* ak_Widget_);
	double boxDistance(k_DesktopBox* ak_Box0_, k_DesktopBox* ak_Box1_);
	double smoothStep(double x) const;
	k_DesktopBox* boxAt(QPoint ak_Point) const;

	QList<k_DesktopBox*> mk_Boxes;
	QHash<k_DesktopBox*, QPointF> mk_BoxLocation;
	QHash<k_DesktopBox*, QPointF> mk_BoxVelocity;
	QHash<k_DesktopBox*, QPointF> mk_BoxAcceleration;
	k_StopWatch mk_StopWatch;
	double md_LastPaintTime;
	QHash<tk_BoxPair, double> mk_BoxConnections;

	// normalized direction vector between two boxes
	QHash<tk_BoxPair, QPointF> mk_BoxDirection;
	QHash<tk_BoxPair, double> mk_BoxDistance;
	QHash<tk_BoxPair, bool> mk_BoxOverlap;

	k_DesktopBox* mk_ArrowStartBox_;
	k_DesktopBox* mk_ArrowEndBox_;
	QTimer mk_AnimationTimer;
	bool mb_AnimationEnabled;
	double md_Scale;

	QSet<k_DesktopBox*> mk_SelectedBoxes;

	r_MouseMode::Enumeration me_MouseMode;
	bool mb_Moving;
	QPoint mk_MoveStart;
	QPainterPath mk_Lasso;
	QPainterPath mk_ActualLasso;
	QPoint mk_LastLassoPoint;
	
	k_Proteomatic& mk_Proteomatic;
};


