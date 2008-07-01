#pragma once

#include <QtCore>


class k_ConsoleString: public QObject
{
	Q_OBJECT
public:
	k_ConsoleString();
	virtual ~k_ConsoleString();
	
	void clear();
	QString text() const;
	void append(QString as_Text);
	
signals:
	void changed();
	
protected:
	QString ms_Output;
	QString ms_CurrentLine;
	int mi_CurrentLineIndex;
};
