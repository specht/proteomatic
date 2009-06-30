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

#include "InputGroupProxyBox.h"
#include "UnclickableLabel.h"


k_InputGroupProxyBox::k_InputGroupProxyBox(k_Desktop* ak_Parent_, k_Proteomatic& ak_Proteomatic,
											QString as_Label, QString as_GroupKey)
	: k_DesktopBox(ak_Parent_, ak_Proteomatic, false, false)
	, ms_Label(as_Label)
	, ms_GroupKey(as_GroupKey)
{
	connect(this, SIGNAL(boxConnected(IDesktopBox*, bool)), this, SLOT(boxConnectedSlot(IDesktopBox*, bool)));
	connect(this, SIGNAL(boxDisconnected(IDesktopBox*, bool)), this, SLOT(boxDisconnectedSlot(IDesktopBox*, bool)));
	setProtectedFromUserDeletion(true);
	setupLayout();
}


k_InputGroupProxyBox::~k_InputGroupProxyBox()
{
}


const QString& k_InputGroupProxyBox::groupKey() const
{
	return ms_GroupKey;
}


QStringList k_InputGroupProxyBox::filenames() const
{
	QStringList lk_Files;
	foreach (IDesktopBox* lk_Box_, incomingBoxes())
	{
		IFileBox* lk_FileBox_ = dynamic_cast<IFileBox*>(lk_Box_);
		if (lk_FileBox_)
			lk_Files += lk_FileBox_->filenames();
	}
	return lk_Files;
}


QString k_InputGroupProxyBox::tagForFilename(const QString& as_Filename) const
{
	//TODO: fix this
	return QString();
}


QString k_InputGroupProxyBox::prefixWithoutTags() const
{
	//TODO: fix this
	return QString();
}


void k_InputGroupProxyBox::boxConnectedSlot(IDesktopBox* ak_Other_, bool ab_Incoming)
{
	if (ab_Incoming)
	{
		IFileBox* lk_FileBox_ = dynamic_cast<IFileBox*>(ak_Other_);
		if (lk_FileBox_)
			connect(dynamic_cast<QObject*>(lk_FileBox_), SIGNAL(filenamesChanged()), this, SIGNAL(filenamesChanged()));
	}
}


void k_InputGroupProxyBox::boxDisconnectedSlot(IDesktopBox* ak_Other_, bool ab_Incoming)
{
	if (ab_Incoming)
	{
		IFileBox* lk_FileBox_ = dynamic_cast<IFileBox*>(ak_Other_);
		if (lk_FileBox_)
			disconnect(dynamic_cast<QObject*>(lk_FileBox_), SIGNAL(filenamesChanged()), this, SIGNAL(filenamesChanged()));
	}
}


void k_InputGroupProxyBox::setupLayout()
{
	QBoxLayout* lk_Layout_ = new QVBoxLayout(this);
	lk_Layout_->setContentsMargins(5, 5, 5, 5);

	QLabel* lk_Label_ = new k_UnclickableLabel(ms_Label, this);
	lk_Layout_->addWidget(lk_Label_);
}
