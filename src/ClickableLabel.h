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


class k_ClickableLabel: public QLabel
{
    Q_OBJECT
public:
    k_ClickableLabel(QWidget* parent = 0, Qt::WindowFlags f = 0);
    k_ClickableLabel(const QString& text, QWidget* parent = 0, Qt::WindowFlags f = 0);
    ~k_ClickableLabel();

signals:
    void clicked();
    void leftClicked();
    void rightClicked();
    void doubleClicked();
    void pressed();
    void released();
    void enter();
    void leave();

protected:
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);
    virtual void mouseDoubleClickEvent(QMouseEvent* event);
    virtual void enterEvent(QMouseEvent* event);
    virtual void leaveEvent(QMouseEvent* event);
    virtual void focusInEvent(QFocusEvent* event);
    virtual void focusOutEvent(QFocusEvent* event);
};
