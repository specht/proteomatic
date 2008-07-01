#pragma once

#include <QtCore>

#ifdef _WIN32
#include <windows.h>
#endif


class k_StopWatch
{
public:
	k_StopWatch();
	k_StopWatch(QString as_Message, QTextStream* ak_OutputStream_ = NULL);
	virtual ~k_StopWatch();

	static QString getTimeAsString(double ad_Time);

	QString getTimeAsString();
	double get_Time();
	void reset();
	void setExitMessage(QString as_Message);

private:

	double get_AbsoluteTime();

#ifdef _WIN32
	LARGE_INTEGER ml_Frequency;
#endif

	double md_StartTime;
	QString ms_Message;
	QTextStream mk_StdOutputStream;
	QTextStream* mk_OutputStream_;
	bool mb_Print;
};
