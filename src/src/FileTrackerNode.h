/*
Copyright (c) 2007-2008 Thaddaeus Slawicki

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
#include "RefPtr.h"
#include "Tango.h"

class k_FileTrackerNode: public QWidget
{
	Q_OBJECT
public:
	k_FileTrackerNode();
	virtual ~k_FileTrackerNode();
	
	virtual float horizontalAlignment() const;
	virtual float verticalAlignment() const;
	virtual const QPointF position() const;
	
public slots:
	virtual void setHorizontalAlignment(float af_HorizontalAlignment);
	virtual void setVerticalAlignment(float af_VerticalAlignment);
	virtual void setAlignment(float af_HorizontalAlignment, float af_VerticalAlignment);
	virtual void setPosition(const QPointF ak_Position);
	virtual void setLabels(QStringList ak_Labels);
	virtual void setMaximumWidth(int ai_MaxWidth);
	virtual void setFrameColor(QString as_Color);
	
protected:
	virtual void paintEvent(QPaintEvent* event);
	virtual void resizeEvent(QResizeEvent* event);
	virtual int nodeWidth();
	
	virtual void adjustPosition();
	
	float mf_HorizontalAlignment;
	float mf_VerticalAlignment;
	QPointF mk_Position;
	QList<RefPtr<QWidget> > mk_LabelWidgets;
	int mi_MaximumWidth;
	QPen mk_FramePen;
};

