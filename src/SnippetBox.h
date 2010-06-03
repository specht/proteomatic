/*
Copyright (c) 2010 Michael Specht

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
#include "ClickableLabel.h"


class k_Proteomatic;

class k_SnippetBox: public k_DesktopBox, public IFileBox
{
    Q_OBJECT
public:
    k_SnippetBox(k_Desktop* ak_Parent_, k_Proteomatic& ak_Proteomatic, IScriptBox* ak_ScriptBoxParent_);
    virtual ~k_SnippetBox();
    
    virtual QStringList filenames() const;
    virtual QString tagForFilename(const QString& as_Filename) const;
    virtual QStringList filenamesForTag(const QString& as_Tag) const;
    virtual QString prefixWithoutTags() const;
    virtual void setListMode(bool ab_Enabled);
    virtual bool listMode() const;
    virtual IScriptBox* scriptBoxParent() const;
    virtual QString text() const;
    virtual QString fileType() const;
    virtual void flushFile();
    
public slots:
    virtual void setText(const QString& as_Text);
    virtual void setFileType(const QString& as_FileType);
    
protected slots:
    virtual void arrowPressedSlot();
    virtual void arrowReleasedSlot();

signals:
    void arrowPressed();
    void arrowReleased();
    void filenamesChanged();
    
protected:
    virtual void setupLayout();

    IScriptBox* mk_ScriptBoxParent_;
    QComboBox mk_FileTypeComboBox;
    QTextEdit mk_TextEdit;
    QPixmap mk_InactiveArrow;
    QPixmap mk_ActiveArrow;
    k_ClickableLabel mk_ArrowLabel;
    QString ms_Basename;
    QString ms_CompleteBasename;
};
