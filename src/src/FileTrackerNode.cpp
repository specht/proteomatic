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

#include "FileTrackerNode.h"

k_FileTrackerNode::k_FileTrackerNode()
{	
	//
	QPainter lk_Painter(this);
	
	QPen lk_Pen(TANGO_ALUMINIUM_3);
	float lf_PenWidth = 1.5;
	lk_Pen.setWidthF(lf_PenWidth);
	lk_Painter.setPen(lk_Pen);
	QBrush lk_Brush(TANGO_ALUMINIUM_0);
	lk_Painter.setBrush(lk_Brush);
	lk_Painter.drawRoundedRect(QRectF(lf_PenWidth * 0.5, lf_PenWidth * 0.5, (qreal)width() - lf_PenWidth, (qreal)height() - lf_PenWidth), 8.0, 8.0);

	//Horizontal Label
	lk_MainLabelLayout = new QHBoxLayout;
	lk_MainLabelLayout->addWidget(lk_NameLabel);
	//lk_MainLabelLayout->addWidget(lk_InsertVerticalLine1);
	lk_MainLabelLayout->addWidget(lk_RunUserLabel);
	//lk_MainLabelLayout->addWidget(lk_InsertVerticalLine2);
	lk_MainLabelLayout->addWidget(lk_SizeLabel);
	
	//Main Label
	lk_MainFurtherInfoLabel = new QVBoxLayout;
	lk_MainFurtherInfoLabel->addWidget(lk_MainLabelLayout);
	//lk_MainFurtherInfoLabel->addWidget(lk_InsertHorizontalLine);
	lk_MainFurtherInfoLabel->addWidget(lk_FurtherInfoLabel);
	
	
	/*
	//grouping Input-Files
	lk_InputFileGroup = new QGroupBox("Input-Files");
	lk_InputFileGroup->addWidget(lk_MainFurtherInfoLabel);
	lk_InputFileGroup->addWidget(lk_MainFurtherInfoLabel);
	
	//grouping Output-Files
	lk_OutputFileGroup = new QGroupBox("Output-Files");
	lk_OutputFileGroup->addWidget(lk_MainFurtherInfoLabel);
	lk_OutputFileGroup->addWidget(lk_MainFurtherInfoLabel);
	
	//grouping Input-Runs
	lk_InputRunGroup = new QGroupBox("Runs with File as Input");
	lk_InputRunGroup->addWidget(lk_MainFurtherInfoLabel);
	lk_InputRunGroup->addWidget(lk_MainFurtherInfoLabel);
	
	//grouping Output-Runs
	lk_OutputRunGroup = new QGroupBox("Runs with File as Output");
	lk_OutputRunGroup->addWidget(lk_MainFurtherInfoLabel);
	lk_OutputRunGroup->addWidget(lk_MainFurtherInfoLabel);
	
	//mainlayout
	lk_MainLayout = new QVBoxLayout
	lk_MainLayout->addWidget(lk_InputFileGroup);
	lk_MainLayout->addWidget(lk_MainFurtherInfoLabel);
	lk_MainLayout->addWidget(lk_OutputFileGroup);
	*/
	
}

