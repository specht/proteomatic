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
#include <QtGui>


struct IDesktopBox
{
    virtual ~IDesktopBox() {};

    virtual int topologicalIndex() const = 0;
    virtual void updateTopologicalIndex() = 0;
    virtual bool hasAnchestor(IDesktopBox* ak_Other_) = 0;
    virtual bool batchMode() const = 0;
    virtual bool protectedFromUserDeletion() const = 0;
    virtual void setProtectedFromUserDeletion(bool ab_Flag) = 0;
    virtual QSet<IDesktopBox*> incomingBoxes() const = 0;
    virtual QSet<IDesktopBox*> outgoingBoxes() const = 0;
    virtual QSet<IDesktopBox*> incomingBoxesRecursive(bool ab_IncludingSelf) = 0;
    virtual QSet<IDesktopBox*> outgoingBoxesRecursive(bool ab_IncludingSelf) = 0;
    virtual QString description() = 0;
    virtual void update() = 0;
    
    // slots
    virtual void setBatchMode(bool ab_Enabled) = 0;
    virtual void connectIncomingBox(IDesktopBox* ak_Other_) = 0;
    virtual void connectOutgoingBox(IDesktopBox* ak_Other_) = 0;
    virtual void disconnectIncomingBox(IDesktopBox* ak_Other_) = 0;
    virtual void disconnectOutgoingBox(IDesktopBox* ak_Other_) = 0;
    virtual void disconnectAll() = 0;
    virtual void setResizable(bool ab_EnabledX, bool ab_EnabledY) = 0;
    virtual void toggleUi() = 0;
    virtual QRectF rect() = 0;
    // invalidate applies to self
    virtual void invalidate() = 0;
    // invalidateNext applies to the next level boxes, and maybe even further
    virtual void invalidateNext(int ai_Distance = 1) = 0;
    
    // signals
    virtual void moved(QPoint ak_Delta) = 0;
    virtual void resized() = 0;
    virtual void clicked(QMouseEvent* event) = 0;
    virtual void boxConnected(IDesktopBox* ak_Other_, bool ab_Incoming) = 0;
    virtual void boxDisconnected(IDesktopBox* ak_Other_, bool ab_Incoming) = 0;
    virtual void requestGlobalUpdate() = 0;
};
