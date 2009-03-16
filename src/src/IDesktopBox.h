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

#include <QtCore>

struct IDesktopBox
{
	virtual ~IDesktopBox() {};
	
	virtual bool batchMode() const = 0;
	virtual QList<IDesktopBox*> incomingBoxes() const = 0;
	virtual QList<IDesktopBox*> outgoingBoxes() const = 0;
	
	// slots
	virtual void setBatchMode(bool ab_Enabled) = 0;
	virtual void connectIncomingBox(IDesktopBox* ak_Other_) = 0;
	virtual void connectOutgoingBox(IDesktopBox* ak_Other_) = 0;
	virtual void disconnectIncomingBox(IDesktopBox* ak_Other_, bool ab_EmitSignal = true) = 0;
	virtual void disconnectOutgoingBox(IDesktopBox* ak_Other_, bool ab_EmitSignal = true) = 0;
	virtual void disconnectAll() = 0;
	virtual void setResizable(bool ab_Enabled) = 0;
	
	// signals
	virtual void deleted() = 0;
	virtual void batchModeChanged(bool) = 0;
	virtual void moved(QPoint ak_Delta) = 0;
	virtual void resized() = 0;
	virtual void clicked(Qt::KeyboardModifiers ae_Modifiers) = 0;
	virtual void boxConnected(IDesktopBox* ak_Other_, bool ab_Incoming) = 0;
	virtual void boxDisconnected(IDesktopBox* ak_Other_, bool ab_Incoming) = 0;
};
