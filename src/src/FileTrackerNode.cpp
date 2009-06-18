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
	//Horizontal Label
	mk_MainLabelLayout = new QHBoxLayout;
	mk_MainLabelLayout->addWidget(k_NameLabel);
	mk_MainLabelLayout->addWidget(k_RunUserLabel);
	mk_MainLabelLayout->addWidget(k_SizeLabel);
	
	//Main Label
	mk_MainFurtherInfoLabel = new QVBoxLayout;
	mk_MainFurtherInfoLabel->addWidget(mk_MainLabelLayout);
	mk_MainFurtherInfoLabel->addWidget(mk_FurtherInfoLabel);
	
	//grouping Input-Files
	mk_InputFileGroup = new QGroupBox("Input-Files");
	mk_InputFileGroup->addWidget(mk_MainFurtherInfoLabel);
	mk_InputFileGroup->addWidget(mk_MainFurtherInfoLabel);
	
	//grouping Output-Files
	mk_OutputFileGroup = new QGroupBox("Output-Files");
	mk_OutputFileGroup->addWidget(mk_MainFurtherInfoLabel);
	mk_OutputFileGroup->addWidget(mk_MainFurtherInfoLabel);
	
	//grouping Input-Runs
	mk_InputRunGroup = new QGroupBox("Runs with File as Input");
	mk_InputRunGroup->addWidget(mk_MainFurtherInfoLabel);
	mk_InputRunGroup->addWidget(mk_MainFurtherInfoLabel);
	
	//grouping Output-Runs
	mk_OutputRunGroup = new QGroupBox("Runs with File as Output");
	mk_OutputRunGroup->addWidget(mk_MainFurtherInfoLabel);
	mk_OutputRunGroup->addWidget(mk_MainFurtherInfoLabel);
	
	//mainlayout
	mk_MainLayout = new QVBoxLayout
	mk_MainLayout->addWidget(mk_InputFileGroup);
	mk_MainLayout->addWidget(mk_MainFurtherInfoLabel);
	mk_MainLayout->addWidget(mk_OutputFileGroup);
}
