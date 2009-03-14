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
#include "IDesktopBox.h"
#include "RefPtr.h"


class k_PipelineMainWindow;
class k_Proteomatic;

/*
	typedef QPair<IDesktopBox*, IDesktopBox*> tk_BoxPair;
	typedef QSet<IDesktopBox*> tk_DesktopBoxSet;
	typedef QSet<IFileBox*> tk_FileBoxSet;
	typedef QSet<tk_BoxPair> tk_BoxPairSet;
	*/


class k_Desktop: public QGraphicsView
{
	Q_OBJECT
public:
	k_Desktop(QWidget* ak_Parent_, k_Proteomatic& ak_Proteomatic, k_PipelineMainWindow& ak_PipelineMainWindow);
	virtual ~k_Desktop();

	virtual void addInputFileBox(const QString& as_Path);
	virtual void addInputFileListBox();
	virtual void addScriptBox(const QString& as_ScriptUri);
	
protected:
	virtual void addBox(RefPtr<IDesktopBox> ak_pBox);
	
	k_Proteomatic& mk_Proteomatic;
	k_PipelineMainWindow& mk_PipelineMainWindow;

	QList<RefPtr<IDesktopBox> > mk_Boxes;
	QHash<IDesktopBox*, QGraphicsProxyWidget*> mk_ProxyWidgetForDesktopBox;
	RefPtr<QGraphicsScene> mk_pGraphicsScene;
};
