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
#include "DesktopBox.h"
#include "FileList.h"


class k_Proteomatic;

class k_OutFileListBox: public k_DesktopBox, public IFileBox
{
	Q_OBJECT
public:
	k_OutFileListBox(k_Desktop* ak_Parent_, k_Proteomatic& ak_Proteomatic, QString as_Label);
	virtual ~k_OutFileListBox();

	virtual QStringList filenames() const;
	virtual QString tagForFilename(const QString& as_Filename) const;
	virtual QString prefixWithoutTags() const;
	
	virtual void setFilenames(QStringList ak_Filenames);
	virtual void setListMode(bool ab_Enabled);
	virtual bool listMode() const;
	virtual QString label() const;
	
signals:
	virtual void arrowPressed();
	virtual void arrowReleased();
	virtual void filenamesChanged();
	
protected slots:
	virtual void setBatchMode(bool ab_Enabled);
	virtual void toggleUi();
	virtual void updateFilenameTags();
	virtual void linkActivated(const QString& as_Url);
	
protected:
	virtual void setupLayout();

	QString ms_Label;
	k_FileList mk_FileList;
	QLabel* mk_Label_;
	QLabel* mk_FileName_;
	QToolButton mk_BatchModeButton;
	bool mb_ListMode;
	QHash<QString, QString> mk_TagForFilename;
	QString ms_PrefixWithoutTags;
};