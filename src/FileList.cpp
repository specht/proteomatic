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


k_FileList::k_FileList(QWidget* ak_Parent_, bool ab_ReallyRemoveItems, 
                       k_Proteomatic& ak_Proteomatic, bool ab_FileMode)
    : QListWidget(ak_Parent_)
    , mk_Proteomatic(ak_Proteomatic)
    , mk_OpenFileAction(QIcon(":icons/document-open.png"), "&Open file", this)
    , mk_OpenContainingFolderAction(QIcon(":icons/folder.png"), "Open containing &folder", this)
    , mk_RemoveFileFromListAction(QIcon(":icons/list-remove.png"), "&Remove from list", this)
    , mk_DeleteFileAction(QIcon(":icons/user-trash.png"), "&Delete file", this)
    , mb_ReallyRemoveItems(ab_ReallyRemoveItems)
    , mb_FileMode(ab_FileMode)
    , mb_Refreshing(false)
    , mi_AvailableFileCount(0)
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
    connect(&mk_RemoveFileFromListAction, SIGNAL(triggered()), this, SLOT(menuRemoveFileFromListSlot()));
    connect(&mk_DeleteFileAction, SIGNAL(triggered()), this, SLOT(menuDeleteFileSlot()));
    mk_PopupMenu.addAction(&mk_OpenFileAction);
    mk_PopupMenu.addAction(&mk_OpenContainingFolderAction);
    mk_PopupMenu.addSeparator();
    mk_PopupMenu.addAction(&mk_RemoveFileFromListAction);
    mk_PopupMenu.addAction(&mk_DeleteFileAction);
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
    mk_Files[as_Key] = QStringList();
    
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
    if (!mk_Keys.empty())
    {
        foreach (QString ls_Key, mk_Keys)
        {
            if (mk_Files[ls_Key].contains(as_Path))
                return;
        }
    }
    else
    {
        if (mk_Files[""].contains(as_Path))
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
    mk_Files[ls_MatchingKey].append(as_Path);
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
    {
        this->addInputFile(ls_Path, false, false);
    }

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
        lk_AllFiles += mk_Files[""];
    }
    else
    {
        foreach (QString ls_Key, mk_Files.keys())
            lk_AllFiles += mk_Files[ls_Key];
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


int k_FileList::availableFileCount() const
{
    return mi_AvailableFileCount;
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
                        mk_Files[ls_Key].removeOne(ls_Path);
                }
                else
                    mk_Files[""].removeOne(ls_Path);
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
    QStringList lk_Paths;
    foreach (QUrl lk_Url, event->mimeData()->urls())
    {
        QString ls_Path = lk_Url.toLocalFile();
        if (!ls_Path.isEmpty())
            if (QFileInfo(ls_Path).isFile())
                lk_Paths << ls_Path;
    }
    if (!lk_Paths.empty())
        addInputFiles(lk_Paths, true);
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

    mi_AvailableFileCount = 0;
    if (mk_Keys.empty())
    {
        QList<QListWidgetItem*> lk_ToBeDeleted;
        QSet<QString> lk_Paths = mk_Files[""].toSet();
        for (int i = 0; i < count(); ++i)
        {
            QListWidgetItem* lk_Item_ = item(i);
            QString ls_Path = lk_Item_->data(Qt::UserRole).toString();
            if (mk_Files[""].contains(ls_Path))
            {
                bool lb_Exists = QFileInfo(ls_Path).exists();
                lk_Item_->setForeground(lb_Exists ? QBrush(TANGO_SKY_BLUE_2) : QBrush(TANGO_ALUMINIUM_3));
                if (lb_Exists)
                    ++mi_AvailableFileCount;
            }
            else
                lk_ToBeDeleted.push_back(lk_Item_);
            
            lk_Paths.remove(ls_Path);
        }
        
        foreach (QListWidgetItem* lk_Item_, lk_ToBeDeleted)
            delete takeItem(row(lk_Item_));
        
        QStringList lk_PathsSorted = lk_Paths.toList();
        qSort(lk_PathsSorted.begin(), lk_PathsSorted.end());
        foreach (QString ls_Path, lk_PathsSorted)
        {
            QListWidgetItem* lk_Item_ = new QListWidgetItem(QFileInfo(ls_Path).fileName(), this);
            lk_Item_->setData(Qt::UserRole, ls_Path);
            bool lb_Exists = QFileInfo(ls_Path).exists();
            lk_Item_->setForeground(lb_Exists ? QBrush(TANGO_SKY_BLUE_2) : QBrush(TANGO_ALUMINIUM_3));
            if (lb_Exists)
                ++mi_AvailableFileCount;
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
            QStringList lk_Paths = mk_Files[ls_Key];
            qSort(lk_Paths.begin(), lk_Paths.end());
            foreach (QString ls_Path, lk_Paths)
            {
                QString ls_Filename = QFileInfo(ls_Path).fileName();
                QListWidgetItem* lk_Item_ = new QListWidgetItem(ls_Filename, this);
                lk_Item_->setData(Qt::UserRole, ls_Path);
                bool lb_Exists = QFileInfo(ls_Path).exists();
                lk_Item_->setForeground(lb_Exists ? QBrush("#000") : QBrush(TANGO_ALUMINIUM_3));
                if (lb_Exists)
                    ++mi_AvailableFileCount;
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
    if (!mb_FileMode)
        return;
    
    if (!ak_Item_)
        return;
    
    if (ak_Point == QPoint())
        ak_Point = QCursor::pos();
    
    bool lb_OneItemSelected = selectedItems().size() == 1;
    
    QString ls_Path = QDir::cleanPath(QFileInfo(ak_Item_->data(Qt::UserRole).toString()).absoluteFilePath());
    mk_OpenFileAction.setEnabled(lb_OneItemSelected && QFileInfo(ls_Path).exists());
    QSet<QString> lk_SelectedFolders;
    foreach (QListWidgetItem* lk_Item_, selectedItems())
        lk_SelectedFolders << QFileInfo(lk_Item_->data(Qt::UserRole).toString()).absolutePath();
    mk_OpenContainingFolderAction.setEnabled((lk_SelectedFolders.size() == 1) && QFileInfo(QFileInfo(ls_Path).absolutePath()).isDir());
    mk_RemoveFileFromListAction.setText(lb_OneItemSelected ? "&Remove from list" : "&Remove selected items from list");
    mk_DeleteFileAction.setText(lb_OneItemSelected ? "&Delete file" : "&Delete selected files");
    mk_RemoveFileFromListAction.setEnabled(mb_ReallyRemoveItems);
    mk_DeleteFileAction.setEnabled(QFileInfo(ls_Path).exists());
    
    mk_PopupMenu.exec(ak_Point);
}


void k_FileList::menuOpenFileSlot()
{
    openFile(currentItem());
}


void k_FileList::menuOpenContainingDirectorySlot()
{
    openContainingDirectory(currentItem());
}


void k_FileList::menuRemoveFileFromListSlot()
{
    removeFileFromList(currentItem());
}


void k_FileList::menuDeleteFileSlot()
{
    deleteFile(currentItem());
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


void k_FileList::removeFileFromList(QListWidgetItem* ak_Item_)
{
    if (!ak_Item_)
        return;
    
    removeSelection();
}


void k_FileList::deleteFile(QListWidgetItem* ak_Item_)
{
    if (!ak_Item_)
        return;
    
    if (selectedItems().size() > 1)
    {
        QStringList lk_Paths;
        foreach (QListWidgetItem* lk_Item_, selectedItems())
        {
            QString ls_Path = lk_Item_->data(Qt::UserRole).toString();
            if (QFileInfo(ls_Path).exists())
                lk_Paths << ls_Path;
        }
        if (lk_Paths.size() > 0)
        {
            if (mk_Proteomatic.showMessageBox("Delete selected files", QString("Are you sure you want to delete %1 file%2?").arg(lk_Paths.size()).arg(lk_Paths.size() == 1 ? "" : "s"), ":icons/dialog-warning.png", QMessageBox::Yes | QMessageBox::No, QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
            {
                QDir lk_Dir;
                int li_ErrorCount = 0;
                foreach (QString ls_Path, lk_Paths)
                {
                    if (!lk_Dir.remove(ls_Path))
                        ++li_ErrorCount;
                }
                if (li_ErrorCount > 0)
                    mk_Proteomatic.showMessageBox("Delete selected files", 
                                                  QString("Error: Unable to delete %1 of %2 files.").
                                                    arg(li_ErrorCount).arg(lk_Paths.size()), 
                                                  ":icons/dialog-warning.png");
            }
        }
    }
    else
    {
        QString ls_Path = ak_Item_->data(Qt::UserRole).toString();
        if (QFileInfo(ls_Path).exists())
        {
            ls_Path = QDir::cleanPath(ls_Path);
            if (mk_Proteomatic.showMessageBox("Delete file", QString("Are you sure you want to delete %1?").arg(ls_Path), ":icons/dialog-warning.png", QMessageBox::Yes | QMessageBox::No, QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
            {
                QDir lk_Dir;
                if (!lk_Dir.remove(ls_Path))
                    mk_Proteomatic.showMessageBox("Delete file", "Error: The file could not be deleted.", ":icons/dialog-warning.png");
            }
        }
    }
}
