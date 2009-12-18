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

#include "HintLineEdit.h"


k_HintLineEdit::k_HintLineEdit(QWidget* parent)
	: QLineEdit(parent)
{
	ms_Hint = "";
}


k_HintLineEdit::k_HintLineEdit(const QString& contents, QWidget* parent)
	: QLineEdit(contents, parent)
{
	this->setText(contents);
	ms_Hint = "";
}


k_HintLineEdit::~k_HintLineEdit()
{
}


void k_HintLineEdit::setHint(const QString& as_Hint)
{
	ms_Hint = as_Hint;
	this->repaint();
}


void k_HintLineEdit::paintEvent(QPaintEvent* event)
{
	QLineEdit::paintEvent(event);
	if (this->text().isEmpty() && (!this->hasFocus()))
	{
		QPainter lk_Painter(this);
		lk_Painter.setPen(QColor("#888"));
#ifdef WIN32
		lk_Painter.drawText(4, 14, ms_Hint);
#else
		lk_Painter.drawText(5, 17, ms_Hint);
#endif
	}
}


void k_HintLineEdit::focusOutEvent(QFocusEvent* event)
{
    emit focusOut();
}
