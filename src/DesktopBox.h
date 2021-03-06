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


class k_DesktopBox: public QWidget, public IDesktopBox
{
    Q_OBJECT
public:
    k_DesktopBox(k_Desktop* ak_Parent_, k_Proteomatic& ak_Proteomatic, bool ab_ResizableX = true, bool ab_ResizableY = true);
    virtual ~k_DesktopBox();
    
    virtual int topologicalIndex() const;
    virtual void updateTopologicalIndex();
    virtual bool hasAnchestor(IDesktopBox* ak_Other_);
    virtual void resize(int w, int h);
    virtual void resize(const QSize& ak_Size);
    
    // IDesktopBox
    
    virtual bool batchMode() const;
    virtual bool protectedFromUserDeletion() const;
    virtual void setProtectedFromUserDeletion(bool ab_Flag);
    virtual QSet<IDesktopBox*> incomingBoxes() const;
    virtual QSet<IDesktopBox*> outgoingBoxes() const;
    virtual QSet<IDesktopBox*> incomingBoxesRecursive(bool ab_IncludingSelf = false);
    virtual QSet<IDesktopBox*> outgoingBoxesRecursive(bool ab_IncludingSelf = false);
    virtual QString description();
    virtual void update();
    
public slots:
    virtual void setBatchMode(bool ab_Enabled);
    virtual void connectIncomingBox(IDesktopBox* ak_Other_);
    virtual void connectOutgoingBox(IDesktopBox* ak_Other_);
    virtual void disconnectIncomingBox(IDesktopBox* ak_Other_);
    virtual void disconnectOutgoingBox(IDesktopBox* ak_Other_);
    virtual void disconnectAll();
    virtual void setResizable(bool ab_EnabledX, bool ab_EnabledY);
    virtual void toggleUi();
    virtual QRectF rect();
    virtual void invalidate();
    virtual void invalidateNext(int ai_Distance = 1);
    
signals:
    void moved(QPoint ak_Delta);
    void resized();
    void clicked(QMouseEvent* event);
    void boxConnected(IDesktopBox* ak_Other_, bool ab_Incoming);
    void boxDisconnected(IDesktopBox* ak_Other_, bool ab_Incoming);
    void requestGlobalUpdate();
    
protected:
    virtual void resizeEvent(QResizeEvent* event);
    virtual void moveEvent(QMoveEvent* event);
    virtual void paintEvent(QPaintEvent* event);
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);
    virtual void mouseMoveEvent(QMouseEvent* event);
    virtual void compactSize();
    
protected:
    k_Desktop* mk_Desktop_;
    k_Proteomatic& mk_Proteomatic;
    int mi_TopologicalIndex;
    bool mb_ResizableX, mb_ResizableY;
    QPixmap mk_ResizeGripPixmap;
    
    bool mb_BatchMode;
    bool mb_ProtectedFromUserDeletion;
    QSet<IDesktopBox*> mk_ConnectedIncomingBoxes;
    QSet<IDesktopBox*> mk_ConnectedOutgoingBoxes;
    
    bool mb_Moving;
    bool mb_Resizing;
    QPoint mk_MousePressPosition;
    QPoint mk_OldPosition;
    QSize mk_OldSize;
};
