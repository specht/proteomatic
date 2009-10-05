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
#include "IFileBox.h"
#include "DesktopBox.h"
#include "RefPtr.h"


class k_Proteomatic;

class k_InputGroupProxyBox: public k_DesktopBox
{
	Q_OBJECT
public:
	k_InputGroupProxyBox(k_Desktop* ak_Parent_, k_Proteomatic& ak_Proteomatic, QString as_Label, QString as_GroupKey);
	virtual ~k_InputGroupProxyBox();
	
	virtual const QString& groupKey() const;

	virtual QList<IFileBox*> fileBoxes() const;
	
signals:
	virtual void filenamesChanged();
	
protected slots:
	virtual void boxConnectedSlot(IDesktopBox* ak_Other_, bool ab_Incoming);
	virtual void boxDisconnectedSlot(IDesktopBox* ak_Other_, bool ab_Incoming);
	
protected:
	virtual void setupLayout();
	
	QString ms_Label;
	QString ms_GroupKey;
	QList<IFileBox*> mk_FileBoxes;
};
