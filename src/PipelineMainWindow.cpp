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

#include <QtGui>
#include "PipelineMainWindow.h"
#include "version.h"


k_PipelineMainWindow::k_PipelineMainWindow(QWidget* ak_Parent_,
                                           QApplication& ak_Application)
    : QMainWindow(ak_Parent_)
    , mk_Application(ak_Application)
{
    resize(800, 600);
    setWindowTitle("Proteomatic mini!");
    setWindowIcon(QIcon(":icons/proteomatic-pipeline.png"));
    QString ls_Path = QDir::cleanPath(QString("%2/%1").arg(PROTEOMATIC_BASE_DIR).arg(mk_Application.applicationDirPath()));
    QString ls_Message = QString("base dir is %1").arg(ls_Path);
    foreach (QString ls_Path, QDir(ls_Path).entryList(QStringList() << "*"))
    {
        ls_Message += "\n" + ls_Path;
    }
    
    QVBoxLayout* lk_Layout_ = new QVBoxLayout(this);
    QLabel* lk_Label_ = new QLabel(ls_Message, this);
    lk_Label_->resize(800, 600);
    lk_Label_->setWordWrap(true);
    lk_Layout_->addWidget(lk_Label_);
    show();
}


k_PipelineMainWindow::~k_PipelineMainWindow()
{
}
