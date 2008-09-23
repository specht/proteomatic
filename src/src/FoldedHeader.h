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
#include "ClickableLabel.h"
#include "RefPtr.h"


class k_FoldedHeader: public QWidget
{
	Q_OBJECT
public:
	k_FoldedHeader(QWidget* ak_Buddy_ = NULL, QWidget* parent = 0, Qt::WindowFlags f = 0);
	k_FoldedHeader(const QString& text, QWidget* ak_Buddy_ = NULL, QWidget* parent = 0, Qt::WindowFlags f = 0);
	~k_FoldedHeader();
	void setSuffix(QString as_Text);
	bool buddyVisible();

public slots:
	void hideBuddy();
	void showBuddy();
	void toggleBuddy();

signals:
	void clicked();
	void enter();
	void leave();
	
protected slots:
	void update();	

protected:
	void init();
	virtual void mousePressEvent(QMouseEvent* event);
	virtual void enterEvent(QMouseEvent* event);
	virtual void leaveEvent(QMouseEvent* event);
	
	QWidget* mk_Buddy_;
	QString ms_Text;
	k_ClickableLabel mk_Label;
	k_ClickableLabel mk_Icon;
	QList<RefPtr<QPixmap> > mk_FoldedIcons;
	QTimer mk_Timer;
	int mi_CurrentIndex;
	bool mb_Increasing;
};
