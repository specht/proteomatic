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
	this->updateStatus();
	connect(this, SIGNAL(textEdited(QString&)), this, SLOT(updateStatus()));
}


k_HintLineEdit::k_HintLineEdit(const QString& contents, QWidget* parent)
	: QLineEdit(contents, parent)
	, ms_Text(contents)
{
	this->setText(contents);
	this->updateStatus();
	connect(this, SIGNAL(textEdited(QString&)), this, SLOT(updateStatus()));
}


k_HintLineEdit::~k_HintLineEdit()
{
}


void k_HintLineEdit::setHint(const QString& as_Hint)
{
	ms_Hint = as_Hint;
	this->updateStatus();
}


QString k_HintLineEdit::text() const
{
	return ms_Text;
}


void k_HintLineEdit::mousePressEvent(QMouseEvent* event)
{
	QLineEdit::mousePressEvent(event);
}


void k_HintLineEdit::updateStatus()
{
	if (ms_Text.isEmpty())
	{
		this->setStyleSheet("color: #888;");
		this->setText(ms_Hint);
	}
	else
	{
		this->setStyleSheet("color: #000;");
		this->setText(ms_Text);
	}
}
