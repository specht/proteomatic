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

#include "FileListBox.h"
#include "ClickableLabel.h"
#include "Desktop.h"
#include "Proteomatic.h"
#include "Tango.h"
#include "UnclickableLabel.h"


k_FileListBox::k_FileListBox(k_Desktop* ak_Parent_, k_Proteomatic& ak_Proteomatic,
                             IScriptBox* ak_ScriptBoxParent_)
    : k_DesktopBox(ak_Parent_, ak_Proteomatic, true, true)
    , mb_ListMode(!ak_ScriptBoxParent_)
    , ms_Key("")
    , ms_Label("")
    , mk_FileList(this, !ak_ScriptBoxParent_, true)
    , mk_Label("<b>File list</b> (empty)", this)
    , mi_MinHeight(21)
    , mk_OpenFileAction_(new QAction(QIcon(":icons/document-open.png"), "&Open file", this))
    , mk_OpenContainingFolderAction_(new QAction(QIcon(":icons/folder.png"), "Open containing &folder", this))
    , mk_InactiveArrow(QPixmap(":icons/arrow-semi-semi-transparent.png").scaledToWidth(20, Qt::SmoothTransformation))
    , mk_ActiveArrow(QPixmap(":icons/arrow-semi-transparent.png").scaledToWidth(20, Qt::SmoothTransformation))
    , mk_ScriptBoxParent_(ak_ScriptBoxParent_)
    , mk_LastListModeSize(QSize(300, 125))
{
    setupLayout();
    int li_FontHeight = mk_FileList.font().pixelSize();
    if (li_FontHeight == -1)
        li_FontHeight = mk_FileList.font().pointSize();
    mi_MinHeight = li_FontHeight + mk_FileList.frameWidth() * 2;
    if (mi_MinHeight < 21)
        mi_MinHeight = 21;
    update();
}


k_FileListBox::~k_FileListBox()
{
}


void k_FileListBox::setKey(QString as_Key)
{
    ms_Key = as_Key;
}


void k_FileListBox::setLabel(QString as_Label)
{
    ms_Label = as_Label;
    ms_Label[0] = ms_Label[0].toUpper();
    toggleUi();
}


QStringList k_FileListBox::filenames() const
{
    return mk_FileList.files();
}


QString k_FileListBox::tagForFilename(const QString& as_Filename) const
{
    if (mk_TagForFilename.contains(as_Filename))
        return mk_TagForFilename[as_Filename];
    else
        return QFileInfo(as_Filename).baseName();
}


QStringList k_FileListBox::filenamesForTag(const QString& as_Tag) const
{
    if (!batchMode())
        return mk_FileList.files();
    
    if (mk_FilenamesForTag.contains(as_Tag))
        return mk_FilenamesForTag[as_Tag];
    else
        return QStringList();
}


QString k_FileListBox::prefixWithoutTags() const
{
    return ms_PrefixWithoutTags;
}


void k_FileListBox::setListMode(bool ab_Enabled)
{
    bool lb_OldListMode = mb_ListMode;
    mb_ListMode = ab_Enabled;
    if (!mk_ScriptBoxParent_)
        mb_ListMode = true;
    if (lb_OldListMode && (!mb_ListMode))
        mk_LastListModeSize = size();
    if ((!lb_OldListMode) && mb_ListMode)
        resize(mk_LastListModeSize);
}


bool k_FileListBox::listMode() const
{
    return mb_ListMode;
}


void k_FileListBox::addPath(const QString& as_Path)
{
    mk_FileList.addInputFile(as_Path, false);
    mk_FileList.refresh();
    toggleUi();
    if ((!mk_ScriptBoxParent_) && batchMode() && (mk_FileList.fileCount() > 0))
        invalidate();
}


void k_FileListBox::addPaths(const QStringList& ak_Paths)
{
    mk_FileList.addInputFiles(ak_Paths, false);
    mk_FileList.refresh();
    toggleUi();
    if ((!mk_ScriptBoxParent_) && batchMode() && (mk_FileList.fileCount() > 0))
        invalidate();
}


void k_FileListBox::setBatchMode(bool ab_Enabled)
{
    k_DesktopBox::setBatchMode(ab_Enabled);
    if (mk_BatchModeButton.isChecked() != ab_Enabled)
        mk_BatchModeButton.setChecked(ab_Enabled);
    invalidate();
    invalidateNext(batchMode() ? 2 : 1);
    mk_Desktop_->invalidate();
    if ((!mk_ScriptBoxParent_) && batchMode() && (mk_FileList.fileCount() > 0))
        invalidate();
    toggleUi();
}


void k_FileListBox::addFilesButtonClicked()
{
    QStringList lk_Files = QFileDialog::getOpenFileNames(mk_Proteomatic.messageBoxParent(), tr("Add files"), mk_Proteomatic.getConfiguration(CONFIG_REMEMBER_INPUT_FILES_PATH).toString());
    if (!lk_Files.empty())
    {
        mk_Proteomatic.getConfigurationRoot()[CONFIG_REMEMBER_INPUT_FILES_PATH] = QFileInfo(lk_Files[0]).absolutePath();
        foreach (QString ls_Path, lk_Files)
            mk_FileList.addInputFile(ls_Path, false);
        mk_FileList.refresh();
        toggleUi();
        mk_Desktop_->setHasUnsavedChanges(true);
    }
}


void k_FileListBox::toggleUi()
{
    if (!mk_ScriptBoxParent_)
        mb_ListMode = true;
    
    setResizable(mb_ListMode, mb_ListMode);
    mk_FileName.setVisible((!mb_ListMode) && (mk_FileList.fileCount() > 0));
    mk_FileList.setVisible(mb_ListMode);
    mk_FileList.refresh();
    
    mk_BatchModeButton.setVisible(mb_ListMode);
    mk_RemoveSelectionButton.setEnabled(!mk_FileList.selectedItems().empty());
    QString ls_Label;
    if (ms_Label.isEmpty())
    {
        if (batchMode())
            ls_Label = "<b>File batch</b>";
        else
            ls_Label = "<b>File list</b>";
    }
    else
        ls_Label = "<b>" + ms_Label + "</b>";

    if (mb_ListMode)
    {
        if (mk_FileList.fileCount() == 0)
            ls_Label += QString(" (empty)");
        else if (mk_FileList.fileCount() == 1)
            ls_Label += QString(" (1 file)");
        else
            ls_Label += QString(" (%1 files)").arg(mk_FileList.fileCount());
    }
    mk_Label.setText(ls_Label);
    mk_FileList.refresh();
    if ((!mb_ListMode) && (mk_FileList.fileCount() > 0))
    {
        QString ls_Path = mk_FileList.files().first();
        setToolTip(ls_Path);
        QFontMetrics lk_FontMetrics(mk_FileName.font());
        QString ls_PrintPath = QFileInfo(ls_Path).fileName();
        if (lk_FontMetrics.boundingRect(ls_Path).width() > 300)
            ls_PrintPath = lk_FontMetrics.elidedText(ls_PrintPath, Qt::ElideLeft, 300);
        if (QFileInfo(ls_Path).exists())
            mk_FileName.setText(QString("<span style='color: %1'>").arg(TANGO_SKY_BLUE_2) + ls_PrintPath + "</span>");
        else
            mk_FileName.setText(QString("<span style='color: %1'>").arg(TANGO_ALUMINIUM_3) + ls_PrintPath + "</span>");
    }

}


void k_FileListBox::filenameDoubleClicked()
{
    QString ls_Path = filenames().first();
    if (QFileInfo(ls_Path).exists())
        k_Proteomatic::openFileLink(ls_Path);
}


void k_FileListBox::showContextMenu()
{
    QString ls_Path = mk_FileList.files().first();
    mk_OpenFileAction_->setEnabled(QFileInfo(ls_Path).exists());
    ls_Path = QFileInfo(ls_Path).absolutePath();
    mk_OpenContainingFolderAction_->setEnabled(QFileInfo(ls_Path).isDir());
    mk_PopupMenu.exec(QCursor::pos());
}


void k_FileListBox::invalidate()
{
    k_DesktopBox::invalidate();
    if (batchMode())
        invalidateNext(2);
}


void k_FileListBox::update()
{
    if (mk_ScriptBoxParent_)
    {
        mk_FileList.resetAll(false);
        mk_FileList.addInputFiles(mk_ScriptBoxParent_->outputFilesForKey(ms_Key));
        
        bool lb_ParentInBatchMode = dynamic_cast<IDesktopBox*>(mk_ScriptBoxParent_)->batchMode();
        if (!lb_ParentInBatchMode)
        {
            setListMode(false);
            setBatchMode(false);
        }
        
        setListMode(lb_ParentInBatchMode);
    }
    // ----------------------------------
    // UPDATE ITERATION TAGS
    // ----------------------------------
    
    mk_Desktop_->createFilenameTags(mk_FileList.files(), mk_TagForFilename, ms_PrefixWithoutTags);
    
    // build tag => filename hash
    mk_FilenamesForTag.clear();
    foreach (QString ls_Filename, mk_TagForFilename.keys())
    {
        QString ls_Tag = mk_TagForFilename[ls_Filename];
        if (!mk_FilenamesForTag.contains(ls_Tag))
            mk_FilenamesForTag[ls_Tag] = QStringList();
        mk_FilenamesForTag[ls_Tag] << ls_Filename;
    }
    
    mk_Desktop_->setHasUnsavedChanges(true);
    toggleUi();
}


void k_FileListBox::setupLayout()
{
    QBoxLayout* lk_VLayout_;
    QBoxLayout* lk_HLayout_;
    
    connect(mk_Desktop_, SIGNAL(pipelineIdle(bool)), &mk_FileList, SLOT(setEnabled(bool)));

    mk_PopupMenu.addAction(mk_OpenFileAction_);
    mk_PopupMenu.addAction(mk_OpenContainingFolderAction_);
    connect(mk_OpenFileAction_, SIGNAL(triggered()), this, SLOT(openFile()));
    connect(mk_OpenContainingFolderAction_, SIGNAL(triggered()), this, SLOT(openContainingDirectory()));
    
    lk_VLayout_ = new QVBoxLayout(this);
    lk_VLayout_->setContentsMargins(11, 11, 11, 11);
    
    lk_HLayout_ = new QHBoxLayout();
    lk_VLayout_->addLayout(lk_HLayout_);
    lk_HLayout_->addWidget(&mk_Label);
    lk_HLayout_->addStretch();
    
    // horizontal add/remove buttons
    if (!mk_ScriptBoxParent_)
    {
        mk_AddFilesButton.setIcon(QIcon(":icons/folder.png"));
        connect(&mk_AddFilesButton, SIGNAL(clicked()), this, SLOT(addFilesButtonClicked()));
        connect(mk_Desktop_, SIGNAL(pipelineIdle(bool)), &mk_AddFilesButton, SLOT(setEnabled(bool)));
        lk_HLayout_->addWidget(&mk_AddFilesButton);
        mk_RemoveSelectionButton.setIcon(QIcon(":icons/list-remove.png"));
        lk_HLayout_->addWidget(&mk_RemoveSelectionButton);
        connect(mk_Desktop_, SIGNAL(pipelineIdle(bool)), &mk_RemoveSelectionButton, SLOT(setEnabled(bool)));
        connect(&mk_RemoveSelectionButton, SIGNAL(clicked()), &mk_FileList, SLOT(removeSelection()));
    }
    
    mk_BatchModeButton.setIcon(QIcon(":icons/cycle.png"));
    mk_BatchModeButton.setCheckable(true);
    mk_BatchModeButton.setChecked(false);
    lk_HLayout_->addWidget(&mk_BatchModeButton);
    connect(&mk_BatchModeButton, SIGNAL(toggled(bool)), this, SLOT(setBatchMode(bool)));
    connect(mk_Desktop_, SIGNAL(pipelineIdle(bool)), &mk_BatchModeButton, SLOT(setEnabled(bool)));

    lk_HLayout_ = new QHBoxLayout();
    lk_HLayout_->addWidget(&mk_FileList);
    connect(&mk_FileList, SIGNAL(selectionChanged(bool)), this, SLOT(toggleUi()));
    connect(&mk_FileList, SIGNAL(changed()), this, SLOT(fileBoxChanged()));
    
    lk_HLayout_->addWidget(&mk_FileName);
    
    QBoxLayout* lk_VSubLayout_ = new QVBoxLayout();
    lk_HLayout_->addLayout(lk_VSubLayout_);
    lk_VSubLayout_->addStretch();
    
    mk_ArrowLabel.setPixmap(mk_InactiveArrow);
    lk_VSubLayout_->addWidget(&mk_ArrowLabel);
    
    mk_FileList.setMinimumHeight(mi_MinHeight);
    
    connect(&mk_ArrowLabel, SIGNAL(pressed()), this, SIGNAL(arrowPressed()));
    connect(&mk_ArrowLabel, SIGNAL(pressed()), this, SLOT(arrowPressedSlot()));
    connect(&mk_ArrowLabel, SIGNAL(released()), this, SIGNAL(arrowReleased()));
    connect(&mk_ArrowLabel, SIGNAL(released()), this, SLOT(arrowReleasedSlot()));
    
    connect(&mk_FileName, SIGNAL(doubleClicked()), this, SLOT(filenameDoubleClicked()));
    connect(&mk_FileName, SIGNAL(rightClicked()), this, SLOT(showContextMenu()));

    
    lk_VLayout_->addLayout(lk_HLayout_);
    
    toggleUi();
    emit resized();
}


void k_FileListBox::arrowPressedSlot()
{
    mk_ArrowLabel.setPixmap(mk_ActiveArrow);
}


void k_FileListBox::arrowReleasedSlot()
{
    mk_ArrowLabel.setPixmap(mk_InactiveArrow);
}


void k_FileListBox::openFile()
{
    if (mk_FileList.files().empty())
        return;
    
    QString ls_Path = mk_FileList.files().first();
    if (QFileInfo(ls_Path).exists())
        k_Proteomatic::openFileLink(ls_Path);
}


void k_FileListBox::openContainingDirectory()
{
    if (mk_FileList.files().empty())
        return;
    
    QString ls_Path = mk_FileList.files().first();
    ls_Path = QFileInfo(ls_Path).absolutePath();
    if (QFileInfo(ls_Path).isDir())
        k_Proteomatic::openFileLink(ls_Path);
}


void k_FileListBox::fileBoxChanged()
{
    invalidate();
}
