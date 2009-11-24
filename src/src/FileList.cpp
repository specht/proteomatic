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
#include "Tango.h"
#include "Proteomatic.h"


k_FileList::k_FileList(QWidget* ak_Parent_, bool ab_ReallyRemoveItems, bool ab_FileMode)
	: QListWidget(ak_Parent_)
	, mb_ReallyRemoveItems(ab_ReallyRemoveItems)
	, mb_FileMode(ab_FileMode)
	, mb_Refreshing(false)
	, mk_OpenFileAction(QIcon(":icons/document-open.png"), "&Open file", this)
	, mk_OpenContainingFolderAction(QIcon(":icons/folder.png"), "Open containing &folder", this)
{
    setMinimumWidth(200);
	setSelectionMode(QAbstractItemView::ExtendedSelection);
	setAcceptDrops(true);
	setDragDropMode(QAbstractItemView::DropOnly);
	connect(this, SIGNAL(itemSelectionChanged()), this, SLOT(selectionChanged()));
	connect(this, SIGNAL(myItemDoubleClicked(QListWidgetItem*)), this, SLOT(itemDoubleClicked(QListWidgetItem*)));
	connect(this, SIGNAL(myItemRightClicked(QListWidgetItem*)), this, SLOT(showFilePopupMenu(QListWidgetItem*)));
	connect(&mk_OpenFileAction, SIGNAL(triggered()), this, SLOT(menuOpenFileSlot()));
	connect(&mk_OpenContainingFolderAction, SIGNAL(triggered()), this, SLOT(menuOpenContainingDirectorySlot()));
	mk_PopupMenu.addAction(&mk_OpenFileAction);
	mk_PopupMenu.addAction(&mk_OpenContainingFolderAction);
}


k_FileList::~k_FileList()
{
}


void k_FileList::resetAll(bool ab_EmitSignal)
{
	if (mb_FileMode)
	{
		mk_Keys.clear();
		mk_Labels.clear();
		mk_Extensions.clear();
		mk_Files.clear();
	}
	this->clear();
	if (ab_EmitSignal)
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
	mk_Files[as_Key] = QMap<QString, bool>();
	
	QListWidgetItem* lk_Item_ = new QListWidgetItem(ls_Label + " files (0)", this);
	lk_Item_->setFlags(Qt::ItemIsEnabled);
	QFont lk_Font = lk_Item_->font();
	lk_Font.setBold(true);
	lk_Item_->setFont(lk_Font);
}


void k_FileList::addInputFile(QString as_Path, bool ab_Refresh, bool ab_EmitSignal)
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
	mk_Files[ls_MatchingKey][as_Path] = true;
	if (ab_Refresh)
		this->refresh();
	if (ab_EmitSignal)
		emit changed();
}


void k_FileList::addInputFiles(QStringList ak_Paths, bool ab_Refresh, bool ab_EmitSignal)
{
	if (!mb_FileMode)
		return;
	
	foreach (QString ls_Path, ak_Paths)
		this->addInputFile(ls_Path, false, false);

	if (ab_Refresh)
		this->refresh();
	if (ab_EmitSignal)
		emit changed();
}


QStringList k_FileList::files() const
{
	QStringList lk_AllFiles;
	if (mk_Keys.empty())
	{
		lk_AllFiles += mk_Files[""].keys();
	}
	else
	{
		foreach (QString ls_Key, mk_Files.keys())
			lk_AllFiles += mk_Files[ls_Key].keys();
	}
	
	return lk_AllFiles;
}


int k_FileList::fileCount() const
{
	int li_Count = 0;
	foreach (QString ls_Key, mk_Files.keys())
		li_Count += mk_Files[ls_Key].size();
	return li_Count;
}


void k_FileList::removeSelection()
{
	if (mb_FileMode)
	{
		if (mb_ReallyRemoveItems)
		{
			foreach (QListWidgetItem* lk_Item_, selectedItems())
			{
				QString ls_Path = lk_Item_->data(Qt::UserRole).toString();
				if (!mk_Keys.empty())
				{
					foreach (QString ls_Key, mk_Keys)
						mk_Files[ls_Key].remove(ls_Path);
				}
				else
					mk_Files[""].remove(ls_Path);
			}
			this->refresh();
		}
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
	if (ak_Event_->matches(QKeySequence::Delete) || ak_Event_->key() == Qt::Key_Backspace)
	{
		ak_Event_->accept();
		removeSelection();
	}
	else if (ak_Event_->key() == Qt::Key_Menu && currentItem())
	{
		ak_Event_->accept();
		showFilePopupMenu(currentItem(), mapToGlobal(pos()));
	}
	else
		QListWidget::keyPressEvent(ak_Event_);
}


void k_FileList::mousePressEvent(QMouseEvent* event)
{
	// always let the parent handle this event and then show
	// the popup menu if there's a popup menu to show
	QListWidget::mousePressEvent(event);
	
	QListWidgetItem* lk_Item_ = itemAt(event->pos());
	if (lk_Item_ && event->button() == Qt::RightButton)
	{
		event->accept();
		emit myItemRightClicked(lk_Item_);
	}
}


void k_FileList::mouseDoubleClickEvent(QMouseEvent* event)
{
	event->accept();
	emit doubleClick();
	QListWidgetItem* lk_Item_ = itemAt(event->pos());
	if (lk_Item_)
		emit myItemDoubleClicked(lk_Item_);
}


void k_FileList::dragEnterEvent(QDragEnterEvent* event)
{
	event->acceptProposedAction();
}


void k_FileList::dragMoveEvent(QDragMoveEvent* event)
{
	event->acceptProposedAction();
}


void k_FileList::dropEvent(QDropEvent* event)
{
	event->accept();
	foreach (QUrl lk_Url, event->mimeData()->urls())
	{
		QString ls_Path = lk_Url.toLocalFile();
		if (!ls_Path.isEmpty())
			if (QFileInfo(ls_Path).isFile())
				addInputFile(ls_Path, true);
	}
}


Qt::DropActions k_FileList::supportedDropActions() const
{
	return Qt::ActionMask;
}


void k_FileList::selectionChanged()
{
	emit selectionChanged(!selectedItems().empty());
}


void k_FileList::refresh()
{
	if (mb_Refreshing)
		return;
	
	mb_Refreshing = true;
	
	if (!mb_FileMode)
		return;

	if (mk_Keys.empty())
	{
		QList<QListWidgetItem*> lk_ToBeDeleted;
		QSet<QString> lk_Paths = mk_Files[""].keys().toSet();
		for (int i = 0; i < count(); ++i)
		{
			QListWidgetItem* lk_Item_ = item(i);
			QString ls_Path = lk_Item_->data(Qt::UserRole).toString();
			if (mk_Files[""].contains(ls_Path))
				lk_Item_->setForeground(QFileInfo(ls_Path).exists() ? QBrush(TANGO_SKY_BLUE_2) : QBrush(TANGO_ALUMINIUM_3));
			else
				lk_ToBeDeleted.push_back(lk_Item_);
			
			lk_Paths.remove(ls_Path);
		}
		
		foreach (QListWidgetItem* lk_Item_, lk_ToBeDeleted)
			delete takeItem(row(lk_Item_));
		
		foreach (QString ls_Path, lk_Paths)
		{
			QListWidgetItem* lk_Item_ = new QListWidgetItem(QFileInfo(ls_Path).fileName(), this);
			lk_Item_->setData(Qt::UserRole, ls_Path);
			lk_Item_->setForeground(QFileInfo(ls_Path).exists() ? QBrush(TANGO_SKY_BLUE_2) : QBrush(TANGO_ALUMINIUM_3));
		}
	}
	else
	{
		// TODO: update instead of clear and re-insert
		this->clear();
		foreach (QString ls_Key, mk_Keys)
		{
			QListWidgetItem* lk_Item_ = new QListWidgetItem(mk_Labels[ls_Key] + QString(" files (%1)").arg(mk_Files[ls_Key].size()), this);
			lk_Item_->setFlags(Qt::ItemIsEnabled);
			QFont lk_Font = lk_Item_->font();
			lk_Font.setBold(true);
			lk_Item_->setFont(lk_Font);
            QStringList lk_Paths = mk_Files[ls_Key].keys();
            qSort(lk_Paths.begin(), lk_Paths.end());
			foreach (QString ls_Path, lk_Paths)
			{
				QString ls_Filename = QFileInfo(ls_Path).fileName();
				QListWidgetItem* lk_Item_ = new QListWidgetItem(ls_Filename, this);
				lk_Item_->setData(Qt::UserRole, ls_Path);
				lk_Item_->setForeground(QFileInfo(ls_Path).exists() ? QBrush("#000") : QBrush(TANGO_ALUMINIUM_3));
			}
		}
	}
	mb_Refreshing = false;
}


void k_FileList::itemDoubleClicked(QListWidgetItem* ak_Item_)
{
	//printf("double click: %s\n", ak_Item_->data(Qt::UserRole).toString().toStdString().c_str());
	openFile(ak_Item_);
}


void k_FileList::showFilePopupMenu(QListWidgetItem* ak_Item_, QPoint ak_Point)
{
	if (ak_Point == QPoint())
		ak_Point = QCursor::pos();
	QString ls_Path = ak_Item_->data(Qt::UserRole).toString();
	mk_PopupMenu.exec(ak_Point);
	//if (QFileInfo(ls_Path).exists())
		//k_Proteomatic::openFileLink(ls_Path);
}


void k_FileList::menuOpenFileSlot()
{
	openFile(currentItem());
}


void k_FileList::menuOpenContainingDirectorySlot()
{
	openContainingDirectory(currentItem());
}


void k_FileList::openFile(QListWidgetItem* ak_Item_)
{
	if (!ak_Item_)
		return;
	
	QString ls_Path = ak_Item_->data(Qt::UserRole).toString();
	if (QFileInfo(ls_Path).exists())
		k_Proteomatic::openFileLink(ls_Path);
}


void k_FileList::openContainingDirectory(QListWidgetItem* ak_Item_)
{
	if (!ak_Item_)
		return;

	QString ls_Path = ak_Item_->data(Qt::UserRole).toString();
	if (QFileInfo(QFileInfo(ls_Path).absolutePath()).isDir())
		k_Proteomatic::openFileLink(QFileInfo(ls_Path).absolutePath());
}
