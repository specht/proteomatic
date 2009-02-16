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


class k_PipelineMainWindow;


struct r_MouseMode
{
	enum Enumeration
	{
		Move, Arrow
	};
};


class k_Desktop: public QGraphicsView
{
	Q_OBJECT
public:
	k_Desktop(QWidget* ak_Parent_, k_Proteomatic& ak_Proteomatic, k_PipelineMainWindow& ak_PipelineMainWindow);
	virtual ~k_Desktop();

	typedef QPair<k_DesktopBox*, k_DesktopBox*> tk_BoxPair;
	typedef QSet<k_DesktopBox*> tk_DesktopBoxSet;
	typedef QSet<k_FileBox*> tk_FileBoxSet;
	typedef QSet<tk_BoxPair> tk_BoxPairSet;
	
	void addBox(k_DesktopBox* ak_Box_, k_DesktopBox* ak_CloseTo_ = NULL, QPoint* ak_Location_ = NULL);
	void removeBox(k_DesktopBox* ak_Box_, bool ab_DoSomeChecks = true);
	void connectBoxes(k_DesktopBox* ak_Box0_, k_DesktopBox* ak_Box1_);
	void disconnectBoxes(k_DesktopBox* ak_Box0_, k_DesktopBox* ak_Box1_);
	void arrowClick(k_DesktopBox* ak_Box_);
	bool boxSelected(k_DesktopBox* ak_Box_) const;
	bool arrowSelected(tk_BoxPair ak_BoxPair) const;
	tk_DesktopBoxSet selectedBoxes() const;
	tk_FileBoxSet fileBoxesForScriptBox(k_ScriptBox* ak_ScriptBox_) const;
	k_PipelineMainWindow& pipelineMainWindow();
	void start();
	void toggleUi();
	bool running();

public slots:
	void setMouseMode(r_MouseMode::Enumeration ae_MouseMode);
	void addScriptBox(QAction* ak_Action_);
	void addFileBox(QString as_Path);
	void boxMoved();
	void fileBoxArrowPressed();
	void fileBoxArrowReleased();
	void boxClicked(Qt::KeyboardModifiers ae_Modifiers);
	void forceRefresh();
	void forceReset();
	
protected slots:
	void scriptFinished();

protected:
	virtual void mousePressEvent(QMouseEvent* ak_Event_);
	virtual void mouseReleaseEvent(QMouseEvent* ak_Event_);
	virtual void mouseMoveEvent(QMouseEvent* ak_Event_);
	virtual void dragEnterEvent(QDragEnterEvent* ak_Event_);
	virtual void dragMoveEvent(QDragMoveEvent* ak_Event_);
	virtual void dropEvent(QDropEvent* ak_Event_);
	virtual void paintEvent(QPaintEvent* ak_Event_);
	virtual void wheelEvent(QWheelEvent* ak_Event_);
	virtual void keyPressEvent(QKeyEvent* ak_Event_);
	
	inline QPoint boxLocation(k_DesktopBox* ak_Box_) const;
	
	double intersect(QPointF p0, QPointF d0, QPointF p1, QPointF d1);
	virtual void boxConnector(k_DesktopBox* ak_Box0_, k_DesktopBox* ak_Box1_, QPointF& ak_Point0, QPointF& ak_Point1);
	QPointF intersectLineWithBox(const QPointF& ak_Point0, const QPointF& ak_Point1, k_DesktopBox* ak_Widget_);
	k_DesktopBox* boxAt(QPoint ak_Point) const;
	void updateBoxConnector(k_DesktopBox* ak_Box0_, k_DesktopBox* ak_Box1_);
	void updateUserArrow();
	void clearSelection();
	void addBoxToSelection(k_DesktopBox* ak_Box_);
	void removeBoxFromSelection(k_DesktopBox* ak_Box_);
	void addArrowToSelection(tk_BoxPair ak_BoxPair);
	void removeArrowFromSelection(tk_BoxPair ak_BoxPair);
	void resetScriptBoxes();
	void updateAllBoxes();
	void startNextScriptBox();

	QList<k_DesktopBox*> mk_Boxes;
	QHash<tk_BoxPair, RefPtr<QGraphicsPathItem> > mk_BoxConnections;
	QHash<QGraphicsPathItem*, RefPtr<QGraphicsLineItem> > mk_ProxyLineForArrow;
	QHash<QGraphicsLineItem*, QGraphicsPathItem*> mk_ArrowForProxyLine;
	QHash<QGraphicsLineItem*, tk_BoxPair> mk_BoxPairForProxyLine;
	QHash<k_DesktopBox*, tk_DesktopBoxSet> mk_BoxConnectionsForBox;
	QHash<k_ScriptBox*, tk_FileBoxSet> mk_FileBoxesForScriptBox;

	k_DesktopBox* mk_ArrowStartBox_;
	k_DesktopBox* mk_ArrowEndBox_;
	// this is the graphics item depicting an arrow that is currently being created
	QGraphicsPathItem* mk_ArrowPathItem_;
	QPointF mk_CurrentMousePosition;

	tk_DesktopBoxSet mk_SelectedBoxes;
	tk_BoxPairSet mk_SelectedArrows;

	r_MouseMode::Enumeration me_MouseMode;
	
	k_Proteomatic& mk_Proteomatic;
	k_PipelineMainWindow& mk_PipelineMainWindow;
	RefPtr<QGraphicsScene> mk_pGraphicsScene;
	
	double md_Scale;
	
	QSet<k_ScriptBox*> mk_RunningScriptBoxQueue;
};


