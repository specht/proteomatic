#include <QtCore>
#include "ConsoleString.h"
#include "Proteomatic.h"
#include "RefPtr.h"


class k_RubyWindow: public QObject
{
	Q_OBJECT
public:
	k_RubyWindow(k_Proteomatic& ak_Proteomatic, QStringList ak_Arguments, QString as_Title = "Ruby script", QString ms_IconPath = ":/icons/proteomatic.png");
	~k_RubyWindow();
		
	void exec();
	
protected slots:
	void processStarted();
	void processFinished(int ai_ExitCode, QProcess::ExitStatus ak_ExitStatus);
	void processReadyRead();

protected:
	k_Proteomatic& mk_Proteomatic;
	QStringList mk_Arguments;
	QString ms_Title;
	
	RefPtr<QDialog> mk_pDialog;
	QTextEdit* mk_Output_;
	QPushButton* mk_AbortButton_;
	QPushButton* mk_CloseButton_;
	k_ConsoleString mk_Output;
	
	void addOutput(QString as_Text);
};
