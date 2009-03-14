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
#include "IScriptBox.h"
#include "PipelineMainWindow.h"
#include "Tango.h"


k_Desktop::k_Desktop(QWidget* ak_Parent_, k_Proteomatic& ak_Proteomatic, k_PipelineMainWindow& ak_PipelineMainWindow)
	: QGraphicsView(ak_Parent_)
	, mk_Proteomatic(ak_Proteomatic)
	, mk_PipelineMainWindow(ak_PipelineMainWindow)
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
}


void k_Desktop::addScriptBox(const QString& as_ScriptUri)
{
	RefPtr<IDesktopBox> lk_pBox = k_DesktopBoxFactory::makeScriptBox(as_ScriptUri, this, mk_Proteomatic);
	if (lk_pBox)
		addBox(lk_pBox);
}


void k_Desktop::addBox(RefPtr<IDesktopBox> ak_pBox)
{
	mk_Boxes.append(ak_pBox);
	k_DesktopBox* lk_DesktopBox_ = dynamic_cast<k_DesktopBox*>(ak_pBox.get_Pointer());
	mk_ProxyWidgetForDesktopBox[ak_pBox.get_Pointer()] = mk_pGraphicsScene->addWidget(lk_DesktopBox_);
}
