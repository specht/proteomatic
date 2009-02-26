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


k_FileList::k_FileList(QWidget* ak_Parent_, bool ab_ReallyRemoveItems, bool ab_FileMode)
	: QListWidget(ak_Parent_)
	, mb_ReallyRemoveItems(ab_ReallyRemoveItems)
	, mb_FileMode(ab_FileMode)
{
	setSelectionMode(QAbstractItemView::ExtendedSelection);
	connect(this, SIGNAL(itemSelectionChanged()), this, SLOT(selectionChanged()));
}


k_FileList::~k_FileList()
{
}


void k_FileList::resetAll()
{
	if (mb_FileMode)
	{
		mk_Keys.clear();
		mk_Labels.clear();
		mk_Extensions.clear();
		mk_Files.clear();
	}
	this->clear();
	emit changed();
}


void k_FileList::forceRemove(QList<QListWidgetItem *> ak_List)
{
	emit remove(ak_List);
	emit changed();
}


void k_FileList::addInputFileGroup(QString as_Key, QString as_Label, QStringList ak_Extensions)
{
	if (!mb_FileMode)
		return;
	
	QString ls_Label = as_Label;
	ls_Label.replace(0, 1, ls_Label.left(1).toUpper());

	mk_Keys.push_back(as_Key);
	mk_Labels[as_Key] = ls_Label;
	mk_Extensions[as_Key] = ak_Extensions;
	mk_Files[as_Key] = QStringList();
	
	QListWidgetItem* lk_Item_ = new QListWidgetItem(ls_Label + " files (0)", this);
	lk_Item_->setFlags(Qt::ItemIsEnabled);
	QFont lk_Font = lk_Item_->font();
	lk_Font.setBold(true);
	lk_Item_->setFont(lk_Font);
}


void k_FileList::addInputFile(QString as_Path)
{
	if (!mb_FileMode)
		return;
	
	// check if file is already in the file list, return if so!
	foreach (QString ls_Key, mk_Keys)
	{
		if (mk_Files[ls_Key].contains(as_Path))
			return;
	}
	
	QString ls_LowerPath = as_Path.toLower();
	QString ls_MatchingKey = "";
	foreach (QString ls_Key, mk_Keys)
	{
		foreach (QString ls_Extension, mk_Extensions[ls_Key])
		{
			if (ls_LowerPath.right(ls_Extension.length()) == ls_Extension.toLower())
			{
				if (ls_MatchingKey != "")
				{
					// oops! we have ambiguous file extensions... TODO!
				}
				ls_MatchingKey = ls_Key;
			}
		}
	}
	mk_Files[ls_MatchingKey].push_back(as_Path);
	this->refresh();
	emit changed();
}


QStringList k_FileList::files() const
{
	QStringList lk_AllFiles;
	if (mk_Keys.empty())
	{
		lk_AllFiles += mk_Files[""];
	}
	else
	{
		foreach (QString ls_Key, mk_Files.keys())
			lk_AllFiles += mk_Files[ls_Key];
	}
	
	return lk_AllFiles;
}


void k_FileList::removeSelection()
{
	if (mb_FileMode)
	{
		foreach (QListWidgetItem* lk_Item_, selectedItems())
		{
			QString ls_Path = lk_Item_->text();
			if (!mk_Keys.empty())
			{
				foreach (QString ls_Key, mk_Keys)
					mk_Files[ls_Key].removeOne(ls_Path);
			}
			else
				mk_Files[""].removeOne(ls_Path);
		}
		this->refresh();
	}
	else
	{
		if (mb_ReallyRemoveItems)
			foreach (QListWidgetItem* lk_Item_, selectedItems())
				delete lk_Item_;
	}
		
	emit remove(selectedItems());
	emit changed();
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


void k_FileList::refresh()
{
	if (!mb_FileMode)
		return;
	
	this->clear();
	if (mk_Keys.empty())
	{
		foreach (QString ls_Path, mk_Files[""])
		{
			QListWidgetItem* lk_Item_ = new QListWidgetItem(ls_Path, this);
			(void)lk_Item_;
		}
	}
	else
	{
		foreach (QString ls_Key, mk_Keys)
		{
			QListWidgetItem* lk_Item_ = new QListWidgetItem(mk_Labels[ls_Key] + QString(" files (%1)").arg(mk_Files[ls_Key].size()), this);
			lk_Item_->setFlags(Qt::ItemIsEnabled);
			QFont lk_Font = lk_Item_->font();
			lk_Font.setBold(true);
			lk_Item_->setFont(lk_Font);
			foreach (QString ls_Path, mk_Files[ls_Key])
			{
				QListWidgetItem* lk_Item_ = new QListWidgetItem(ls_Path, this);
				(void)lk_Item_;
			}
		}
	}
}
