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


class k_FileList: public QListWidget
{
	Q_OBJECT
public:
	k_FileList(QWidget* ak_Parent_, bool ab_ReallyRemoveItems);
	~k_FileList();
	void forceRemove(QList<QListWidgetItem *> ak_List);

signals:
	void remove(QList<QListWidgetItem *>);
	void selectionChanged(bool);
	void doubleClick();

public slots:
	void removeSelection();
	void selectionChanged();

protected:
	virtual void keyPressEvent(QKeyEvent* ak_Event_);
	virtual void mouseDoubleClickEvent(QMouseEvent* event);

private:
	bool mb_ReallyRemoveItems;
};
