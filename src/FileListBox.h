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
#include "IFileBox.h"
#include "IDesktopBox.h"
#include "IScriptBox.h"
#include "DesktopBox.h"
#include "FileList.h"
#include "ClickableLabel.h"
#include "UnclickableLabel.h"


class k_Proteomatic;

class k_FileListBox: public k_DesktopBox, public IFileBox
{
    Q_OBJECT
public:
    k_FileListBox(k_Desktop* ak_Parent_, k_Proteomatic& ak_Proteomatic, IScriptBox* ak_ScriptBoxParent_);
    virtual ~k_FileListBox();

    virtual QString description();
    virtual void setKey(QString as_Key);
    virtual void setLabel(QString as_Label);
    
    virtual QStringList filenames() const;  
    virtual QString tagForFilename(const QString& as_Filename) const;
    virtual QStringList filenamesForTag(const QString& as_Tag) const;
    virtual QString prefixWithoutTags() const;
    virtual void setListMode(bool ab_Enabled);
    virtual bool listMode() const;
    virtual IScriptBox* scriptBoxParent() const;
    virtual void addPath(const QString& as_Path);
    virtual void addPaths(const QStringList& ak_Paths);
    virtual void update();
    
public slots:
    virtual void connectOutgoingBox(IDesktopBox* ak_Other_);
    virtual void disconnectOutgoingBox(IDesktopBox* ak_Other_);
    virtual void setBatchMode(bool ab_Enabled);
    virtual void invalidate();
    
signals:
    void arrowPressed();
    void arrowReleased();
    void filenamesChanged();
    
protected slots:
    virtual void addFilesButtonClicked();
    virtual void toggleUi();
    virtual void filenameDoubleClicked();
    virtual void showContextMenu();
    virtual void arrowPressedSlot();
    virtual void arrowReleasedSlot();
    virtual void openFile();
    virtual void openContainingDirectory();
    virtual void deleteFile();
    virtual void fileBoxChanged();
    
protected:
    virtual void setupLayout();
    
    bool mb_ListMode;
    QString ms_Key;
    QString ms_Label;
    k_FileList mk_FileList;
    k_UnclickableLabel mk_Label;
    int mi_MinHeight;
    QToolButton mk_AddFilesButton;
    QToolButton mk_RemoveSelectionButton;
    QToolButton mk_BatchModeButton;
    QHash<QString, QString> mk_TagForFilename;
    QHash<QString, QStringList> mk_FilenamesForTag;
    QString ms_PrefixWithoutTags;
    QMenu mk_PopupMenu;
    QAction* mk_OpenFileAction_;
    QAction* mk_OpenContainingFolderAction_;
    QAction* mk_DeleteFileAction_;
    QPixmap mk_InactiveArrow;
    QPixmap mk_ActiveArrow;
    k_ClickableLabel mk_ArrowLabel;
    k_ClickableLabel mk_FileName;
    IScriptBox* mk_ScriptBoxParent_;
    QSize mk_LastListModeSize;
};
