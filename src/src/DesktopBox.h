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

#pragma once

#include <QtGui>
#include "Proteomatic.h"
#include "StopWatch.h"
#include "Script.h"
#include <math.h>
#include <stdlib.h>


class k_Desktop;
class k_ScriptBox;
class k_FileBox;


class k_DesktopBox: public QWidget
{
	Q_OBJECT
public:
	k_DesktopBox(k_Desktop* ak_Parent_, k_Proteomatic& ak_Proteomatic);
	virtual ~k_DesktopBox();

	void scale(double ad_Scale);

protected:
	virtual void paintEvent(QPaintEvent* ak_Event_);

protected:
	k_Desktop* mk_Desktop_;
	QBrush mk_Background;
	QPen mk_Border;
	double md_OriginalFontSize;
	int mi_OriginalMargin;
	int mi_OriginalIconSize;
	QVBoxLayout mk_Layout;
	k_Proteomatic& mk_Proteomatic;
};


class k_ScriptBox: public k_DesktopBox
{
	Q_OBJECT
public:
	k_ScriptBox(QString as_ScriptName, k_Desktop* ak_Parent_, k_Proteomatic& ak_Proteomatic);
	virtual ~k_ScriptBox();

protected slots:
	void toggleOutput(bool ab_Enabled);

protected:
	k_Script* mk_Script_;
	QHash<QString, k_FileBox*> mk_OutputFileBoxes;
};


class k_FileBox: public k_DesktopBox
{
	Q_OBJECT
public:
	k_FileBox(k_Desktop* ak_Parent_, k_Proteomatic& ak_Proteomatic);
	virtual ~k_FileBox();
	void setFilename(const QString& as_Filename);

protected:
	QLabel mk_Label;
	QString ms_Filename;
};
