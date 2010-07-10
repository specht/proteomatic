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

#include "RubyWindow.h"
#include <QtGui>


k_RubyWindow::k_RubyWindow(k_Proteomatic& ak_Proteomatic, QStringList ak_Arguments, QString as_Title, QString as_IconPath)
    : QObject(NULL)
    , mk_Proteomatic(ak_Proteomatic)
    , mk_Arguments(ak_Arguments)
    , ms_Title(as_Title)
{
    mk_pDialog = QSharedPointer<QDialog>(new QDialog(mk_Proteomatic.messageBoxParent()));
    mk_pDialog->setWindowIcon(QIcon(as_IconPath));
    mk_pDialog->setWindowTitle(as_Title);
    mk_pDialog->resize(512, 250);
    QBoxLayout* lk_VLayout_ = new QVBoxLayout(mk_pDialog.data());
    QBoxLayout* lk_HLayout_ = new QHBoxLayout(NULL);
    mk_Output_ = new k_ConsoleTextEdit(mk_Proteomatic, mk_pDialog.data());
    lk_VLayout_->addWidget(mk_Output_);
    lk_VLayout_->addLayout(lk_HLayout_);
    lk_HLayout_->addStretch();
    mk_AbortButton_ = new QPushButton(QIcon(":/icons/dialog-cancel.png"), "Abort", mk_pDialog.data());
    lk_HLayout_->addWidget(mk_AbortButton_);
    mk_CloseButton_ = new QPushButton(QIcon(":/icons/dialog-ok.png"), "Close", mk_pDialog.data());
    lk_HLayout_->addWidget(mk_CloseButton_);
    mk_pDialog->setLayout(lk_VLayout_);
    mk_pDialog->setModal(true);
    mk_Output_->setReadOnly(true);
    
    mk_Output_->setFont(mk_Proteomatic.consoleFont());
    connect(mk_CloseButton_, SIGNAL(clicked()), mk_pDialog.data(), SLOT(accept()));
}


k_RubyWindow::~k_RubyWindow()
{
}

    
bool k_RubyWindow::exec()
{
    mk_AbortButton_->setEnabled(false);
    mk_CloseButton_->setEnabled(false);
    mk_Output_->clear();
    
    QSharedPointer<QProcess> lk_pProcess(new QProcess());
    connect(mk_AbortButton_, SIGNAL(clicked()), lk_pProcess.data(), SLOT(kill()));
    connect(lk_pProcess.data(), SIGNAL(started()), this, SLOT(processStarted()));
    connect(lk_pProcess.data(), SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(processFinished(int, QProcess::ExitStatus)));
    connect(lk_pProcess.data(), SIGNAL(readyReadStandardError()), this, SLOT(processReadyRead()));
    connect(lk_pProcess.data(), SIGNAL(readyReadStandardOutput()), this, SLOT(processReadyRead()));
    
    mb_ScriptFinishedFine = false;
    
    QFileInfo lk_FileInfo(mk_Arguments.first());
    if (lk_FileInfo.exists())
    {
        lk_pProcess->setWorkingDirectory(lk_FileInfo.absolutePath());
        QString ls_Script = QFileInfo(mk_Arguments.takeFirst()).fileName();
        mk_Arguments.insert(0, ls_Script);
    }
    lk_pProcess->setProcessChannelMode(QProcess::MergedChannels);
    lk_pProcess->start(mk_Proteomatic.scriptInterpreter("ruby"), mk_Arguments, QIODevice::ReadOnly | QIODevice::Unbuffered);

    mk_pDialog->exec();
    return mb_ScriptFinishedFine;
}


void k_RubyWindow::processStarted()
{
    addOutput(ms_Title + ":\n");
    processReadyRead();
    mk_AbortButton_->setEnabled(true);
}


void k_RubyWindow::processFinished(int ai_ExitCode, QProcess::ExitStatus ak_ExitStatus)
{
    processReadyRead();
    addOutput("-----------------------------------\n");
    if (ai_ExitCode != 0)
        addOutput(QString("Process failed with exit code %1\n").arg(ai_ExitCode));
    else
    {
        if (ak_ExitStatus == QProcess::NormalExit)
            mb_ScriptFinishedFine = true;
    }

    mk_AbortButton_->setEnabled(false);
    mk_CloseButton_->setEnabled(true);
}


void k_RubyWindow::processReadyRead()
{
    QProcess* lk_Process_ = dynamic_cast<QProcess*>(sender());
    if (lk_Process_ != NULL)
        addOutput(lk_Process_->readAll());
}


void k_RubyWindow::addOutput(QString as_Text)
{
    mk_Output_->append(as_Text);
}
