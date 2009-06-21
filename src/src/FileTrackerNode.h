/*
Copyright (c) 2007-2008 Thaddäus Slawicki

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
#include "Tango.h"

class k_FileTrackerNode: public QWidget
{
	Q_OBJECT
public:
	k_FileTrackerNode();
	
private:
	QLabel* lk_NameLabel;
	QLabel* lk_RunUserLabel;
	QLabel* lk_SizeLabel;
	QLabel* lk_FurtherInfoLabel;
	QGraphicsLineItem* lk_InsertVerticalLine1;
	QGraphicsLineItem* lk_InsertVerticalLine2;
	QGraphicsLineItem* lk_InsertHorizontalLine;
	QPen* lk_Pen;
	QPainter* lk_Painter;
	
	QHBoxLayout* lk_MainLabelLayout;
	QVBoxLayout* lk_MainFurtherInfoLabel;
/* 	QVBoxLayout* lk_OutInputLayout;
	//temporarily static mainlayout
	QVBoxLayout* lk_MainLayout;
	
	QGroupBox* lk_InputFileGroup;
	QGroupBox* lk_OutputFileGroup;
	QGroupBox* lk_InputRunGroup;
	QGroupBox* lk_OutputRunGroup;
	*/
};
