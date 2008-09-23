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

#include "TicketWindow.h"


k_TicketWindow::k_TicketWindow(k_Proteomatic& ak_Proteomatic, QString as_ScriptUri, QString as_Ticket)
	: QWidget(ak_Proteomatic.messageBoxParent(), Qt::Window)
	, mk_Proteomatic(ak_Proteomatic)
	, ms_ScriptUri(as_ScriptUri)
	, ms_Ticket(as_Ticket)
	, mi_Delay(1)
	, mb_GotStandardOutput(false)
{
	setWindowIcon(QIcon(":/icons/proteomatic.png"));
	setWindowTitle(QString("Check ticket: %1").arg(ms_Ticket));
	mk_StateIcon_ = new QLabel(this);
	mk_StateIcon_->setPixmap(QPixmap(":/icons/appointment.png"));
	QBoxLayout* lk_Layout_ = new QVBoxLayout(this);
	QBoxLayout* lk_SubLayout_ = new QHBoxLayout(this);
	QBoxLayout* lk_SubSubLayout_ = new QVBoxLayout(this);
	lk_SubLayout_->addWidget(mk_StateIcon_);
	lk_SubSubLayout_->addWidget(new QLabel(QString("Ticket: <b>%1</b>").arg(ms_Ticket), this));
	lk_SubSubLayout_->addWidget(new QLabel(QString("Remote script: %1").arg(mk_Proteomatic.scriptInfo(ms_ScriptUri, "title")), this));
	lk_SubLayout_->addLayout(lk_SubSubLayout_);
	lk_SubLayout_->addStretch();
	lk_Layout_->addLayout(lk_SubLayout_);
	mk_StateLabel_ = new QLabel(this);
	lk_Layout_->addWidget(mk_StateLabel_);
	
	mk_Output_ = new QTextEdit(this);
	mk_Output_->setReadOnly(true);
	mk_Output_->setFont(mk_Proteomatic.consoleFont());
	lk_Layout_->addWidget(mk_Output_);
	
	// output directory
	lk_SubLayout_ = new QHBoxLayout(this);
	mk_OutputDirectoryLabel_ = new QLabel("Output directory:", this);
	mk_OutputDirectory_ = new QLineEdit();
	mk_OutputDirectory_->setReadOnly(true);
	mk_OutputDirectoryLabel_->setBuddy(mk_OutputDirectory_);
	lk_SubLayout_->addWidget(mk_OutputDirectoryLabel_);
	lk_SubLayout_->addWidget(mk_OutputDirectory_);
	mk_ChooseOutputDirectory_ = new QToolButton();
	mk_ChooseOutputDirectory_->setIcon(QIcon(":/icons/folder.png"));
	lk_SubLayout_->addWidget(mk_ChooseOutputDirectory_);
	lk_Layout_->addLayout(lk_SubLayout_);
	
	// output file prefix
	lk_SubLayout_ = new QHBoxLayout(this);
	mk_PrefixLabel_ = new QLabel("Output file prefix:", this);
	mk_Prefix_ = new QLineEdit();
	mk_PrefixLabel_->setBuddy(mk_Prefix_);
	lk_SubLayout_->addWidget(mk_PrefixLabel_);
	lk_SubLayout_->addWidget(mk_Prefix_);
	lk_Layout_->addLayout(lk_SubLayout_);

	lk_SubLayout_ = new QHBoxLayout(this);
	lk_SubLayout_->addStretch();
	mk_SaveButton_ = new QPushButton(QIcon(":/icons/document-save.png"), "Save files", this);
	connect(mk_SaveButton_, SIGNAL(clicked()), this, SLOT(saveOutputFiles()));
	QPushButton* lk_CloseButton_ = new QPushButton(QIcon(":/icons/dialog-ok.png"), "Close", this);
	connect(lk_CloseButton_, SIGNAL(clicked()), this, SLOT(close()));
	lk_SubLayout_->addWidget(mk_SaveButton_);
	lk_SubLayout_->addWidget(lk_CloseButton_);
	lk_Layout_->addLayout(lk_SubLayout_);
	setLayout(lk_Layout_);
	connect(&mk_Proteomatic, SIGNAL(remoteHubRequestFinished(int, bool, QString)), this, SLOT(remoteRequestFinished(int, bool, QString)));
	queryTicket();
	
	mk_OutputDirectoryLabel_->setEnabled(false);
	mk_OutputDirectory_->setEnabled(false);
	mk_ChooseOutputDirectory_->setEnabled(false);
	mk_PrefixLabel_->setEnabled(false);
	mk_Prefix_->setEnabled(false);
	mk_SaveButton_->setEnabled(false);
	
	resize(600, 260);
	show();
};


k_TicketWindow::~k_TicketWindow()
{
}


void k_TicketWindow::closeEvent(QCloseEvent* ak_Event_)
{
	emit closed();
}


void k_TicketWindow::queryTicket()
{
	mk_RemoteRequests[mk_Proteomatic.queryRemoteHub(ms_ScriptUri, QStringList() << "---gui" << "--query" << ms_Ticket)] = r_RemoteRequest(r_RemoteRequestType::QueryTicket);
}


void k_TicketWindow::remoteRequestFinished(int ai_SocketId, bool ab_Error, QString as_Result)
{
	if (mk_RemoteRequests.contains(ai_SocketId))
	{
		r_RemoteRequest lr_RemoteRequest = mk_RemoteRequests[ai_SocketId];
		mk_RemoteRequests.remove(ai_SocketId);
		
		if (lr_RemoteRequest.me_Type == r_RemoteRequestType::QueryTicket)
		{
			bool lb_Error = true;
			QStringList lk_Response = as_Result.split("\n");
			if (!lk_Response.empty() && lk_Response.takeFirst().trimmed() == "---queryTicket")
			{
				QString ls_State = lk_Response.takeFirst().trimmed();
				if (ls_State == "waiting")
				{
					QString ls_InFront = lk_Response.takeFirst().trimmed();
					mk_StateIcon_->setPixmap(QPixmap(":/icons/appointment.png"));
					QString ls_StateLabel = QString("Your job is waiting to be processed.\nThere %1 in front of it.").arg(ls_InFront == "1"? "is 1 job": "are " + ls_InFront + " jobs");
					mk_StateLabel_->setText(ls_StateLabel);
					lb_Error = false;
				}
				else if (ls_State == "running")
				{
					mk_StateIcon_->setPixmap(QPixmap(":/icons/applications-system.png"));
					mk_StateLabel_->setText(QString("Your job is currently being processed."));
					lb_Error = false;
				}
				else if (ls_State == "finished")
				{
					mk_StateIcon_->setPixmap(QPixmap(":/icons/dialog-ok.png"));
					mk_StateLabel_->setText(QString("Your job has been finished."));
					lb_Error = false;
					if (!mb_GotStandardOutput)
					{
						QString ls_Directory = "";
						QString ls_Prefix = "";
						if (!lk_Response.empty())
							ls_Directory = lk_Response.takeFirst().trimmed();
						if (!lk_Response.empty())
							ls_Prefix = lk_Response.takeFirst().trimmed();
						while (!lk_Response.empty())
						{
							QString ls_Filename = lk_Response.takeFirst().trimmed();
							if (!ls_Filename.isEmpty())
								mk_OutputFiles << ls_Filename;
						}
							
						if (!mk_OutputFiles.empty())
						{
							mk_OutputDirectory_->setText(ls_Directory);
							mk_Prefix_->setText(ls_Prefix);
							
							mk_OutputDirectoryLabel_->setEnabled(true);
							mk_OutputDirectory_->setEnabled(true);
							mk_ChooseOutputDirectory_->setEnabled(true);
							mk_PrefixLabel_->setEnabled(true);
							mk_Prefix_->setEnabled(true);
							mk_SaveButton_->setEnabled(true);
						}
							
						mb_GotStandardOutput = true;
						mk_RemoteRequests[mk_Proteomatic.queryRemoteHub(ms_ScriptUri, QStringList() << "---gui" << "--getStandardOutput" << ms_Ticket)] = 
							r_RemoteRequest(r_RemoteRequestType::GetStandardOutput);
					}
				}
				
				if (mi_Delay < 10)
					mi_Delay += 1;
				if (mi_Delay < 20)
					mi_Delay += 2;
					
				QTimer::singleShot(mi_Delay * 1000, this, SLOT(queryTicket()));
			}
			if (lb_Error)
				mk_StateIcon_->setPixmap(QPixmap(":/icons/dialog-warning.png"));
		}
		else if (lr_RemoteRequest.me_Type == r_RemoteRequestType::GetStandardOutput)
		{
			mk_StandardOutput.append(as_Result);
			mk_Output_->setText(mk_StandardOutput.text());
		}
		else if (lr_RemoteRequest.me_Type == r_RemoteRequestType::GetOutputFiles)
		{
			QString ls_Icon = ":/icons/document-save.png";
			if (as_Result.startsWith("error", Qt::CaseInsensitive))
				ls_Icon = ":/icons/dialog-warning.png";
			mk_Proteomatic.showMessageBox("Save output files", as_Result, ls_Icon);
		}
	}
}


void k_TicketWindow::saveOutputFiles()
{
	QStringList lk_Arguments;
	lk_Arguments << "---gui" << "--getOutputFiles" << ms_Ticket;
	if (!mk_OutputDirectory_->text().isEmpty())
		lk_Arguments << "--outputDirectory" << mk_OutputDirectory_->text();
	if (!mk_Prefix_->text().isEmpty())
		lk_Arguments << "--outputPrefix" << mk_Prefix_->text();
	mk_RemoteRequests[mk_Proteomatic.queryRemoteHub(ms_ScriptUri, lk_Arguments)] = r_RemoteRequest(r_RemoteRequestType::GetOutputFiles);
}
