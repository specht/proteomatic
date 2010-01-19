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


k_FileListBox::k_FileListBox(k_Desktop* ak_Parent_, k_Proteomatic& ak_Proteomatic)
    : k_DesktopBox(ak_Parent_, ak_Proteomatic, true, true)
    , mk_FileList(this, true, true)
    , mk_Label("<b>File list</b> (empty)", this)
    , mi_MinHeight(20)
{
    connect(&mk_FileList, SIGNAL(changed()), this, SIGNAL(changed()));
    setupLayout();
    int li_FontHeight = mk_FileList.font().pixelSize();
    if (li_FontHeight == -1)
        li_FontHeight = mk_FileList.font().pointSize();
    mi_MinHeight = li_FontHeight + mk_FileList.frameWidth() * 2;
    if (mi_MinHeight < 20)
        mi_MinHeight = 20;
    fprintf(stderr, "[%d]", mi_MinHeight);
    update();
}


k_FileListBox::~k_FileListBox()
{
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


void k_FileListBox::addPath(const QString& as_Path)
{
    mk_FileList.addInputFile(as_Path, false);
    mk_FileList.refresh();
    toggleUi();
    mk_Desktop_->setHasUnsavedChanges(true);
}


void k_FileListBox::addPaths(const QStringList& ak_Paths)
{
    mk_FileList.addInputFiles(ak_Paths, false);
    mk_FileList.refresh();
    toggleUi();
    mk_Desktop_->setHasUnsavedChanges(true);
}


void k_FileListBox::setBatchMode(bool ab_Enabled)
{
    k_DesktopBox::setBatchMode(ab_Enabled);
    if (mk_BatchModeButton.isChecked() != ab_Enabled)
        mk_BatchModeButton.setChecked(ab_Enabled);
    mk_Desktop_->setHasUnsavedChanges(true);
    update();
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
    mk_RemoveSelectionButton.setEnabled(!mk_FileList.selectedItems().empty());
    mk_RemoveSelectionButtonH.setEnabled(!mk_FileList.selectedItems().empty());
    QString ls_Label;
    if (batchMode())
        ls_Label = "<b>File batch</b>";
    else
        ls_Label = "<b>File list</b>";
    if (mk_FileList.fileCount() == 0)
        ls_Label += QString(" (empty)");
    else if (mk_FileList.fileCount() == 1)
        ls_Label += QString(" (1 file)");
    else
        ls_Label += QString(" (%1 files)").arg(mk_FileList.fileCount());
    mk_Label.setText(ls_Label);
    mk_FileList.refresh();
}


void k_FileListBox::update()
{
    // ----------------------------------
    // UPDATE ITERATION TAGS
    // ----------------------------------
    
    mk_Desktop_->createFilenameTags(mk_FileList.files(), mk_TagForFilename, ms_PrefixWithoutTags);
    // :TODO: attention, code duplication here (OutFileListBox.cpp)
    
    // build tag => filename hash
    mk_FilenamesForTag.clear();
    foreach (QString ls_Filename, mk_TagForFilename.keys())
    {
        QString ls_Tag = mk_TagForFilename[ls_Filename];
        if (!mk_FilenamesForTag.contains(ls_Tag))
            mk_FilenamesForTag[ls_Tag] = QStringList();
        mk_FilenamesForTag[ls_Tag] << ls_Filename;
    }
    
    if (mk_FileList.files().size() == 1)
    {
        mk_FileList.setMaximumHeight(mi_MinHeight);
        this->setResizable(true, false);
        mk_AddFilesButton.hide();
        mk_RemoveSelectionButton.hide();
        mk_AddFilesButtonH.show();
        mk_RemoveSelectionButtonH.show();
    }
    else
    {
        mk_FileList.setMaximumHeight(QWIDGETSIZE_MAX);
        this->setResizable(true, true);
        mk_AddFilesButtonH.hide();
        mk_RemoveSelectionButtonH.hide();
        mk_AddFilesButton.show();
        mk_RemoveSelectionButton.show();
    }
    
    mk_Desktop_->setHasUnsavedChanges(true);
    emit changed();
    toggleUi();
}


void k_FileListBox::setupLayout()
{
    QBoxLayout* lk_VLayout_;
    QBoxLayout* lk_HLayout_;
    
    connect(mk_Desktop_, SIGNAL(pipelineIdle(bool)), &mk_FileList, SLOT(setEnabled(bool)));

    lk_VLayout_ = new QVBoxLayout(this);
    lk_VLayout_->setContentsMargins(11, 11, 11, 11);
    
    lk_HLayout_ = new QHBoxLayout();
    lk_VLayout_->addLayout(lk_HLayout_);
    lk_HLayout_->addWidget(&mk_Label);
    lk_HLayout_->addStretch();
    
    // horizontal add/remove buttons
    mk_AddFilesButtonH.setIcon(QIcon(":icons/folder.png"));
    connect(&mk_AddFilesButtonH, SIGNAL(clicked()), this, SLOT(addFilesButtonClicked()));
    connect(mk_Desktop_, SIGNAL(pipelineIdle(bool)), &mk_AddFilesButtonH, SLOT(setEnabled(bool)));
    lk_HLayout_->addWidget(&mk_AddFilesButtonH);
    mk_RemoveSelectionButtonH.setIcon(QIcon(":icons/list-remove.png"));
    lk_HLayout_->addWidget(&mk_RemoveSelectionButtonH);
    connect(mk_Desktop_, SIGNAL(pipelineIdle(bool)), &mk_RemoveSelectionButtonH, SLOT(setEnabled(bool)));
    connect(&mk_RemoveSelectionButtonH, SIGNAL(clicked()), &mk_FileList, SLOT(removeSelection()));
    
    mk_BatchModeButton.setIcon(QIcon(":icons/cycle.png"));
    mk_BatchModeButton.setCheckable(true);
    mk_BatchModeButton.setChecked(false);
    lk_HLayout_->addWidget(&mk_BatchModeButton);
    connect(&mk_BatchModeButton, SIGNAL(toggled(bool)), this, SLOT(setBatchMode(bool)));
    connect(mk_Desktop_, SIGNAL(pipelineIdle(bool)), &mk_BatchModeButton, SLOT(setEnabled(bool)));

    lk_HLayout_ = new QHBoxLayout();
    lk_HLayout_->addWidget(&mk_FileList);
    connect(&mk_FileList, SIGNAL(selectionChanged(bool)), this, SLOT(toggleUi()));
    connect(&mk_FileList, SIGNAL(changed()), this, SLOT(update()));
    mk_FileList.resize(100, 100);
    
    QBoxLayout* lk_VSubLayout_ = new QVBoxLayout();
    lk_HLayout_->addLayout(lk_VSubLayout_);
    mk_AddFilesButton.setIcon(QIcon(":icons/folder.png"));
    connect(&mk_AddFilesButton, SIGNAL(clicked()), this, SLOT(addFilesButtonClicked()));
    connect(mk_Desktop_, SIGNAL(pipelineIdle(bool)), &mk_AddFilesButton, SLOT(setEnabled(bool)));
    lk_VSubLayout_->addWidget(&mk_AddFilesButton);
    mk_RemoveSelectionButton.setIcon(QIcon(":icons/list-remove.png"));
    lk_VSubLayout_->addWidget(&mk_RemoveSelectionButton);
    connect(mk_Desktop_, SIGNAL(pipelineIdle(bool)), &mk_RemoveSelectionButton, SLOT(setEnabled(bool)));
    connect(&mk_RemoveSelectionButton, SIGNAL(clicked()), &mk_FileList, SLOT(removeSelection()));
    lk_VSubLayout_->addStretch();
    
    k_ClickableLabel* lk_ArrowLabel_ = new k_ClickableLabel(this);
    lk_ArrowLabel_->setPixmap(QPixmap(":icons/arrow-semi-transparent.png").scaledToWidth(20, Qt::SmoothTransformation));
    lk_VSubLayout_->addWidget(lk_ArrowLabel_);
    
    connect(lk_ArrowLabel_, SIGNAL(pressed()), this, SIGNAL(arrowPressed()));
    connect(lk_ArrowLabel_, SIGNAL(released()), this, SIGNAL(arrowReleased()));
    
    lk_VLayout_->addLayout(lk_HLayout_);
    toggleUi();
    emit resized();
}
