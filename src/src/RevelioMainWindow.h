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
#include "Surface.h"
#include "FileTrackerNode.h"

class k_RevelioMainWindow: public QMainWindow
{
	Q_OBJECT
public:
	k_RevelioMainWindow(QWidget* ak_Parent_ = NULL);
	virtual ~k_RevelioMainWindow();
	QString md5ForFile(QString as_Path);
	virtual void adjustLayout();
	QScrollArea& paneScrollArea();
	
public slots:

signals:
	
protected slots:
	virtual void loadFile();
	
protected:
	//k_Surface* mk_Surface_;
	k_Surface mk_Surface;
	//RefPtr<k_Surface> mk_pSurface;
	//QLabel mk_HashLabel;
	//QLabel mk_ParamLabel;
	//QTextEdit mk_StdoutTextEdit;

	QScrollArea mk_PaneScrollArea;
};
