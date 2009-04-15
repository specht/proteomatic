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
	k_FileList(QWidget* ak_Parent_, bool ab_ReallyRemoveItems, bool ab_FileMode = false);
	~k_FileList();
	void resetAll();
	void forceRemove(QList<QListWidgetItem *> ak_List);
	void addInputFileGroup(QString as_Key, QString as_Label, QStringList ak_Extensions);
	void addInputFile(QString as_Path, bool ab_Refresh = true);
	// TODO: tell files sorted by key to solve ambiguous cases
	QStringList files() const;
	int fileCount() const;

signals:
	void remove(QList<QListWidgetItem *>);
	void selectionChanged(bool);
	void doubleClick();
	void changed();

public slots:
	void removeSelection();
	void selectionChanged();
	void refresh();

protected:
	virtual void keyPressEvent(QKeyEvent* ak_Event_);
	virtual void mouseDoubleClickEvent(QMouseEvent* event);
	
	QStringList mk_Keys;
	QHash<QString, QString> mk_Labels;
	QHash<QString, QStringList> mk_Extensions;
	QHash<QString, QMap<QString, bool> > mk_Files;

private:
	bool mb_ReallyRemoveItems;
	bool mb_FileMode;
};
