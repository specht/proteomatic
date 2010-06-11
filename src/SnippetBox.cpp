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

#include "SnippetBox.h"
#include "Proteomatic.h"
#include "Desktop.h"
#include "UnclickableLabel.h"


k_SnippetBox::k_SnippetBox(k_Desktop* ak_Parent_, k_Proteomatic& ak_Proteomatic, IScriptBox* ak_ScriptBoxParent_)
    : k_DesktopBox(ak_Parent_, ak_Proteomatic, true, true)
    , mk_ScriptBoxParent_(ak_ScriptBoxParent_)
    , mk_InactiveArrow(QPixmap(":icons/arrow-semi-semi-transparent.png").scaledToWidth(20, Qt::SmoothTransformation))
    , mk_ActiveArrow(QPixmap(":icons/arrow-semi-transparent.png").scaledToWidth(20, Qt::SmoothTransformation))
{
    mk_TextEdit.setAcceptRichText(false);
    setupLayout();
    // basename contains snippet box pointer and current date/time
    ms_Basename = QString("snippet-%2-%3")
        .arg((size_t)this, 0, 36)
        .arg(QDateTime::currentDateTime().toTime_t(), 0, 36);
    ms_CompleteBasename = QString("%1/%2")
        .arg(mk_Proteomatic.tempPath())
        .arg(ms_Basename);
}


k_SnippetBox::~k_SnippetBox()
{
    // delete temp files
    QDir lk_TempDir(mk_Proteomatic.tempPath());
    QStringList lk_TempFiles = lk_TempDir.entryList(QStringList() << ms_Basename + "*");
    foreach (QString ls_Path, lk_TempFiles)
        lk_TempDir.remove(ls_Path);
}


QStringList k_SnippetBox::filenames() const
{
    return QStringList() << ms_CompleteBasename + fileType();
}


QString k_SnippetBox::tagForFilename(const QString& /*as_Filename*/) const
{
    return QString();
}


QStringList k_SnippetBox::filenamesForTag(const QString& /*as_Tag*/) const
{
    return filenames();
}


QString k_SnippetBox::prefixWithoutTags() const
{
    return QString();
}


void k_SnippetBox::setListMode(bool /*ab_Enabled*/)
{
}


bool k_SnippetBox::listMode() const
{
    return false;
}


IScriptBox* k_SnippetBox::scriptBoxParent() const
{
    return mk_ScriptBoxParent_;
}


QString k_SnippetBox::text() const
{
    return mk_TextEdit.toPlainText();
}


QString k_SnippetBox::fileType() const
{
    return mk_FileTypeComboBox.itemData(mk_FileTypeComboBox.currentIndex()).toString();
}


void k_SnippetBox::flushFile()
{
    QFile lk_File(filenames().first());
    if (lk_File.open(QIODevice::WriteOnly))
    {
        QTextStream lk_Stream(&lk_File);
        lk_Stream << text();
        lk_Stream.flush();
        lk_File.close();
    }
}


void k_SnippetBox::setText(const QString& as_Text)
{
    mk_TextEdit.setPlainText(as_Text);
}


void k_SnippetBox::setFileType(const QString& as_FileType)
{
    int li_Index = mk_FileTypeComboBox.findData(as_FileType);
    if (li_Index >= 0)
        mk_FileTypeComboBox.setCurrentIndex(li_Index);
}


void k_SnippetBox::arrowPressedSlot()
{
    mk_ArrowLabel.setPixmap(mk_ActiveArrow);
}


void k_SnippetBox::arrowReleasedSlot()
{
    mk_ArrowLabel.setPixmap(mk_InactiveArrow);
}


void k_SnippetBox::setupLayout()
{
    QBoxLayout* lk_VLayout_;
    QBoxLayout* lk_HLayout_;
    
    connect(mk_Desktop_, SIGNAL(pipelineIdle(bool)), &mk_FileTypeComboBox, SLOT(setEnabled(bool)));
    connect(mk_Desktop_, SIGNAL(pipelineIdle(bool)), &mk_TextEdit, SLOT(setEnabled(bool)));

    lk_VLayout_ = new QVBoxLayout(this);
    lk_VLayout_->setContentsMargins(11, 11, 11, 11);
    
    lk_HLayout_ = new QHBoxLayout();
    lk_VLayout_->addLayout(lk_HLayout_);
    lk_HLayout_->addWidget(new k_UnclickableLabel("<b>Snippet</b>", this));
    lk_HLayout_->addStretch();
    lk_HLayout_->addWidget(&mk_FileTypeComboBox);
    
    lk_HLayout_ = new QHBoxLayout();
    lk_VLayout_->addLayout(lk_HLayout_);
    lk_HLayout_->addWidget(&mk_TextEdit);
    
    QBoxLayout* lk_VSubLayout_ = new QVBoxLayout();
//     lk_HLayout_->addStretch();
    lk_HLayout_->addLayout(lk_VSubLayout_);
    lk_VSubLayout_->addStretch();
    
    mk_ArrowLabel.setPixmap(mk_InactiveArrow);
    lk_VSubLayout_->addWidget(&mk_ArrowLabel);
    
    connect(&mk_ArrowLabel, SIGNAL(pressed()), this, SIGNAL(arrowPressed()));
    connect(&mk_ArrowLabel, SIGNAL(pressed()), this, SLOT(arrowPressedSlot()));
    connect(&mk_ArrowLabel, SIGNAL(released()), this, SIGNAL(arrowReleased()));
    connect(&mk_ArrowLabel, SIGNAL(released()), this, SLOT(arrowReleasedSlot()));
    
    QMap<QString, QPair<QString, QStringList> > lk_TextFileFormats = mk_Proteomatic.textFileFormats();
    foreach (QString ls_LowerDescription, lk_TextFileFormats.keys())
    {
        QString ls_Description = lk_TextFileFormats[ls_LowerDescription].first;
        QString ls_Label = QString("%1 (%2)")
            .arg(ls_Description)
            .arg(lk_TextFileFormats[ls_LowerDescription].second.join("|"));
        mk_FileTypeComboBox.addItem(ls_Label, lk_TextFileFormats[ls_LowerDescription].second.first());
    }
    setFileType(".txt");
    mk_TextEdit.setFont(mk_Proteomatic.consoleFont());
    mk_TextEdit.setWordWrapMode(QTextOption::NoWrap);
    
    emit resized();
}
