#include "RubyWindow.h"
#include <QtGui>


k_RubyWindow::k_RubyWindow(k_Proteomatic& ak_Proteomatic, QStringList ak_Arguments, QString as_Title, QString as_IconPath)
	: QObject(NULL)
	, mk_Proteomatic(ak_Proteomatic)
	, mk_Arguments(ak_Arguments)
	, ms_Title(as_Title)
{
	mk_pDialog = RefPtr<QDialog>(new QDialog(mk_Proteomatic.messageBoxParent()));
	mk_pDialog->setWindowIcon(QIcon(as_IconPath));
	mk_pDialog->setWindowTitle(as_Title);
	mk_pDialog->resize(512, 250);
	QBoxLayout* lk_VLayout_ = new QVBoxLayout(mk_pDialog.get_Pointer());
	QBoxLayout* lk_HLayout_ = new QHBoxLayout(mk_pDialog.get_Pointer());
	mk_Output_ = new QTextEdit(mk_pDialog.get_Pointer());
	lk_VLayout_->addWidget(mk_Output_);
	lk_VLayout_->addLayout(lk_HLayout_);
	lk_HLayout_->addStretch();
	mk_AbortButton_ = new QPushButton(QIcon(":/icons/dialog-cancel.png"), "Abort", mk_pDialog.get_Pointer());
	lk_HLayout_->addWidget(mk_AbortButton_);
	mk_CloseButton_ = new QPushButton(QIcon(":/icons/dialog-ok.png"), "Close", mk_pDialog.get_Pointer());
	lk_HLayout_->addWidget(mk_CloseButton_);
	mk_pDialog->setLayout(lk_VLayout_);
	mk_pDialog->setModal(true);
	mk_Output_->setReadOnly(true);
	
	QFontDatabase lk_FontDatabase;
	QStringList lk_Fonts = QStringList() << "Consolas" << "Bitstream Vera Sans Mono" << "Lucida Console" << "Courier New" << "Courier";
	while (!lk_Fonts.empty())
	{
		QString ls_Font = lk_Fonts.takeFirst();
		if (lk_FontDatabase.families().contains(ls_Font))
		{
			mk_Output_->setFont(QFont(ls_Font, 8));
			break;
		}
	}
	connect(mk_CloseButton_, SIGNAL(clicked()), mk_pDialog.get_Pointer(), SLOT(accept()));
}


k_RubyWindow::~k_RubyWindow()
{
}

	
void k_RubyWindow::exec()
{
	mk_AbortButton_->setEnabled(false);
	mk_CloseButton_->setEnabled(false);
	mk_Output_->clear();
	
	RefPtr<QProcess> lk_pProcess(new QProcess());
	connect(mk_AbortButton_, SIGNAL(clicked()), lk_pProcess.get_Pointer(), SLOT(kill()));
	connect(lk_pProcess.get_Pointer(), SIGNAL(started()), this, SLOT(processStarted()));
	connect(lk_pProcess.get_Pointer(), SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(processFinished(int, QProcess::ExitStatus)));
	connect(lk_pProcess.get_Pointer(), SIGNAL(readyReadStandardError()), this, SLOT(processReadyRead()));
	connect(lk_pProcess.get_Pointer(), SIGNAL(readyReadStandardOutput()), this, SLOT(processReadyRead()));
	
	QFileInfo lk_FileInfo(mk_Arguments.first());
	if (lk_FileInfo.exists())
		lk_pProcess->setWorkingDirectory(lk_FileInfo.absolutePath());
	lk_pProcess->setProcessChannelMode(QProcess::MergedChannels);
	lk_pProcess->start(mk_Proteomatic.getConfiguration(CONFIG_PATH_TO_RUBY).toString(), mk_Arguments, QIODevice::ReadOnly | QIODevice::Unbuffered);

	mk_pDialog->exec();
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
	mk_Output.append(as_Text);
	mk_Output_->setText(mk_Output.text());
	mk_Output_->moveCursor(QTextCursor::End);
	mk_Output_->ensureCursorVisible();
}
