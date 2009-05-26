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


k_OutFileListBox::k_OutFileListBox(k_Desktop* ak_Parent_, k_Proteomatic& ak_Proteomatic,
									QString as_Label)
	: k_DesktopBox(ak_Parent_, ak_Proteomatic, true)
	, ms_Label(as_Label)
	, mk_FileList(this, true, true)
	, mb_ListMode(false)
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


QString k_OutFileListBox::prefixWithoutTags() const
{
	return ms_PrefixWithoutTags;
}


void k_OutFileListBox::setFilenames(QStringList ak_Filenames)
{
	mk_FileList.resetAll();
	foreach (QString ls_Path, ak_Filenames)
		mk_FileList.addInputFile(ls_Path, false);
	mk_FileList.refresh();
	
	toggleUi();
	updateFilenameTags();
	emit filenamesChanged();
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
	k_DesktopBox::setBatchMode(ab_Enabled);
	if (mk_BatchModeButton.isChecked() != ab_Enabled)
		mk_BatchModeButton.setChecked(ab_Enabled);
}


void k_OutFileListBox::toggleUi()
{
	setResizable(mb_ListMode);
	mk_FileName_->setVisible((!mb_ListMode) && (mk_FileList.fileCount() > 0));
	mk_FileList.setVisible(mb_ListMode);
	mk_BatchModeButton.setVisible(mb_ListMode);
	QString ls_String = "<b>" + ms_Label + "</b>";
	mk_Label_->setText(ls_String);
	if ((!mb_ListMode) && (mk_FileList.fileCount() > 0))
	{
		QString ls_Path = mk_FileList.files().first();
		if (QFileInfo(ls_Path).exists())
			mk_FileName_->setText("<a href='file://" + ls_Path + "'>" + QFileInfo(ls_Path).fileName() + "</a>");
		else
			mk_FileName_->setText(QFileInfo(ls_Path).fileName());
	}
}


void k_OutFileListBox::updateFilenameTags()
{
	mk_Desktop_->createFilenameTags(mk_FileList.files(), mk_TagForFilename, ms_PrefixWithoutTags);
}


void k_OutFileListBox::linkActivated(const QString& as_Url)
{
	QDesktopServices::openUrl(as_Url);
}


void k_OutFileListBox::setupLayout()
{
	QBoxLayout* lk_VLayout_;
	QBoxLayout* lk_HLayout_;
	
	lk_HLayout_ = new QHBoxLayout(this);
	
	lk_VLayout_ = new QVBoxLayout();
	lk_HLayout_->addLayout(lk_VLayout_);
	mk_Label_ = new k_UnclickableLabel("", this);
	mk_FileName_ = new QLabel("", this);
	lk_VLayout_->addWidget(mk_Label_);
	lk_VLayout_->addWidget(mk_FileName_);
	connect(mk_FileName_, SIGNAL(linkActivated(const QString&)), this, SLOT(linkActivated(const QString&)));
	lk_VLayout_->addWidget(&mk_FileList);
	
	lk_VLayout_ = new QVBoxLayout();
	lk_HLayout_->addLayout(lk_VLayout_);
	
	mk_BatchModeButton.setIcon(QIcon(":icons/cycle.png"));
	mk_BatchModeButton.setCheckable(true);
	mk_BatchModeButton.setChecked(false);
	lk_VLayout_->addWidget(&mk_BatchModeButton);
	connect(&mk_BatchModeButton, SIGNAL(toggled(bool)), this, SLOT(setBatchMode(bool)));
	
	k_ClickableLabel* lk_ArrowLabel_ = new k_ClickableLabel(this);
	lk_ArrowLabel_->setPixmap(QPixmap(":icons/arrow-semi-transparent.png").scaledToWidth(24, Qt::SmoothTransformation));
	lk_VLayout_->addStretch();
	lk_VLayout_->addWidget(lk_ArrowLabel_);
	
	mk_FileList.hide();
	setResizable(false);
	mk_BatchModeButton.hide();
	
	connect(lk_ArrowLabel_, SIGNAL(pressed()), this, SIGNAL(arrowPressed()));
	connect(lk_ArrowLabel_, SIGNAL(released()), this, SIGNAL(arrowReleased()));
	
	resize(300, 100);
	emit resized();
}