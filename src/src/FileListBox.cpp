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
	: k_DesktopBox(ak_Parent_, ak_Proteomatic, true)
	, mk_FileList(this, true, true)
	, mk_Label("<b>File list</b> (empty)", this)
{
	setupLayout();
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


void k_FileListBox::setBatchMode(bool ab_Enabled)
{
	k_DesktopBox::setBatchMode(ab_Enabled);
	if (mk_BatchModeButton.isChecked() != ab_Enabled)
		mk_BatchModeButton.setChecked(ab_Enabled);
	toggleUi();
	if (ab_Enabled)
		updateFilenameTags();
	mk_Desktop_->setHasUnsavedChanges(true);
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


void k_FileListBox::updateFilenameTags()
{
	mk_Desktop_->createFilenameTags(mk_FileList.files(), mk_TagForFilename, ms_PrefixWithoutTags);
	mk_Desktop_->setHasUnsavedChanges(true);
}


void k_FileListBox::setupLayout()
{
	QBoxLayout* lk_VLayout_;
	QBoxLayout* lk_HLayout_;
	
	connect(mk_Desktop_, SIGNAL(pipelineIdle(bool)), &mk_FileList, SLOT(setEnabled(bool)));

	lk_VLayout_ = new QVBoxLayout(this);
	lk_VLayout_->setContentsMargins(0, 0, 0, 0);
	
	lk_HLayout_ = new QHBoxLayout();
	lk_VLayout_->addLayout(lk_HLayout_);
	lk_HLayout_->addWidget(&mk_Label);
	lk_HLayout_->addStretch();
	mk_BatchModeButton.setIcon(QIcon(":icons/cycle.png"));
	mk_BatchModeButton.setCheckable(true);
	mk_BatchModeButton.setChecked(false);
	lk_HLayout_->addWidget(&mk_BatchModeButton);
	connect(&mk_BatchModeButton, SIGNAL(toggled(bool)), this, SLOT(setBatchMode(bool)));
	connect(mk_Desktop_, SIGNAL(pipelineIdle(bool)), &mk_BatchModeButton, SLOT(setEnabled(bool)));
	mk_BatchModeButton.hide();
	
	lk_HLayout_ = new QHBoxLayout();
	lk_HLayout_->addWidget(&mk_FileList);
	connect(&mk_FileList, SIGNAL(selectionChanged(bool)), this, SLOT(toggleUi()));
	connect(&mk_FileList, SIGNAL(changed()), this, SIGNAL(filenamesChanged()));
	connect(&mk_FileList, SIGNAL(changed()), this, SLOT(updateFilenameTags()));
	mk_FileList.resize(100, 100);
	
	QBoxLayout* lk_VSubLayout_ = new QVBoxLayout();
	lk_HLayout_->addLayout(lk_VSubLayout_);
	QToolButton* lk_AddFilesButton_ = new QToolButton(this);
	lk_AddFilesButton_->setIcon(QIcon(":icons/folder.png"));
	connect(lk_AddFilesButton_, SIGNAL(clicked()), this, SLOT(addFilesButtonClicked()));
	connect(mk_Desktop_, SIGNAL(pipelineIdle(bool)), lk_AddFilesButton_, SLOT(setEnabled(bool)));
	lk_VSubLayout_->addWidget(lk_AddFilesButton_);
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
