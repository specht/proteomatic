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

#include "FileList.h"


k_FileList::k_FileList(QWidget* ak_Parent_, bool ab_ReallyRemoveItems)
	: QListWidget(ak_Parent_)
	, mb_ReallyRemoveItems(ab_ReallyRemoveItems)
{
	setSelectionMode(QAbstractItemView::ExtendedSelection);
	connect(this, SIGNAL(itemSelectionChanged()), this, SLOT(selectionChanged()));
}


k_FileList::~k_FileList()
{
}


void k_FileList::forceRemove(QList<QListWidgetItem *> ak_List)
{
	emit remove(ak_List);
}


void k_FileList::removeSelection()
{
	//int li_Row = currentRow();

	if (mb_ReallyRemoveItems)
		foreach (QListWidgetItem* lk_Item_, selectedItems())
			delete lk_Item_;

	emit remove(selectedItems());

/*
	if (li_Row < count())
		setCurrentRow(li_Row);
		*/
}


void k_FileList::keyPressEvent(QKeyEvent* ak_Event_)
{
	if (ak_Event_->key() == Qt::Key_Delete)
	{
		ak_Event_->accept();
		removeSelection();
	}
	else
		QListWidget::keyPressEvent(ak_Event_);
}


void k_FileList::mouseDoubleClickEvent(QMouseEvent* event)
{
	event->accept();
	emit doubleClick();
}


void k_FileList::selectionChanged()
{
	emit selectionChanged(!selectedItems().empty());
}
