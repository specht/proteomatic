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

#include "UnclickableLabel.h"


k_UnclickableLabel::k_UnclickableLabel(QWidget* parent, Qt::WindowFlags f)
	: QLabel(parent, f)
{
}


k_UnclickableLabel::k_UnclickableLabel(const QString& text, QWidget* parent, Qt::WindowFlags f)
	: QLabel(text, parent, f)
{
}


k_UnclickableLabel::~k_UnclickableLabel()
{
}


void k_UnclickableLabel::mousePressEvent(QMouseEvent* event)
{
	event->ignore();
}


void k_UnclickableLabel::mouseReleaseEvent(QMouseEvent* event)
{
	event->ignore();
}


void k_UnclickableLabel::mouseMoveEvent(QMouseEvent* event)
{
	event->ignore();
}


void k_UnclickableLabel::enterEvent(QMouseEvent* event)
{
	event->ignore();
}


void k_UnclickableLabel::leaveEvent(QMouseEvent* event)
{
	event->ignore();
}


void k_UnclickableLabel::focusInEvent(QFocusEvent* event)
{
	event->ignore();
}


void k_UnclickableLabel::focusOutEvent(QFocusEvent* event)
{
	event->ignore();
}

