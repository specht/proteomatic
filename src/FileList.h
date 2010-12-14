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
#include "Proteomatic.h"


class k_FileList: public QListWidget
{
    Q_OBJECT
public:
    k_FileList(QWidget* ak_Parent_, bool ab_ReallyRemoveItems, k_Proteomatic& ak_Proteomatic, bool ab_FileMode = false);
    ~k_FileList();
    void resetAll(bool ab_EmitSignal = true);
    void forceRemove(QList<QListWidgetItem *> ak_List);
    void addInputFileGroup(QString as_Key, QString as_Label, QStringList ak_Extensions);
    void addInputFile(QString as_Path, bool ab_Refresh = true, bool ab_EmitSignal = true);
    void addInputFiles(QStringList ak_Paths, bool ab_Refresh = true, bool ab_EmitSignal = true);
    // TODO: tell files sorted by key to solve ambiguous cases
    QStringList files() const;
    int fileCount() const;
    int availableFileCount() const;

signals:
    void remove(QList<QListWidgetItem *>);
    void selectionChanged(bool);
    void doubleClick();
    void myItemDoubleClicked(QListWidgetItem*);
    void myItemRightClicked(QListWidgetItem*);
    void changed();

public slots:
    void removeSelection();
    void selectionChanged();
    void refresh();
    
protected slots:
    virtual void itemDoubleClicked(QListWidgetItem* ak_Item_);
    virtual void showFilePopupMenu(QListWidgetItem* ak_Item_, QPoint ak_Point = QPoint());
    virtual void menuOpenFileSlot();
    virtual void menuOpenContainingDirectorySlot();
    virtual void menuRemoveFileFromListSlot();
    virtual void menuDeleteFileSlot();
    virtual void openFile(QListWidgetItem* ak_Item_);
    virtual void openContainingDirectory(QListWidgetItem* ak_Item_);
    virtual void removeFileFromList(QListWidgetItem* ak_Item_);
    virtual void deleteFile(QListWidgetItem* ak_Item_);

protected:
    virtual void keyPressEvent(QKeyEvent* ak_Event_);
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseDoubleClickEvent(QMouseEvent* event);
    virtual void dragEnterEvent(QDragEnterEvent* event);
    virtual void dragMoveEvent(QDragMoveEvent* event);
    virtual void dropEvent(QDropEvent* event);
    virtual Qt::DropActions supportedDropActions() const;
    
    k_Proteomatic& mk_Proteomatic;
    QStringList mk_Keys;
    QHash<QString, QString> mk_Labels;
    QHash<QString, QStringList> mk_Extensions;
    QHash<QString, QMap<QString, bool> > mk_Files;
    QAction mk_OpenFileAction;
    QAction mk_OpenContainingFolderAction;
    QAction mk_RemoveFileFromListAction;
    QAction mk_DeleteFileAction;
    QMenu mk_PopupMenu;
    int mi_AvailableFileCount;

private:
    bool mb_ReallyRemoveItems;
    bool mb_FileMode;
    bool mb_Refreshing;
};
