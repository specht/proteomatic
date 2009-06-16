/*
Copyright (c) 2007-2008 Thadd�us Slawicki

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
//#include "ISurfaceBox.h"

class k_RevelioMainWindow;


class k_Surface: public QGraphicsView
{
	Q_OBJECT
public:
	k_Surface(k_RevelioMainWindow& ak_RevelioMainWindow, QWidget* ak_Parent_ = NULL);
	virtual ~k_Surface();
	
	virtual QGraphicsScene& graphicsScene();
	//virtual void addRect(ISurfaceBox* ak_Box_);
	
public slots:
//addRect(QRectf());
	
signals:

protected:
	
	virtual void resizeEvent(QResizeEvent* event);
	
	k_RevelioMainWindow& mk_RevelioMainWindow;
	QGraphicsScene mk_GraphicsScene;
	
	
	float mf_SceneWidth2; 
	float mf_SceneHeight2;
	
	
	
	
};