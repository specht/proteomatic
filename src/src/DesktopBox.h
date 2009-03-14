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

#include "IDesktopBox.h"
#include <QtCore>
#include <QtGui>


class k_Desktop;
class k_Proteomatic;


struct r_ConnectionDirection
{
	enum Enumeration
	{
		Incoming,
		Outgoing
	};
};


class k_DesktopBox: public QWidget, public IDesktopBox
{
	Q_OBJECT
public:
	k_DesktopBox(k_Desktop* ak_Parent_, k_Proteomatic& ak_Proteomatic);
	virtual ~k_DesktopBox();
	
	// IDesktopBox
	
	virtual bool batchMode() const;
	virtual QList<IDesktopBox*> incomingBoxes() const;
	virtual QList<IDesktopBox*> outgoingBoxes() const;
	
public slots:
	virtual void setBatchMode(bool ab_Enabled);
	virtual void connectIncomingBox(IDesktopBox* ak_Other_);
	virtual void connectOutgoingBox(IDesktopBox* ak_Other_);
	virtual void disconnectBox(IDesktopBox* ak_Other_);
	virtual void handleIncomingBoxesChanged() = 0;
	
signals:
	virtual void batchModeChanged(bool);
	virtual void moved();
	virtual void resized();
	
protected:
	virtual void paintEvent(QPaintEvent* event);

	k_Desktop* mk_Desktop_;
	k_Proteomatic& mk_Proteomatic;
	
	bool mb_BatchMode;
	QSet<IDesktopBox*> mk_ConnectedIncomingBoxes;
	QSet<IDesktopBox*> mk_ConnectedOutgoingBoxes;
};
