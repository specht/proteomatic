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
#include "PipelineMainWindow.h"
#include <math.h>


k_Desktop::k_Desktop(QWidget* ak_Parent_, k_Proteomatic& ak_Proteomatic, k_PipelineMainWindow& ak_PipelineMainWindow)
	: QGraphicsView(ak_Parent_)
	, mk_ArrowStartBox_(NULL)
	, mk_ArrowEndBox_(NULL)
	, mk_ArrowPathItem_(NULL)
	, me_MouseMode(r_MouseMode::Move)
	, mk_Proteomatic(ak_Proteomatic)
	, mk_PipelineMainWindow(ak_PipelineMainWindow)
	, md_Scale(1.0)
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
	
	connect(&mk_PipelineMainWindow, SIGNAL(forceRefresh()), this, SLOT(forceReset()));
}


k_Desktop::~k_Desktop()
{
	foreach (k_DesktopBox* lk_Box_, mk_Boxes)
		this->removeBox(lk_Box_);
}


void k_Desktop::addBox(k_DesktopBox* ak_Box_, k_DesktopBox* ak_CloseTo_, QPoint* ak_Location_)
{
	mk_Boxes.push_back(ak_Box_);
	if (ak_Location_)
		ak_Box_->move(*ak_Location_);
	
	mk_BoxConnectionsForBox[ak_Box_] = tk_DesktopBoxSet();
	
	if (dynamic_cast<k_ScriptBox*>(ak_Box_) != NULL)
		mk_FileBoxesForScriptBox[dynamic_cast<k_ScriptBox*>(ak_Box_)] = tk_FileBoxSet();
	
	mk_pGraphicsScene->addWidget(ak_Box_);
	if (ak_CloseTo_)
		ak_Box_->move(ak_CloseTo_->pos() + QPoint(ak_CloseTo_->width() / 2 - ak_Box_->width() / 2, ak_CloseTo_->height() + 64));
	else
		ak_Box_->move(ak_Box_->pos() - QPoint(ak_Box_->size().width(), ak_Box_->size().height()) / 2);
	
	connect(ak_Box_, SIGNAL(moved()), this, SLOT(boxMoved()));
	connect(ak_Box_, SIGNAL(resized()), this, SLOT(boxMoved()));
	k_FileBox* lk_FileBox_ = dynamic_cast<k_FileBox*>(ak_Box_);
	if (lk_FileBox_ != NULL)
	{
		connect(lk_FileBox_, SIGNAL(arrowPressed()), this, SLOT(fileBoxArrowPressed()));
		connect(lk_FileBox_, SIGNAL(arrowReleased()), this, SLOT(fileBoxArrowReleased()));
	}
	k_OutputFileBox* lk_OutputFileBox_ = dynamic_cast<k_OutputFileBox*>(ak_Box_);
	if (lk_OutputFileBox_ != NULL)
	{
		lk_OutputFileBox_->setDirectory(mk_PipelineMainWindow.outputDirectory());
		connect(&mk_PipelineMainWindow, SIGNAL(outputDirectoryChanged(const QString&)), lk_OutputFileBox_, SLOT(setDirectory(const QString&)));
	}
	connect(ak_Box_, SIGNAL(mousePressed(Qt::KeyboardModifiers)), this, SLOT(boxClicked(Qt::KeyboardModifiers)));
	ak_Box_->snapToGrid();
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
	
	// disconnect all signals and slots (this is not arrow disconnection!)
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
	
	QPen lk_ProxyPen(QColor("#f8f8f8"));
	lk_ProxyPen.setWidth(10);
	
	QPointF lk_Point0, lk_Point1;
	this->boxConnector(ak_Box0_, ak_Box1_, lk_Point0, lk_Point1);
	QGraphicsLineItem* lk_GraphicsLineItem_ =
		mk_pGraphicsScene->addLine(QLineF(lk_Point0, lk_Point1), lk_ProxyPen);
	lk_GraphicsLineItem_->setZValue(-2.0);
	mk_ProxyLineForArrow[lk_GraphicsPathItem_] = RefPtr<QGraphicsLineItem>(lk_GraphicsLineItem_);
	mk_ArrowForProxyLine[lk_GraphicsLineItem_] = lk_GraphicsPathItem_;
	mk_BoxPairForProxyLine[lk_GraphicsLineItem_] = tk_BoxPair(ak_Box0_, ak_Box1_);
	
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
	
	QGraphicsLineItem* lk_ProxyLine_ = mk_ProxyLineForArrow[mk_BoxConnections[tk_BoxPair(ak_Box0_, ak_Box1_)].get_Pointer()].get_Pointer();
	mk_ProxyLineForArrow.remove(mk_BoxConnections[tk_BoxPair(ak_Box0_, ak_Box1_)].get_Pointer());
	mk_ArrowForProxyLine.remove(lk_ProxyLine_);
	mk_BoxPairForProxyLine.remove(lk_ProxyLine_);
	mk_BoxConnections.remove(tk_BoxPair(ak_Box0_, ak_Box1_));
	mk_BoxConnectionsForBox[ak_Box0_].remove(ak_Box1_);
	mk_BoxConnectionsForBox[ak_Box1_].remove(ak_Box0_);
	
	if (dynamic_cast<k_ScriptBox*>(ak_Box1_) != NULL &&
		dynamic_cast<k_FileBox*>(ak_Box0_) != NULL)
	{
		mk_FileBoxesForScriptBox[dynamic_cast<k_ScriptBox*>(ak_Box1_)].remove(dynamic_cast<k_FileBox*>(ak_Box0_));
		dynamic_cast<k_ScriptBox*>(ak_Box1_)->fileBoxDisconnected(dynamic_cast<k_FileBox*>(ak_Box0_));
	}
	mk_SelectedArrows.remove(tk_BoxPair(ak_Box0_, ak_Box1_));
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


bool k_Desktop::arrowSelected(tk_BoxPair ak_BoxPair) const
{
	return mk_SelectedArrows.contains(ak_BoxPair);
}


k_Desktop::tk_DesktopBoxSet k_Desktop::selectedBoxes() const
{
	return mk_SelectedBoxes;
}


k_Desktop::tk_FileBoxSet k_Desktop::fileBoxesForScriptBox(k_ScriptBox* ak_ScriptBox_) const
{
	if (!mk_FileBoxesForScriptBox.contains(ak_ScriptBox_))
		return tk_FileBoxSet();
	return mk_FileBoxesForScriptBox[ak_ScriptBox_];
}


k_PipelineMainWindow& k_Desktop::pipelineMainWindow()
{
	return mk_PipelineMainWindow;
}


void k_Desktop::start()
{
	this->resetScriptBoxes();
	this->updateAllBoxes();
	
	// see if all boxes are ready...
	
	bool lb_Errors = false;
	foreach (k_DesktopBox* lk_Box_, mk_Boxes)
		if (lk_Box_->status() != r_BoxStatus::Ready)
			lb_Errors = true;

	if (lb_Errors)
	{
		mk_Proteomatic.showMessageBox("Error", "There are problems with some scripts or files. Please solve these problems and try again. For a description of each problem, click on the warning sign icon in each box.", ":icons/emblem-important.png");
		return;
	}
	
	// now look for script boxes that are ready to go!
	foreach (k_DesktopBox* lk_Box_, mk_Boxes)
	{
		k_ScriptBox* lk_ScriptBox_ = dynamic_cast<k_ScriptBox*>(lk_Box_);
		if (lk_ScriptBox_)
			mk_RunningScriptBoxQueue.insert(lk_ScriptBox_);
	}
	
	this->startNextScriptBox();
	this->toggleUi();
}


void k_Desktop::toggleUi()
{
	mk_PipelineMainWindow.toggleUi();
}


bool k_Desktop::running()
{
	return !mk_RunningScriptBoxQueue.empty();
}


void k_Desktop::setMouseMode(r_MouseMode::Enumeration ae_MouseMode)
{
	me_MouseMode = ae_MouseMode;
}


void k_Desktop::addScriptBox(QAction* ak_Action_)
{
	k_ScriptBox* lk_ScriptBox_ = new k_ScriptBox(ak_Action_->data().toString(), this, mk_Proteomatic);
	this->addBox(lk_ScriptBox_);
	
	connect(lk_ScriptBox_, SIGNAL(scriptFinished()), this, SLOT(scriptFinished()));
	
	// activate default output files after the script box has been constructed
	k_Script* lk_Script_ = lk_ScriptBox_->script();
	foreach (QString ls_Key, lk_Script_->outFiles())
	{
		QHash<QString, QString> lk_OutFile = lk_Script_->outFileDetails(ls_Key);
		if (lk_OutFile.contains("default"))
			lk_ScriptBox_->toggleOutputFile(ls_Key, lk_OutFile["default"] == "true" || lk_OutFile["default"] == "yes");
	}
	
	// select script box plus auto output file boxe
	this->clearSelection();
	this->addBoxToSelection(lk_ScriptBox_);
	foreach (k_OutputFileBox* lk_Box_, lk_ScriptBox_->outputFileBoxes())
		this->addBoxToSelection(lk_Box_);
}


void k_Desktop::addFileBox(QString as_Path)
{
	QFileInfo lk_FileInfo(as_Path);
	if (lk_FileInfo.isDir())
		return;
	
	k_InputFileBox* lk_InputFileBox_ = new k_InputFileBox(this, mk_Proteomatic);
	lk_InputFileBox_->setFilename(as_Path);
	this->addBox(lk_InputFileBox_);
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
	mk_ArrowPathItem_->setZValue(1000.0);
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
		if (!this->boxSelected(lk_Box_))
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


void k_Desktop::forceRefresh()
{
	this->updateAllBoxes();
}


void k_Desktop::forceReset()
{
	this->resetScriptBoxes();
	this->updateAllBoxes();
}


void k_Desktop::scriptFinished()
{
	this->startNextScriptBox();
	this->updateAllBoxes();
}


void k_Desktop::mousePressEvent(QMouseEvent* ak_Event_)
{
	// click on background without ctrl clears selection
	if (mk_pGraphicsScene->items(this->mapToScene(ak_Event_->pos())).empty())
		if ((ak_Event_->modifiers() & Qt::ControlModifier) == 0)
			this->clearSelection();

	QList<QGraphicsItem*> lk_Items = mk_pGraphicsScene->items(this->mapToScene(ak_Event_->pos()));
	if (!lk_Items.empty())
	{
		// pick the last item in the list because proxy lines should be lowest of all
		QGraphicsLineItem* lk_LineItem_ = dynamic_cast<QGraphicsLineItem*>(lk_Items.last());
		if (lk_LineItem_)
		{
			tk_BoxPair lk_BoxPair = mk_BoxPairForProxyLine[lk_LineItem_];
			if ((ak_Event_->modifiers() & Qt::ControlModifier) != 0)
			{
				// ctrl pressed
				if (!this->arrowSelected(lk_BoxPair))
					this->addArrowToSelection(lk_BoxPair);
				else
					this->removeArrowFromSelection(lk_BoxPair);
			}
			else
			{
				// no ctrl pressed
				this->clearSelection();
				this->addArrowToSelection(lk_BoxPair); 
			}
			// return to avoid mouse panning
			return;
		}
	}
	
	QGraphicsView::mousePressEvent(ak_Event_);
	return;
}


void k_Desktop::mouseReleaseEvent(QMouseEvent* ak_Event_)
{
	QGraphicsView::mouseReleaseEvent(ak_Event_);
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
}


void k_Desktop::wheelEvent(QWheelEvent* ak_Event_)
{
	if ((ak_Event_->modifiers() & Qt::ControlModifier) != 0)
	{
		double ld_ScaleDelta = pow(1.1, fabs(ak_Event_->delta() / 100.0));
		if (ak_Event_->delta() < 0)
			ld_ScaleDelta = 1.0 / ld_ScaleDelta;
		md_Scale *= ld_ScaleDelta;
		md_Scale = std::max<double>(md_Scale, 0.3);
		md_Scale = std::min<double>(md_Scale, 1.0);
		QMatrix lk_Matrix = this->matrix();
		lk_Matrix.setMatrix(md_Scale, lk_Matrix.m12(), lk_Matrix.m21(), md_Scale, lk_Matrix.dx(), lk_Matrix.dy());
		this->setMatrix(lk_Matrix);
	}
}


void k_Desktop::keyPressEvent(QKeyEvent* ak_Event_)
{
	// do this so that pressing delete in a line edit widget 
	// won't delete currently selected boxes / arrows
	ak_Event_->ignore();
	QGraphicsView::keyPressEvent(ak_Event_);
	if (ak_Event_->isAccepted())
		return;
	
	if (ak_Event_->key() == Qt::Key_Delete)
	{
		foreach (tk_BoxPair lk_BoxPair, mk_SelectedArrows)
		{
			// if its an arrow that connects a script box with an output file box,
			// remove the output file box entirely
			if (dynamic_cast<k_ScriptBox*>(lk_BoxPair.first) &&
				dynamic_cast<k_OutputFileBox*>(lk_BoxPair.second))
				this->removeBox(lk_BoxPair.second);
			else
				this->disconnectBoxes(lk_BoxPair.first, lk_BoxPair.second);
		}

		foreach (k_DesktopBox* lk_Box_, mk_SelectedBoxes)
			this->removeBox(lk_Box_);
		
		this->clearSelection();
	}
}


inline QPoint k_Desktop::boxLocation(k_DesktopBox* ak_Box_) const
{
	return ak_Box_->pos() + QPoint(ak_Box_->width(), ak_Box_->height()) / 2;
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
	QPen lk_Pen = lk_PathItem_->pen();
	if (mk_SelectedArrows.contains(lk_BoxPair))
		lk_Pen.setWidthF(2.0);
	else
		lk_Pen.setWidthF(1.0);
	lk_Pen.setJoinStyle(Qt::MiterJoin);
	lk_PathItem_->setPen(lk_Pen);
	
	mk_ProxyLineForArrow[lk_PathItem_]->setLine(QLineF(lk_Start, lk_End));
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
	foreach (tk_BoxPair lk_BoxPair, mk_SelectedArrows)
		this->removeArrowFromSelection(lk_BoxPair);
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


void k_Desktop::addArrowToSelection(tk_BoxPair ak_BoxPair)
{
	mk_SelectedArrows.insert(ak_BoxPair);
	this->updateBoxConnector(ak_BoxPair.first, ak_BoxPair.second);
}


void k_Desktop::removeArrowFromSelection(tk_BoxPair ak_BoxPair)
{
	mk_SelectedArrows.remove(ak_BoxPair);
	this->updateBoxConnector(ak_BoxPair.first, ak_BoxPair.second);
}


void k_Desktop::resetScriptBoxes()
{
	foreach (k_DesktopBox* lk_Box_, mk_Boxes)
	{
		k_ScriptBox* lk_ScriptBox_ = dynamic_cast<k_ScriptBox*>(lk_Box_);
		if (lk_ScriptBox_)
			lk_ScriptBox_->resetScript();
	}
}


void k_Desktop::updateAllBoxes()
{
	foreach (k_DesktopBox* lk_Box_, mk_Boxes)
		lk_Box_->updateStatus();
}


void k_Desktop::startNextScriptBox()
{
	if (mk_RunningScriptBoxQueue.empty())
	{
		this->toggleUi();
		return;
	}
	
	foreach (k_ScriptBox* lk_Box_, mk_RunningScriptBoxQueue)
	{
		if (lk_Box_->status() == r_BoxStatus::Ready && lk_Box_->allInputFilesExist())
		{
			lk_Box_->start();
			return;
		}
	}
	
	// if we came here, there was no box to start!
	mk_RunningScriptBoxQueue.clear();
	this->toggleUi();
}
