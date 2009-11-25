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

#include "OutFileListBox.h"
#include "ClickableLabel.h"
#include "Desktop.h"
#include "Tango.h"
#include "UnclickableLabel.h"
#include "Proteomatic.h"


k_OutFileListBox::k_OutFileListBox(k_Desktop* ak_Parent_, 
								   k_Proteomatic& ak_Proteomatic,
								   QString as_Key, 
								   QString as_Label, 
								   bool ab_ItemsDeletable/* = true*/)
	: k_DesktopBox(ak_Parent_, ak_Proteomatic, true)
	, ms_Key(as_Key)
	, ms_Label(as_Label)
	, mk_FileList(this, ab_ItemsDeletable, true)
	, mb_ListMode(false)
    , mk_OpenFileAction_(new QAction(QIcon(":icons/document-open.png"), "&Open file", this))
    , mk_OpenContainingFolderAction_(new QAction(QIcon(":icons/folder.png"), "Open containing &folder", this))
{
	if (!ms_Label.isEmpty())
		ms_Label[0] = ms_Label[0].toUpper();
	setupLayout();
}


k_OutFileListBox::~k_OutFileListBox()
{
}


QStringList k_OutFileListBox::filenames() const
{
	return mk_FileList.files();
}


QString k_OutFileListBox::tagForFilename(const QString& as_Filename) const
{
	if (mk_TagForFilename.contains(as_Filename))
		return mk_TagForFilename[as_Filename];
	else
		return QFileInfo(as_Filename).baseName();
}


QStringList k_OutFileListBox::filenamesForTag(const QString& as_Tag) const
{
    if (!batchMode())
        return mk_FileList.files();
    
	if (mk_FilenamesForTag.contains(as_Tag))
		return mk_FilenamesForTag[as_Tag];
	else
		return QStringList();
}


QString k_OutFileListBox::prefixWithoutTags() const
{
	return ms_PrefixWithoutTags;
}


void k_OutFileListBox::setListMode(bool ab_Enabled)
{
	mb_ListMode = ab_Enabled;
	if (!ab_Enabled)
		this->setBatchMode(false);
	toggleUi();
}


bool k_OutFileListBox::listMode() const
{
	return mb_ListMode;
}


QString k_OutFileListBox::label() const
{
	return ms_Label;
}


void k_OutFileListBox::setBatchMode(bool ab_Enabled)
{
    if (!mb_ListMode)
        ab_Enabled = false;
    
	k_DesktopBox::setBatchMode(ab_Enabled);
	if (mk_BatchModeButton.isChecked() != ab_Enabled)
		mk_BatchModeButton.setChecked(ab_Enabled);
    emit changed();
    mk_Desktop_->setHasUnsavedChanges(true);
}


void k_OutFileListBox::toggleUi()
{
	setResizable(mb_ListMode, mb_ListMode);
	mk_FileName_->setVisible((!mb_ListMode) && (mk_FileList.fileCount() > 0));
	mk_FileList.setVisible(mb_ListMode);
	mk_FileList.refresh();
	
	mk_BatchModeButton.setVisible(mb_ListMode);
	
	QString ls_String = "<b>" + ms_Label + "</b>";
    if (mb_ListMode && (mk_FileList.fileCount() > 0))
        ls_String += QString(" (%1 file%2)").arg(mk_FileList.fileCount()).arg(mk_FileList.fileCount() == 1 ? "" : "s");
	mk_Label_->setText(ls_String);
	if ((!mb_ListMode) && (mk_FileList.fileCount() > 0))
	{
		QString ls_Path = mk_FileList.files().first();
		setToolTip(ls_Path);
		QFontMetrics lk_FontMetrics(mk_FileName_->font());
		QString ls_PrintPath = QFileInfo(ls_Path).fileName();
		if (lk_FontMetrics.boundingRect(ls_Path).width() > 300)
			ls_PrintPath = lk_FontMetrics.elidedText(ls_PrintPath, Qt::ElideLeft, 300);
		if (QFileInfo(ls_Path).exists())
			mk_FileName_->setText(QString("<span style='color: %1'>").arg(TANGO_SKY_BLUE_2) + ls_PrintPath + "</span>");
		else
			mk_FileName_->setText(QString("<span style='color: %1'>").arg(TANGO_ALUMINIUM_3) + ls_PrintPath + "</span>");
	}
}


void k_OutFileListBox::filenameDoubleClicked()
{
	QString ls_Path = filenames().first();
	if (QFileInfo(ls_Path).exists())
		k_Proteomatic::openFileLink(ls_Path);
}


void k_OutFileListBox::update()
{
	if (mk_ConnectedIncomingBoxes.size() == 0)
		return;
	
	IDesktopBox* lk_DesktopScriptBox_ = mk_ConnectedIncomingBoxes.toList().first();
	IScriptBox* lk_ScriptBox_ = dynamic_cast<IScriptBox*>(lk_DesktopScriptBox_);
	if (!lk_ScriptBox_)
		return;
	
	// ----------------------------------------------
	// HANDLE BATCH MODE / SCRIPT TYPE
	// ----------------------------------------------
	if (lk_ScriptBox_->script()->type() == r_ScriptType::Processor)
		setListMode(lk_DesktopScriptBox_->batchMode());
	else
		setListMode(true);
	
	// ----------------------------------------------
	// UPDATE FILENAMES
	// ----------------------------------------------
	
	mk_FileList.resetAll(false);
	mk_FileList.addInputFiles(lk_ScriptBox_->outputFilesForKey(ms_Key), true, false);
    
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
    
    mk_Desktop_->setHasUnsavedChanges(true);
    emit changed();
	toggleUi();
}


void k_OutFileListBox::showContextMenu()
{
    QString ls_Path = mk_FileList.files().first();
    mk_OpenFileAction_->setEnabled(QFileInfo(ls_Path).exists());
    ls_Path = QFileInfo(ls_Path).absolutePath();
    mk_OpenContainingFolderAction_->setEnabled(QFileInfo(ls_Path).isDir());
    mk_PopupMenu.exec(QCursor::pos());
}


void k_OutFileListBox::openFile()
{
    if (mk_FileList.files().empty())
        return;
    
    QString ls_Path = mk_FileList.files().first();
    if (QFileInfo(ls_Path).exists())
        k_Proteomatic::openFileLink(ls_Path);
}


void k_OutFileListBox::openContainingDirectory()
{
    if (mk_FileList.files().empty())
        return;
    
    QString ls_Path = mk_FileList.files().first();
    ls_Path = QFileInfo(ls_Path).absolutePath();
    if (QFileInfo(ls_Path).isDir())
        k_Proteomatic::openFileLink(ls_Path);
}


void k_OutFileListBox::setupLayout()
{
	QBoxLayout* lk_VLayout_;
	QBoxLayout* lk_HLayout_;
	
	lk_HLayout_ = new QHBoxLayout(this);
	lk_HLayout_->setContentsMargins(11, 11, 11, 11);
	
	connect(mk_Desktop_, SIGNAL(pipelineIdle(bool)), &mk_FileList, SLOT(setEnabled(bool)));
	
	lk_VLayout_ = new QVBoxLayout();
	lk_HLayout_->addLayout(lk_VLayout_);
	mk_Label_ = new k_UnclickableLabel("", this);
    mk_PopupMenu.addAction(mk_OpenFileAction_);
    mk_PopupMenu.addAction(mk_OpenContainingFolderAction_);
    connect(mk_OpenFileAction_, SIGNAL(triggered()), this, SLOT(openFile()));
    connect(mk_OpenContainingFolderAction_, SIGNAL(triggered()), this, SLOT(openContainingDirectory()));
    
	mk_FileName_ = new k_ClickableLabel("", this);
	connect(mk_FileName_, SIGNAL(doubleClicked()), this, SLOT(filenameDoubleClicked()));
    connect(mk_FileName_, SIGNAL(rightClicked()), this, SLOT(showContextMenu()));
	lk_VLayout_->addWidget(mk_Label_);
	lk_VLayout_->addWidget(mk_FileName_);
	lk_VLayout_->addWidget(&mk_FileList);
	
	lk_VLayout_ = new QVBoxLayout();
	
	mk_BatchModeButton.setIcon(QIcon(":icons/cycle.png"));
	mk_BatchModeButton.setCheckable(true);
	mk_BatchModeButton.setChecked(false);
	
	lk_VLayout_->addWidget(&mk_BatchModeButton);
	connect(&mk_BatchModeButton, SIGNAL(toggled(bool)), this, SLOT(setBatchMode(bool)));
	connect(mk_Desktop_, SIGNAL(pipelineIdle(bool)), &mk_BatchModeButton, SLOT(setEnabled(bool)));
	
	k_ClickableLabel* lk_ArrowLabel_ = new k_ClickableLabel(this);
	lk_ArrowLabel_->setPixmap(QPixmap(":icons/arrow-semi-transparent.png").scaledToWidth(20, Qt::SmoothTransformation));
	lk_VLayout_->addStretch();
	lk_VLayout_->addWidget(lk_ArrowLabel_);
    lk_HLayout_->addLayout(lk_VLayout_);
	
	mk_FileList.hide();
	setResizable(false, false);
	mk_BatchModeButton.hide();
	
	connect(lk_ArrowLabel_, SIGNAL(pressed()), this, SIGNAL(arrowPressed()));
	connect(lk_ArrowLabel_, SIGNAL(released()), this, SIGNAL(arrowReleased()));
	
	connect(this, SIGNAL(resized()), this, SLOT(toggleUi()));
	
	connect(&mk_FileList, SIGNAL(changed()), this, SIGNAL(changed()));
	
	resize(300, 100);
	emit resized();
    toggleUi();
}
