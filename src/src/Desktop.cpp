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
#include <math.h>


k_Desktop::k_Desktop(QWidget* ak_Parent_, k_Proteomatic& ak_Proteomatic)
	: QGraphicsView(ak_Parent_)
	, mk_ArrowStartBox_(NULL)
	, mk_ArrowEndBox_(NULL)
	, mk_ArrowPathItem_(NULL)
	, me_MouseMode(r_MouseMode::Move)
	, mk_Proteomatic(ak_Proteomatic)
{
	setAcceptDrops(true);
	mk_pGraphicsScene = RefPtr<QGraphicsScene>(new QGraphicsScene(ak_Parent_));
	this->setScene(mk_pGraphicsScene.get_Pointer());
	this->setRenderHint(QPainter::Antialiasing, true);
	this->setRenderHint(QPainter::TextAntialiasing, true);
	this->setRenderHint(QPainter::SmoothPixmapTransform, true);
	this->setBackgroundBrush(QBrush(QColor("#f8f8f8")));
	this->setSceneRect(-10000.0, -10000.0, 20000.0, 20000.0);
	this->setDragMode(QGraphicsView::ScrollHandDrag);
	this->translate(0.5, 0.5);
}


k_Desktop::~k_Desktop()
{
	foreach (k_DesktopBox* lk_Box_, mk_Boxes)
		this->removeBox(lk_Box_);
}


void k_Desktop::addBox(k_DesktopBox* ak_Box_, k_DesktopBox* ak_CloseTo_, QPoint* ak_Location_)
{
	mk_Boxes.push_back(ak_Box_);
	if (ak_CloseTo_)
		ak_Box_->move(ak_CloseTo_->pos() + QPoint(ak_CloseTo_->width() / 2, ak_CloseTo_->height() + 64));
	if (ak_Location_)
		ak_Box_->move(*ak_Location_);
	
	mk_BoxConnectionsForBox[ak_Box_] = tk_DesktopBoxSet();
	
	if (dynamic_cast<k_ScriptBox*>(ak_Box_) != NULL)
		mk_FileBoxesForScriptBox[dynamic_cast<k_ScriptBox*>(ak_Box_)] = tk_FileBoxSet();
	
	mk_pGraphicsScene->addWidget(ak_Box_);
	ak_Box_->move(ak_Box_->pos() - QPoint(ak_Box_->size().width(), ak_Box_->size().height()) / 2);
	connect(ak_Box_, SIGNAL(moved()), this, SLOT(boxMoved()));
	connect(ak_Box_, SIGNAL(resized()), this, SLOT(boxMoved()));
	k_FileBox* lk_FileBox_ = dynamic_cast<k_FileBox*>(ak_Box_);
	if (lk_FileBox_ != NULL)
	{
		connect(lk_FileBox_, SIGNAL(arrowPressed()), this, SLOT(fileBoxArrowPressed()));
		connect(lk_FileBox_, SIGNAL(arrowReleased()), this, SLOT(fileBoxArrowReleased()));
	}
	connect(ak_Box_, SIGNAL(mousePressed(Qt::KeyboardModifiers)), this, SLOT(boxClicked(Qt::KeyboardModifiers)));
}


void k_Desktop::removeBox(k_DesktopBox* ak_Box_, bool ab_DoSomeChecks)
{
	if (!mk_Boxes.contains(ak_Box_))
		return;

	if (ab_DoSomeChecks)
	{
		// if it's a script box, delete all associated output file boxes
		if (dynamic_cast<k_ScriptBox*>(ak_Box_))
		{
			QList<k_OutputFileBox*> lk_OutputFileBoxes = dynamic_cast<k_ScriptBox*>(ak_Box_)->outputFileBoxes();
			foreach (k_OutputFileBox* lk_Box_, lk_OutputFileBoxes)
				this->removeBox(lk_Box_);
		}
		
		// if it's an output file box, let the script box remove it
		k_OutputFileBox* lk_OutputFileBox_ = dynamic_cast<k_OutputFileBox*>(ak_Box_);
		if (lk_OutputFileBox_)
		{
			// determine the appropriate script box for this output file box
			foreach (k_DesktopBox* lk_Box_, mk_BoxConnectionsForBox[ak_Box_])
			{
				k_ScriptBox* lk_ScriptBox_ = dynamic_cast<k_ScriptBox*>(lk_Box_);
				if (lk_ScriptBox_)
				{
					if (lk_ScriptBox_->outputFileBoxes().contains(lk_OutputFileBox_))
					{
						lk_ScriptBox_->removeOutputFileBox(lk_OutputFileBox_);
						return;
					}
				}
			}
		}
	}
	
	ak_Box_->disconnect();
		
	QList<tk_BoxPair> lk_DeletePairs;
	foreach (tk_BoxPair lk_BoxPair, mk_BoxConnections.keys())
		if (lk_BoxPair.first == ak_Box_ || lk_BoxPair.second == ak_Box_)
			lk_DeletePairs.push_back(lk_BoxPair);
	foreach (tk_BoxPair lk_BoxPair, lk_DeletePairs)
		this->disconnectBoxes(lk_BoxPair.first, lk_BoxPair.second);

	mk_Boxes.removeAll(ak_Box_);
	mk_SelectedBoxes.remove(ak_Box_);
	// TODO: maybe we should delete not only the entries for this box,
	// but also remove the box from all sets in which it is contained?!
	mk_BoxConnectionsForBox.remove(ak_Box_);
	if (dynamic_cast<k_ScriptBox*>(ak_Box_) != NULL)
		mk_FileBoxesForScriptBox.remove(dynamic_cast<k_ScriptBox*>(ak_Box_));

	delete ak_Box_;
}


void k_Desktop::connectBoxes(k_DesktopBox* ak_Box0_, k_DesktopBox* ak_Box1_)
{
	if (ak_Box0_ == ak_Box1_)
		return;
	
	QPen lk_Pen(QColor("#888a85"));
	lk_Pen.setWidth(1);
	
	QGraphicsPathItem* lk_GraphicsPathItem_ = 
		mk_pGraphicsScene->addPath(QPainterPath(), lk_Pen, QBrush(QColor("#888a85")));
	lk_GraphicsPathItem_->setZValue(-1.0);
	mk_BoxConnections.insert(tk_BoxPair(ak_Box0_, ak_Box1_), RefPtr<QGraphicsPathItem>(lk_GraphicsPathItem_));
	mk_BoxConnectionsForBox[ak_Box0_].insert(ak_Box1_);
	mk_BoxConnectionsForBox[ak_Box1_].insert(ak_Box0_);
	
	if (dynamic_cast<k_FileBox*>(ak_Box0_) != NULL &&
		dynamic_cast<k_ScriptBox*>(ak_Box1_) != NULL)
	{
		mk_FileBoxesForScriptBox[dynamic_cast<k_ScriptBox*>(ak_Box1_)].insert(dynamic_cast<k_FileBox*>(ak_Box0_));
		dynamic_cast<k_ScriptBox*>(ak_Box1_)->fileBoxConnected(dynamic_cast<k_FileBox*>(ak_Box0_));
	}
	
	this->updateBoxConnector(ak_Box0_, ak_Box1_);
}


void k_Desktop::disconnectBoxes(k_DesktopBox* ak_Box0_, k_DesktopBox* ak_Box1_)
{
	if (ak_Box0_ == ak_Box1_)
		return;
	mk_BoxConnections.remove(tk_BoxPair(ak_Box0_, ak_Box1_));
	mk_BoxConnectionsForBox[ak_Box0_].remove(ak_Box1_);
	mk_BoxConnectionsForBox[ak_Box1_].remove(ak_Box0_);
	
	if (dynamic_cast<k_ScriptBox*>(ak_Box1_) != NULL &&
		dynamic_cast<k_FileBox*>(ak_Box0_) != NULL)
	{
		mk_FileBoxesForScriptBox[dynamic_cast<k_ScriptBox*>(ak_Box1_)].remove(dynamic_cast<k_FileBox*>(ak_Box0_));
		dynamic_cast<k_ScriptBox*>(ak_Box1_)->fileBoxDisconnected(dynamic_cast<k_FileBox*>(ak_Box0_));
	}
}


void k_Desktop::arrowClick(k_DesktopBox* ak_Box_)
{
	if (mk_ArrowStartBox_ == NULL)
	{
		mk_ArrowStartBox_ = ak_Box_;
	}
	else
	{
		connectBoxes(mk_ArrowStartBox_, ak_Box_);
		mk_ArrowStartBox_ = NULL;
	}
}


bool k_Desktop::boxSelected(k_DesktopBox* ak_Box_) const
{
	return mk_SelectedBoxes.contains(ak_Box_);
}


k_Desktop::tk_FileBoxSet k_Desktop::fileBoxesForScriptBox(k_ScriptBox* ak_ScriptBox_) const
{
	if (!mk_FileBoxesForScriptBox.contains(ak_ScriptBox_))
		return tk_FileBoxSet();
	return mk_FileBoxesForScriptBox[ak_ScriptBox_];
}


void k_Desktop::setMouseMode(r_MouseMode::Enumeration ae_MouseMode)
{
	me_MouseMode = ae_MouseMode;
}


void k_Desktop::addScriptBox(QAction* ak_Action_)
{
	k_ScriptBox* lk_ScriptBox_ = new k_ScriptBox(ak_Action_->data().toString(), this, mk_Proteomatic);
	this->addBox(lk_ScriptBox_);
}


void k_Desktop::boxMoved()
{
	k_DesktopBox* lk_Box_ = dynamic_cast<k_DesktopBox*>(sender());
	if (lk_Box_ == NULL)
		return;
	
	// update connections between boxes
	foreach (k_DesktopBox* lk_OtherBox_, mk_BoxConnectionsForBox[lk_Box_])
		this->updateBoxConnector(lk_Box_, lk_OtherBox_);
}


void k_Desktop::fileBoxArrowPressed()
{
	k_FileBox* lk_FileBox_ = dynamic_cast<k_FileBox*>(sender());
	if (lk_FileBox_ == NULL)
		return;
	
	mk_ArrowStartBox_ = lk_FileBox_;
	mk_ArrowEndBox_ = NULL;
	QPen lk_Pen(QColor("#888a85"));
	lk_Pen.setWidth(1);
	mk_ArrowPathItem_ = mk_pGraphicsScene->
		addPath(QPainterPath(), lk_Pen, QBrush(QColor("#888a85")));
}


void k_Desktop::fileBoxArrowReleased()
{
	k_FileBox* lk_FileBox_ = dynamic_cast<k_FileBox*>(sender());
	if (lk_FileBox_ == NULL)
		return;
	
	if (mk_ArrowEndBox_ != NULL)
		this->connectBoxes(mk_ArrowStartBox_, mk_ArrowEndBox_);

	mk_ArrowStartBox_ = NULL;
	mk_ArrowEndBox_ = NULL;
	if (mk_ArrowPathItem_)
		delete mk_ArrowPathItem_;
}


void k_Desktop::boxClicked(Qt::KeyboardModifiers ae_Modifiers)
{
	k_DesktopBox* lk_Box_ = dynamic_cast<k_DesktopBox*>(sender());
	if (!lk_Box_)
		return;
	
	if ((ae_Modifiers & Qt::ControlModifier) == 0)
	{
		// no ctrl pressed
		this->clearSelection();
		this->addBoxToSelection(lk_Box_);
	}
	else
	{
		// ctrl pressed
		if (this->boxSelected(lk_Box_))
			this->removeBoxFromSelection(lk_Box_);
		else
			this->addBoxToSelection(lk_Box_);
	}
}


void k_Desktop::mousePressEvent(QMouseEvent* ak_Event_)
{
	if ((ak_Event_->modifiers() & Qt::ControlModifier) == 0)
		this->clearSelection();
	
	QGraphicsView::mousePressEvent(ak_Event_);
	return;
	
	/*
	ak_Event_->accept();
	k_DesktopBox* lk_Box_ = boxAt(ak_Event_->pos());
	if (lk_Box_ != NULL)
	{
		if (me_MouseMode == r_MouseMode::Move)
		{
			mb_Moving = true;
			mk_MoveStart = ak_Event_->pos();
			if ((ak_Event_->modifiers() & Qt::ControlModifier) != Qt::ControlModifier)
			{
				if (!mk_SelectedBoxes.contains(lk_Box_))
				{
					mk_SelectedBoxes.clear();
					mk_SelectedBoxes.insert(lk_Box_);
				}
			}
			else
			{
				if (boxSelected(lk_Box_))
					mk_SelectedBoxes.remove(lk_Box_);
				else
					mk_SelectedBoxes.insert(lk_Box_);
			}
		}
		else if (me_MouseMode == r_MouseMode::Arrow)
		{
			mk_ArrowStartBox_ = lk_Box_;
			mk_ArrowEndBox_ = NULL;
		}
	}
	else
	{
		if (me_MouseMode == r_MouseMode::Move)
		{
			mk_SelectedBoxes.clear();
			mk_Lasso = QPainterPath();
			mk_Lasso.moveTo(ak_Event_->pos());
			mk_LastLassoPoint = ak_Event_->pos();
			mk_ActualLasso = mk_Lasso;
			mk_ActualLasso.lineTo(mapFromGlobal(QCursor::pos()));
			mk_ActualLasso.closeSubpath();
		}
	}
	animate();
	*/
}


void k_Desktop::mouseReleaseEvent(QMouseEvent* ak_Event_)
{
	/*
	if (mk_ArrowStartBox_ != NULL)
	{
		if (mk_ArrowEndBox_ != NULL)
			this->connectBoxes(mk_ArrowStartBox_, mk_ArrowEndBox_);
		mk_ArrowStartBox_ = NULL;
		mk_ArrowEndBox_ = NULL;
		if (mk_ArrowPathItem_)
			delete mk_ArrowPathItem_;
	}
	*/

	QGraphicsView::mouseReleaseEvent(ak_Event_);
	/*
	ak_Event_->accept();
	k_DesktopBox* lk_Box_ = boxAt(ak_Event_->pos());
	if (lk_Box_ != NULL)
	{
		mb_Moving = false;
		if (me_MouseMode == r_MouseMode::Arrow)
		{
			if (mk_ArrowStartBox_ != NULL && mk_ArrowEndBox_ != NULL)
				connectBoxes(mk_ArrowStartBox_, mk_ArrowEndBox_);
		}
	}
	mk_ArrowStartBox_ = NULL;
	mk_ArrowEndBox_ = NULL;
	mk_Lasso = QPainterPath();
	animate();
	*/
}


void k_Desktop::mouseMoveEvent(QMouseEvent* ak_Event_)
{
	if (mk_ArrowStartBox_ != NULL)
	{
		mk_CurrentMousePosition = this->mapToScene(ak_Event_->pos());
		mk_ArrowEndBox_ = NULL;
		k_DesktopBox* lk_Box_ = this->boxAt(mk_CurrentMousePosition.toPoint());
		if (lk_Box_ != NULL)
		{
			k_ScriptBox* lk_ScriptBox_ = dynamic_cast<k_ScriptBox*>(lk_Box_);
			if (lk_ScriptBox_ != NULL)
			{
				mk_ArrowEndBox_ = lk_ScriptBox_;
				// check if arrow just goes back: NOT GOOD
				if (mk_ArrowEndBox_ != NULL &&
					mk_BoxConnectionsForBox.contains(mk_ArrowEndBox_) &&
					mk_BoxConnectionsForBox[mk_ArrowEndBox_].contains(mk_ArrowStartBox_))
					mk_ArrowEndBox_ = NULL;
				// check if arrow already exists between these boxes: NOT GOOD
				if (mk_ArrowEndBox_ != NULL &&
					mk_BoxConnectionsForBox.contains(mk_ArrowStartBox_) &&
					mk_BoxConnectionsForBox[mk_ArrowStartBox_].contains(mk_ArrowEndBox_))
					mk_ArrowEndBox_ = NULL;
			}
		}
		this->updateUserArrow();
		ak_Event_->accept();
	} 
	else
		QGraphicsView::mouseMoveEvent(ak_Event_);

	/*
	ak_Event_->accept();
	if (me_MouseMode == r_MouseMode::Move)
	{
		if (mb_Moving)
		{
			QPoint lk_Delta = ak_Event_->pos() - mk_MoveStart;
			foreach (k_DesktopBox* lk_Box_, mk_SelectedBoxes)
				lk_Box_->move(lk_Box_->pos() + lk_Delta);
			mk_MoveStart = ak_Event_->pos();
		}
		else
		{
			if ((mk_LastLassoPoint - ak_Event_->pos()).manhattanLength() > 8)
			{
				mk_Lasso.lineTo(ak_Event_->pos());
				mk_LastLassoPoint = ak_Event_->pos();
			}
			mk_ActualLasso = mk_Lasso;
			mk_ActualLasso.lineTo(mapFromGlobal(QCursor::pos()));
			mk_ActualLasso.closeSubpath();
			mk_SelectedBoxes.clear();
			foreach (k_DesktopBox* lk_Box_, mk_Boxes)
			{
				if (mk_ActualLasso.intersects(lk_Box_->frameGeometry()))
					mk_SelectedBoxes.insert(lk_Box_);
			}
		}
	}
	else if (me_MouseMode == r_MouseMode::Arrow)
	{
		if (mk_ArrowStartBox_ != NULL)
		{
			k_DesktopBox* lk_Box_ = boxAt(ak_Event_->pos());
			if (lk_Box_ == mk_ArrowStartBox_)
				lk_Box_ = NULL;
			mk_ArrowEndBox_ = lk_Box_;
		}
	}
	animate();
	*/
}


void k_Desktop::dragEnterEvent(QDragEnterEvent* ak_Event_)
{
	ak_Event_->acceptProposedAction();
}


void k_Desktop::dragMoveEvent(QDragMoveEvent* ak_Event_)
{
	ak_Event_->acceptProposedAction();
}


void k_Desktop::dropEvent(QDropEvent* ak_Event_)
{
	ak_Event_->acceptProposedAction();
	foreach (QUrl lk_Url, ak_Event_->mimeData()->urls())
	{
		QString ls_Path = lk_Url.toLocalFile();
		if (ls_Path != "")
		{
			QFileInfo lk_FileInfo(ls_Path);
			if (!lk_FileInfo.isDir())
			{
				k_InputFileBox* lk_InputFileBox_ = new k_InputFileBox(this, mk_Proteomatic);
				lk_InputFileBox_->setFilename(ls_Path);
				QPoint lk_Position = this->mapToScene(ak_Event_->pos()).toPoint();
				addBox(lk_InputFileBox_, NULL, &lk_Position);
			}
		}
	}
}


void k_Desktop::paintEvent(QPaintEvent* ak_Event_)
{
	QGraphicsView::paintEvent(ak_Event_);
	return;
	
	/*
	QPainter lk_Painter(this);
	lk_Painter.fillRect(0, 0, width(), height(), QBrush(QColor("#eeeeec")));
	lk_Painter.setRenderHints(QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform | QPainter::Antialiasing, true);

	if (!mk_Lasso.isEmpty())
	{
		lk_Painter.setPen(Qt::SolidLine);
		lk_Painter.setPen(QColor("#888a85"));
		lk_Painter.setBrush(QBrush(QColor("#d3d7cf")));
		lk_Painter.drawPath(mk_ActualLasso);
	}

	lk_Painter.setPen(Qt::SolidLine);
	lk_Painter.setPen(QColor::fromRgb(128, 128, 128));
	QObjectList lk_Objects = this->children();
	foreach (tk_BoxPair lk_Pair, mk_BoxConnections.keys())
		drawArrow(&lk_Painter, lk_Pair.first, lk_Pair.second);

	if (me_MouseMode == r_MouseMode::Arrow && mk_ArrowStartBox_ != NULL)
	{
		if (mk_ArrowEndBox_ == NULL)
			drawArrow(&lk_Painter, mk_ArrowStartBox_, mapFromGlobal(QCursor::pos()));
		else
			drawArrow(&lk_Painter, mk_ArrowStartBox_, mk_ArrowEndBox_);
	}
	*/
}


void k_Desktop::wheelEvent(QWheelEvent* ak_Event_)
{
	if ((ak_Event_->modifiers() & Qt::ControlModifier) != 0)
	{
		double ld_ScaleDelta = pow(1.1, fabs(ak_Event_->delta() / 100.0));
		if (ak_Event_->delta() < 0)
			ld_ScaleDelta = 1.0 / ld_ScaleDelta;
		this->scale(ld_ScaleDelta, ld_ScaleDelta);
	}
}


void k_Desktop::keyPressEvent(QKeyEvent* ak_Event_)
{
	if (ak_Event_->key() == Qt::Key_Delete)
	{
		foreach (k_DesktopBox* lk_Box_, mk_SelectedBoxes)
			this->removeBox(lk_Box_);
	}
	QGraphicsView::keyPressEvent(ak_Event_);
}


inline QPoint k_Desktop::boxLocation(k_DesktopBox* ak_Box_) const
{
	return ak_Box_->pos() + QPoint(ak_Box_->width(), ak_Box_->height()) / 2;
}


void k_Desktop::drawArrow(QPainter* ak_Painter_, k_DesktopBox* ak_Box0_, k_DesktopBox* ak_Box1_)
{
	/*
	QPointF lk_Point0, lk_Point1;
	QPointF lk_Dir = mk_BoxLocation[ak_Box1_] - mk_BoxLocation[ak_Box0_];
	double ld_Length = sqrt(lk_Dir.x() * lk_Dir.x() + lk_Dir.y() * lk_Dir.y());
	lk_Dir /= ld_Length;
	QPointF lk_Normal = QPointF(-lk_Dir.y(), lk_Dir.x());
	boxConnector(ak_Box0_, ak_Box1_, lk_Point0, lk_Point1);
	ak_Painter_->drawLine(lk_Point0, lk_Point1);
	QPolygonF lk_Arrow;
	lk_Arrow << lk_Point1;
	lk_Arrow << lk_Point1 - lk_Dir * 10.0 + lk_Normal * 3.5;
	lk_Arrow << lk_Point1 - lk_Dir * 7.0;
	lk_Arrow << lk_Point1 - lk_Dir * 10.0 - lk_Normal * 3.5;
	lk_Arrow << lk_Point1;
	ak_Painter_->setBrush(QBrush(ak_Painter_->pen().color()));
	ak_Painter_->drawPolygon(lk_Arrow);
	*/
}


void k_Desktop::drawArrow(QPainter* ak_Painter_, k_DesktopBox* ak_Box0_, QPoint ak_Point)
{
	/*
	QPointF lk_Point0 = mk_BoxLocation[ak_Box0_];
	QPointF lk_Point1(ak_Point);
	QPointF lk_Dir = lk_Point1 - lk_Point0;
	double ld_Length = sqrt(lk_Dir.x() * lk_Dir.x() + lk_Dir.y() * lk_Dir.y());
	if (fabs(ld_Length) < 0.0001)
		return;
	lk_Dir /= ld_Length;
	QPointF lk_Normal = QPointF(-lk_Dir.y(), lk_Dir.x());
	ak_Painter_->drawLine(lk_Point0, lk_Point1);
	QPolygonF lk_Arrow;
	lk_Arrow << lk_Point1;
	lk_Arrow << lk_Point1 - lk_Dir * 10.0 + lk_Normal * 3.5;
	lk_Arrow << lk_Point1 - lk_Dir * 7.0;
	lk_Arrow << lk_Point1 - lk_Dir * 10.0 - lk_Normal * 3.5;
	lk_Arrow << lk_Point1;
	ak_Painter_->setBrush(QBrush(ak_Painter_->pen().color()));
	ak_Painter_->drawPolygon(lk_Arrow);
	*/
}


double k_Desktop::intersect(QPointF p0, QPointF d0, QPointF p1, QPointF d1)
{
	double d = d0.x() * d1.y() - d0.y() * d1.x();
	if (fabs(d) < 0.000001)
		return 0.0;
	return (d1.x() * (p0.y() - p1.y()) + d1.y() * (p1.x() - p0.x())) / d;
}


void k_Desktop::boxConnector(k_DesktopBox* ak_Box0_, k_DesktopBox* ak_Box1_, QPointF& ak_Point0, QPointF& ak_Point1)
{
	QPointF lk_Point0 = boxLocation(ak_Box0_);
	QPointF lk_Point1 = boxLocation(ak_Box1_);
	ak_Point0 = intersectLineWithBox(lk_Point1, lk_Point0, ak_Box0_);
	ak_Point1 = intersectLineWithBox(lk_Point0, lk_Point1, ak_Box1_);
}


QPointF k_Desktop::intersectLineWithBox(const QPointF& ak_Point0, const QPointF& ak_Point1, k_DesktopBox* ak_Box_)
{
	QPointF lk_Dir = ak_Point1 - ak_Point0;
	QPointF lk_Quadrant = QPointF(fabs(lk_Dir.x()), fabs(lk_Dir.y()));
	QPointF lk_Diagonal = QPointF(ak_Box_->width(), ak_Box_->height());
	double t = 1.0;
	if (lk_Quadrant.x() * lk_Diagonal.y() > lk_Quadrant.y() * lk_Diagonal.x())
	{
		QPointF lk_Quadrant = ak_Point0 - ak_Point1;
		if (lk_Quadrant.x() < 0)
			t = intersect(ak_Point0, lk_Dir, boxLocation(ak_Box_) - QPointF(ak_Box_->width(), ak_Box_->height()) * 0.5, QPointF(0, ak_Box_->height()));
		else
			t = intersect(ak_Point0, lk_Dir, boxLocation(ak_Box_) - QPointF(ak_Box_->width(), ak_Box_->height()) * 0.5 + QPointF(ak_Box_->width(), 0), QPointF(0, ak_Box_->height()));
	}
	else
	{
		QPointF lk_Quadrant = ak_Point0 - ak_Point1;
		if (lk_Quadrant.y() < 0)
			t = intersect(ak_Point0, lk_Dir, boxLocation(ak_Box_) - QPointF(ak_Box_->width(), ak_Box_->height()) * 0.5, QPointF(ak_Box_->width(), 0));
		else
			t = intersect(ak_Point0, lk_Dir, boxLocation(ak_Box_) - QPointF(ak_Box_->width(), ak_Box_->height()) * 0.5 + QPointF(0, ak_Box_->height()), QPointF(ak_Box_->width(), 0));
	}
	
	return ak_Point0 + lk_Dir * t;
}


k_DesktopBox* k_Desktop::boxAt(QPoint ak_Point) const
{
	QGraphicsItem* lk_GraphicsItem_ = mk_pGraphicsScene->itemAt(QPointF() + ak_Point);
	QGraphicsWidget* lk_GraphicsWidget_ = dynamic_cast<QGraphicsWidget*>(lk_GraphicsItem_);
	if (lk_GraphicsWidget_ == NULL)
		return NULL;
	QGraphicsProxyWidget* lk_GraphicsProxyWidget_ = dynamic_cast<QGraphicsProxyWidget*>(lk_GraphicsWidget_);
	if (lk_GraphicsProxyWidget_ == NULL)
		return NULL;
	return dynamic_cast<k_DesktopBox*>(lk_GraphicsProxyWidget_->widget());
}


void k_Desktop::updateBoxConnector(k_DesktopBox* ak_Box0_, k_DesktopBox* ak_Box1_)
{
	tk_BoxPair lk_BoxPair(ak_Box0_, ak_Box1_);
	
	// it might be the other way around! because the connector is directed
	if (!mk_BoxConnections.contains(lk_BoxPair))
		lk_BoxPair = tk_BoxPair(ak_Box1_, ak_Box0_);

	// oops, better return here, should not happen, but who knows
	if (!mk_BoxConnections.contains(lk_BoxPair))
		return;
	
	QGraphicsPathItem* lk_PathItem_ = mk_BoxConnections[lk_BoxPair].get_Pointer();
	QPainterPath lk_Path;
	QPointF lk_Start, lk_End;
	boxConnector(lk_BoxPair.first, lk_BoxPair.second, lk_Start, lk_End);
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


void k_Desktop::updateUserArrow()
{
	if (mk_ArrowPathItem_ == NULL || mk_ArrowStartBox_ == NULL)
		return;
	
	QGraphicsPathItem* lk_PathItem_ = mk_ArrowPathItem_;
	QPainterPath lk_Path;
	
	QPointF lk_Start = boxLocation(mk_ArrowStartBox_);
	QPointF lk_End = (mk_ArrowEndBox_ == NULL) ? mk_CurrentMousePosition : boxLocation(mk_ArrowEndBox_);
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


void k_Desktop::clearSelection()
{
	foreach (k_DesktopBox* lk_Box_, mk_SelectedBoxes)
		this->removeBoxFromSelection(lk_Box_);
}


void k_Desktop::addBoxToSelection(k_DesktopBox* ak_Box_)
{
	mk_SelectedBoxes.insert(ak_Box_);
	ak_Box_->update();
}


void k_Desktop::removeBoxFromSelection(k_DesktopBox* ak_Box_)
{
	mk_SelectedBoxes.remove(ak_Box_);
	ak_Box_->update();
}
