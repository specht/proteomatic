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

#include "ClickableLabel.h"


k_ClickableLabel::k_ClickableLabel(QWidget* parent, Qt::WindowFlags f)
    : QLabel(parent, f)
{
    this->setFocusPolicy(Qt::TabFocus);
}


k_ClickableLabel::k_ClickableLabel(const QString& text, QWidget* parent, Qt::WindowFlags f)
    : QLabel(text, parent, f)
{
    this->setFocusPolicy(Qt::TabFocus);
}


k_ClickableLabel::~k_ClickableLabel()
{
}


void k_ClickableLabel::mousePressEvent(QMouseEvent* event)
{
    event->accept();
    emit clicked();
    emit pressed();
    if (event->button() == Qt::LeftButton)
        emit leftClicked();
    if (event->button() == Qt::RightButton)
        emit rightClicked();
}


void k_ClickableLabel::mouseReleaseEvent(QMouseEvent* event)
{
    event->accept();
    emit released();
}


void k_ClickableLabel::mouseDoubleClickEvent(QMouseEvent* event)
{
    event->accept();
    emit doubleClicked();
}


void k_ClickableLabel::enterEvent(QMouseEvent* event)
{
    event->accept();
    emit enter();
}


void k_ClickableLabel::leaveEvent(QMouseEvent* event)
{
    event->accept();
    emit leave();
}


void k_ClickableLabel::focusInEvent(QFocusEvent* /*event*/)
{
    //this->setBackgroundRole(QPalette::AlternateBase);
}


void k_ClickableLabel::focusOutEvent(QFocusEvent* /*event*/)
{
    //this->setBackgroundRole(QPalette::NoRole);
}

