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
#include "Tango.h"
#include "UnclickableLabel.h"


k_OutFileListBox::k_OutFileListBox(k_Desktop* ak_Parent_, k_Proteomatic& ak_Proteomatic,
									QString as_Label, QString as_Filename)
	: k_DesktopBox(ak_Parent_, ak_Proteomatic, true)
	, ms_Label(as_Label)
	, ms_Filename(as_Filename)
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


void k_OutFileListBox::setFilenames(QStringList ak_Filenames)
{
	mk_FileList.resetAll();
	foreach (QString ls_Path, ak_Filenames)
		mk_FileList.addInputFile(ls_Path);
	if (ak_Filenames.empty())
		ms_Filename = "";
	else
		ms_Filename = ak_Filenames.first();
	toggleUi();
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


QString k_OutFileListBox::filename() const
{
	return ms_Filename;
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
	mk_FileList.setVisible(mb_ListMode);
	mk_BatchModeButton.setVisible(mb_ListMode);
	if (mb_ListMode)
		mk_Label_->setText("<b>" + ms_Label + " files</b>");
	else
		mk_Label_->setText(ms_Filename);
}


void k_OutFileListBox::setupLayout()
{
	QBoxLayout* lk_VLayout_;
	QBoxLayout* lk_HLayout_;
	
	lk_HLayout_ = new QHBoxLayout(this);
	
	lk_VLayout_ = new QVBoxLayout();
	lk_HLayout_->addLayout(lk_VLayout_);
	mk_Label_ = new k_UnclickableLabel(ms_Filename, this);
	lk_VLayout_->addWidget(mk_Label_);
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
