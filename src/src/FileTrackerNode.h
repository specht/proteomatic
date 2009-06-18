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

class k_FileTrackerNode:: public QWidget
{
public:
	k_FileTrackerNode();
	
private:
	
	QLabel* k_NameLabel;
	QLabel* k_UserLabel;
	QLabel* K_SizeLabel;
	QLabel* K_FurtherInfoLabel;
	
	
	QHBoxLayout* k_MainLabelLayout;
	QVBoxLayout* k_MainFurtherInfoLabel;
	QVBoxLayout* k_OutInputLayout;
	//temporarily static mainlayout
	QVBoxLayout* k_MainLayout;
	
	QGroupBox* k_InputFileGroup;
	QGroupBox* k_OutputFileGroup;
	QGroupBox* k_InputRunGroup;
	QGroupBox* k_OutputRunGroup;
	
};
