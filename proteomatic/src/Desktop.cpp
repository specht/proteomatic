#include "Desktop.h"
#include <math.h>


k_Desktop::k_Desktop(QWidget* ak_Parent_, k_Proteomatic& ak_Proteomatic)
	: QWidget(ak_Parent_)
	, mk_ArrowStartBox_(NULL)
	, mk_ArrowEndBox_(NULL)
	, mb_AnimationEnabled(false)
	, mk_AnimationTimer(this)
	, md_Scale(1.0)
	, me_MouseMode(r_MouseMode::Move)
	, mb_Moving(false)
	, mk_Proteomatic(ak_Proteomatic)
{
	setAcceptDrops(true);
	mk_StopWatch.reset();
	md_LastPaintTime = mk_StopWatch.get_Time();
	mk_AnimationTimer.setInterval(20);
	connect(&mk_AnimationTimer, SIGNAL(timeout()), this, SLOT(animate()));
}


k_Desktop::~k_Desktop()
{
}


void k_Desktop::addBox(k_DesktopBox* ak_Box_, k_DesktopBox* ak_CloseTo_)
{
	mk_Boxes.push_back(ak_Box_);
	mk_BoxLocation[ak_Box_] = QPointF((double)width() * 0.5 + (((double)rand() / RAND_MAX) - 0.5) * 100.0, 
		(double)height() * 0.5 + (((double)rand() / RAND_MAX) - 0.5) * 100.0);
	if (ak_CloseTo_ != NULL)
	{
		mk_BoxLocation[ak_Box_] = mk_BoxLocation[ak_CloseTo_] + QPointF(0.0, ak_CloseTo_->height() / 2 + ak_Box_->height() / 2);
	}
	mk_BoxVelocity[ak_Box_] = QPointF(0.0, 0.0);
	mk_BoxAcceleration[ak_Box_] = QPointF(0.0, 0.0);
	ak_Box_->scale(md_Scale);
	ak_Box_->move(mk_BoxLocation[ak_Box_].x() - ak_Box_->width() / 2, mk_BoxLocation[ak_Box_].y() - ak_Box_->height() / 2);
	ak_Box_->show();
	mk_SelectedBoxes.clear();
	mk_SelectedBoxes.insert(ak_Box_);
	update();
}


void k_Desktop::removeBox(k_DesktopBox* ak_Box_)
{
	if (mk_Boxes.contains(ak_Box_))
	{
		QList<tk_BoxPair> lk_DeletePairs;
		foreach (tk_BoxPair lk_BoxPair, mk_BoxConnections.keys())
			if (lk_BoxPair.first == ak_Box_ || lk_BoxPair.second == ak_Box_)
				lk_DeletePairs.push_back(lk_BoxPair);
		foreach (tk_BoxPair lk_BoxPair, lk_DeletePairs)
			mk_BoxConnections.remove(lk_BoxPair);

		mk_Boxes.removeAll(ak_Box_);
		mk_SelectedBoxes.remove(ak_Box_);

		update();

		delete ak_Box_;
	}
}


void k_Desktop::connectBoxes(k_DesktopBox* ak_Box0_, k_DesktopBox* ak_Box1_)
{
	if (ak_Box0_ == ak_Box1_)
		return;
	mk_BoxConnections.insert(QPair<k_DesktopBox*, k_DesktopBox*>(ak_Box0_, ak_Box1_), 60.0);
}


void k_Desktop::disconnectBoxes(k_DesktopBox* ak_Box0_, k_DesktopBox* ak_Box1_)
{
	if (ak_Box0_ == ak_Box1_)
		return;
	mk_BoxConnections.remove(tk_BoxPair(ak_Box0_, ak_Box1_));
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


void k_Desktop::setMouseMode(r_MouseMode::Enumeration ae_MouseMode)
{
	me_MouseMode = ae_MouseMode;
}


void k_Desktop::animate()
{
	if (!mb_AnimationEnabled)
	{
		foreach (k_DesktopBox* lk_Box_, mk_Boxes)
		{
			mk_BoxAcceleration[lk_Box_] = QPointF(0.0, 0.0);
			mk_BoxVelocity[lk_Box_] = QPointF(0.0, 0.0);
			mk_BoxLocation[lk_Box_] = QPointF(lk_Box_->x() + lk_Box_->width() / 2, lk_Box_->y() + lk_Box_->height() / 2);	
		}
	}
	else
	{
		double ld_CurrentTime = mk_StopWatch.get_Time();
		double ld_DeltaTime = ld_CurrentTime - md_LastPaintTime;
		md_LastPaintTime = ld_CurrentTime;

		mk_BoxDirection.clear();
		mk_BoxDistance.clear();
		mk_BoxOverlap.clear();

		for (int li_FirstBoxIndex = 0; li_FirstBoxIndex < mk_Boxes.size(); ++li_FirstBoxIndex)
		{
			for (int li_SecondBoxIndex = li_FirstBoxIndex + 1; li_SecondBoxIndex < mk_Boxes.size(); ++li_SecondBoxIndex)
			{
				// calculate direction vector between two boxes
				k_DesktopBox* lk_FirstBox_ = mk_Boxes[li_FirstBoxIndex];
				k_DesktopBox* lk_SecondBox_ = mk_Boxes[li_SecondBoxIndex];
				QPointF lk_Dir = mk_BoxLocation[lk_SecondBox_] - mk_BoxLocation[lk_FirstBox_];
				double ld_Length = sqrt(lk_Dir.x() * lk_Dir.x() + lk_Dir.y() * lk_Dir.y());
				if (fabs(ld_Length) > 0.00001)
					lk_Dir /= ld_Length;
				else
					lk_Dir = QPointF(0.0, 0.0);
				mk_BoxDirection[tk_BoxPair(lk_FirstBox_, lk_SecondBox_)] = lk_Dir;
				mk_BoxDirection[tk_BoxPair(lk_SecondBox_, lk_FirstBox_)] = -lk_Dir;

				// calculate box distance if necessary
				QRect lk_FirstRect(lk_FirstBox_->pos(), lk_FirstBox_->size());
				QRect lk_FirstRectExt = lk_FirstRect;
				lk_FirstRectExt.setTopLeft(lk_FirstRectExt.topLeft() - QPoint(20, 20));
				lk_FirstRectExt.setBottomRight(lk_FirstRectExt.bottomRight() + QPoint(20, 20));
				QRect lk_SecondRect(lk_SecondBox_->pos(), lk_SecondBox_->size());
				QRect lk_SecondRectExt = lk_FirstRect;
				lk_SecondRectExt.setTopLeft(lk_SecondRectExt.topLeft() - QPoint(20, 20));
				lk_SecondRectExt.setBottomRight(lk_SecondRectExt.bottomRight() + QPoint(20, 20));
				if (lk_FirstRectExt.intersects(lk_SecondRect) || lk_SecondRectExt.intersects(lk_FirstRect))
				{
					double ld_Distance = boxDistance(lk_FirstBox_, lk_SecondBox_);
					mk_BoxDistance[tk_BoxPair(lk_FirstBox_, lk_SecondBox_)] = ld_Distance;
					mk_BoxDistance[tk_BoxPair(lk_SecondBox_, lk_FirstBox_)] = ld_Distance;
				}

				// calculate box overlap
				lk_FirstRect = QRect(lk_FirstBox_->pos(), lk_FirstBox_->size());
				lk_SecondRect = QRect(lk_SecondBox_->pos(), lk_SecondBox_->size());
				bool lb_Overlap = (lk_FirstRect.intersects(lk_SecondRect) || lk_SecondRect.intersects(lk_FirstRect));
				mk_BoxOverlap[tk_BoxPair(lk_FirstBox_, lk_SecondBox_)] = lb_Overlap;
				mk_BoxOverlap[tk_BoxPair(lk_SecondBox_, lk_FirstBox_)] = lb_Overlap;
			}
		}

		// calculate remaining box distances for connectes boxes
		foreach (tk_BoxPair lk_BoxPair, mk_BoxConnections.keys())
		{
			if (!mk_BoxOverlap.contains(lk_BoxPair))
			{
				k_DesktopBox* lk_FirstBox_ = lk_BoxPair.first;
				k_DesktopBox* lk_SecondBox_ = lk_BoxPair.second;
				double ld_Distance = boxDistance(lk_FirstBox_, lk_SecondBox_);
				mk_BoxDistance[tk_BoxPair(lk_FirstBox_, lk_SecondBox_)] = ld_Distance;
				mk_BoxDistance[tk_BoxPair(lk_SecondBox_, lk_FirstBox_)] = ld_Distance;
			}
		}

		foreach (k_DesktopBox* lk_Box_, mk_Boxes)
		{
			if (!mk_SelectedBoxes.contains(lk_Box_))
			{
				mk_BoxAcceleration[lk_Box_] = QPointF(0.0, 0.0);
				foreach (k_DesktopBox* lk_Box2_, mk_Boxes)
				{
					if (lk_Box_ != lk_Box2_)
					{
						tk_BoxPair lk_BoxPair(lk_Box_, lk_Box2_);
						if (mk_BoxOverlap[lk_BoxPair])
						{
							QPointF lk_Direction = mk_BoxDirection[lk_BoxPair];
							if (lk_Direction.isNull())
							{
								double ld_Phi = ((double)rand() / RAND_MAX) * 2.0 * 3.14159;
								lk_Direction = QPointF(cos(ld_Phi), sin(ld_Phi));
							}
							double ld_Force = 100000.0;
							mk_BoxAcceleration[lk_Box_] -= lk_Direction * ld_Force;
						}
						else if (mk_BoxDistance.contains(lk_BoxPair))
						{
							double ld_Force = 100000.0 * (1.0 - smoothStep(mk_BoxDistance[lk_BoxPair] / 20.0));
							mk_BoxAcceleration[lk_Box_] -= mk_BoxDirection[lk_BoxPair] * ld_Force;
						}
					}
				}
			}
		}
		/*
		foreach (tk_BoxPair lk_BoxPair, mk_BoxConnections.keys())
		{
			// gegenseitige Anziehung
			k_DesktopBox* lk_FirstBox_ = lk_BoxPair.first;
			k_DesktopBox* lk_SecondBox_ = lk_BoxPair.second;
			double ld_SpringLength = mk_BoxConnections[lk_BoxPair];
			double ld_Distance = mk_BoxDistance[lk_BoxPair];
			double ld_MinDistance = ld_SpringLength * md_Scale - 10.0;
			double ld_MaxDistance = ld_SpringLength * md_Scale + 10.0;

			if (ld_Distance > ld_MaxDistance)
				ld_Distance = ld_MaxDistance - ld_Distance;
			else if (ld_Distance < ld_MinDistance)
				ld_Distance = ld_MinDistance - ld_Distance;
			else
				ld_Distance = 0.0;
			double ld_Force = 300.0 * ld_Distance;
			mk_BoxAcceleration[lk_FirstBox_] -= mk_BoxDirection[lk_BoxPair] * ld_Force;
			mk_BoxAcceleration[lk_SecondBox_] += mk_BoxDirection[lk_BoxPair] * ld_Force;
		}
		*/

		foreach (k_DesktopBox* lk_Box_, mk_Boxes)
		{
			if (mk_SelectedBoxes.contains(lk_Box_))
			{
				mk_BoxAcceleration[lk_Box_] = QPointF(0.0, 0.0);
				mk_BoxVelocity[lk_Box_] = QPointF(0.0, 0.0);
				mk_BoxLocation[lk_Box_] = QPointF(lk_Box_->x() + lk_Box_->width() / 2, lk_Box_->y() + lk_Box_->height() / 2);
			}
		}

		foreach (k_DesktopBox* lk_Box_, mk_Boxes)
		{
			mk_BoxVelocity[lk_Box_] += mk_BoxAcceleration[lk_Box_] * ld_DeltaTime;
			double ld_Velocity = sqrt(mk_BoxVelocity[lk_Box_].x() * mk_BoxVelocity[lk_Box_].x() + mk_BoxVelocity[lk_Box_].y() * mk_BoxVelocity[lk_Box_].y());
			if (ld_Velocity > 300.0)
				mk_BoxVelocity[lk_Box_] *= 300.0 / ld_Velocity;
			mk_BoxLocation[lk_Box_] += mk_BoxVelocity[lk_Box_] * ld_DeltaTime;
			if (mk_BoxLocation[lk_Box_].x() < lk_Box_->width() / 2)
				mk_BoxLocation[lk_Box_].setX(lk_Box_->width() / 2);
			if (mk_BoxLocation[lk_Box_].x() > width() - lk_Box_->width() / 2)
				mk_BoxLocation[lk_Box_].setX(width() - lk_Box_->width() / 2);
			if (mk_BoxLocation[lk_Box_].y() < lk_Box_->height() / 2)
				mk_BoxLocation[lk_Box_].setY(lk_Box_->height() / 2);
			if (mk_BoxLocation[lk_Box_].y() > height() - lk_Box_->height() / 2)
				mk_BoxLocation[lk_Box_].setY(height() - lk_Box_->height() / 2);
			//mk_BoxVelocity[lk_Box_] *= pow(0.00001, ld_DeltaTime);
			mk_BoxVelocity[lk_Box_] = QPointF(0.0, 0.0);
			lk_Box_->move(mk_BoxLocation[lk_Box_].x() - lk_Box_->width() / 2, mk_BoxLocation[lk_Box_].y() - lk_Box_->height() / 2);
		}
	}
	update();
}


void k_Desktop::addScriptBox(QAction* ak_Action_)
{
	k_ScriptBox* lk_ScriptBox_ = new k_ScriptBox(ak_Action_->data().toString(), this, mk_Proteomatic);
	this->addBox(lk_ScriptBox_);
}


void k_Desktop::enableAnimation(bool ab_Enable)
{
	mb_AnimationEnabled = ab_Enable;
	md_LastPaintTime = mk_StopWatch.get_Time();
	if (mb_AnimationEnabled)
		mk_AnimationTimer.start();
	else
		mk_AnimationTimer.stop();
}


void k_Desktop::mousePressEvent(QMouseEvent* ak_Event_)
{
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
}


void k_Desktop::mouseReleaseEvent(QMouseEvent* ak_Event_)
{
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
}


void k_Desktop::mouseMoveEvent(QMouseEvent* ak_Event_)
{
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
}


void k_Desktop::dragEnterEvent(QDragEnterEvent* ak_Event_)
{
	ak_Event_->acceptProposedAction();
}


void k_Desktop::dropEvent(QDropEvent* ak_Event_)
{
	ak_Event_->accept();
	foreach (QUrl lk_Url, ak_Event_->mimeData()->urls())
	{
		QString ls_Path = lk_Url.toLocalFile();
		if (ls_Path != "")
		{
			QFileInfo lk_FileInfo(ls_Path);
			if (!lk_FileInfo.isDir())
			{
				k_FileBox* lk_FileBox_ = new k_FileBox(this, mk_Proteomatic);
				lk_FileBox_->setFilename(ls_Path);
				addBox(lk_FileBox_);
			}
		}
	}
}


void k_Desktop::paintEvent(QPaintEvent* ak_Event_)
{
	QPainter lk_Painter(this);
	lk_Painter.fillRect(0, 0, width(), height(), QBrush(QColor::fromRgb(220, 220, 220)));
	lk_Painter.setRenderHints(QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform | QPainter::Antialiasing, true);

	if (!mk_Lasso.isEmpty())
	{
		lk_Painter.setPen(Qt::SolidLine);
		lk_Painter.setPen(QColor::fromRgb(160, 160, 160));
		lk_Painter.setBrush(QBrush(QColor::fromRgb(230, 230, 230)));
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
}


void k_Desktop::wheelEvent(QWheelEvent* ak_Event_)
{
	if ((ak_Event_->modifiers() & Qt::ControlModifier) != 0)
	{
		double ld_ScaleDelta = pow(1.1, fabs(ak_Event_->delta() / 100.0));
		if (ak_Event_->delta() < 0)
			ld_ScaleDelta = 1.0 / ld_ScaleDelta;
		md_Scale *= ld_ScaleDelta;
		md_Scale = std::min<double>(md_Scale, 3.0);
		md_Scale = std::max<double>(md_Scale, 0.5);
		foreach (k_DesktopBox* lk_Box_, mk_Boxes)
			lk_Box_->scale(md_Scale);
		animate();
	}
}


void k_Desktop::drawArrow(QPainter* ak_Painter_, k_DesktopBox* ak_Box0_, k_DesktopBox* ak_Box1_)
{
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
}


void k_Desktop::drawArrow(QPainter* ak_Painter_, k_DesktopBox* ak_Box0_, QPoint ak_Point)
{
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
	QPointF lk_Point0 = mk_BoxLocation[ak_Box0_];
	QPointF lk_Point1 = mk_BoxLocation[ak_Box1_];
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
			t = intersect(ak_Point0, lk_Dir, mk_BoxLocation[ak_Box_] - QPointF(ak_Box_->width(), ak_Box_->height()) * 0.5, QPointF(0, ak_Box_->height()));
		else
			t = intersect(ak_Point0, lk_Dir, mk_BoxLocation[ak_Box_] - QPointF(ak_Box_->width(), ak_Box_->height()) * 0.5 + QPointF(ak_Box_->width(), 0), QPointF(0, ak_Box_->height()));
	}
	else
	{
		QPointF lk_Quadrant = ak_Point0 - ak_Point1;
		if (lk_Quadrant.y() < 0)
			t = intersect(ak_Point0, lk_Dir, mk_BoxLocation[ak_Box_] - QPointF(ak_Box_->width(), ak_Box_->height()) * 0.5, QPointF(ak_Box_->width(), 0));
		else
			t = intersect(ak_Point0, lk_Dir, mk_BoxLocation[ak_Box_] - QPointF(ak_Box_->width(), ak_Box_->height()) * 0.5 + QPointF(0, ak_Box_->height()), QPointF(ak_Box_->width(), 0));
	}
	
	return ak_Point0 + lk_Dir * t;
}


double k_Desktop::boxDistance(k_DesktopBox* ak_Box0_, k_DesktopBox* ak_Box1_)
{
	QPointF lk_Point0, lk_Point1;
	boxConnector(ak_Box0_, ak_Box1_, lk_Point0, lk_Point1);
	QPointF lk_Dir = lk_Point0 - lk_Point1;
	return sqrt(lk_Dir.x() * lk_Dir.x() + lk_Dir.y() * lk_Dir.y());
}


double k_Desktop::smoothStep(double x) const
{
	if (x < 0.0)
		return 0.0;
	if (x > 1.0)
		return 1.0;
	return 3 * x * x - 2 * x * x * x;
}


k_DesktopBox* k_Desktop::boxAt(QPoint ak_Point) const
{
	QWidget* lk_Widget_ = childAt(ak_Point);
	k_DesktopBox* lk_Box_ = dynamic_cast<k_DesktopBox*>(lk_Widget_);
	while (lk_Box_ == NULL && lk_Widget_ != this && lk_Widget_ != NULL)
	{
		lk_Widget_ = lk_Widget_->parentWidget();
		lk_Box_ = dynamic_cast<k_DesktopBox*>(lk_Widget_);
	}
	return lk_Box_;
}

