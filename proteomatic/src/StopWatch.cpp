#include "StopWatch.h"


#ifndef _WIN32
#include <sys/time.h>
#endif


k_StopWatch::k_StopWatch()
	: ms_Message(QString())
	, mk_StdOutputStream(stdout)
	, mk_OutputStream_(&mk_StdOutputStream)
	, mb_Print(false)
{
#ifdef _WIN32
	::QueryPerformanceFrequency(&ml_Frequency);
#endif
	this->reset();
}


k_StopWatch::k_StopWatch(QString as_Message, QTextStream* ak_OutputStream_)
	: ms_Message(as_Message)
	, mk_StdOutputStream(stdout)
	, mk_OutputStream_(ak_OutputStream_ != NULL? ak_OutputStream_: &mk_StdOutputStream)
	, mb_Print(true)
{
#ifdef _WIN32
	::QueryPerformanceFrequency(&ml_Frequency);
#endif
	this->reset();
}


k_StopWatch::~k_StopWatch()
{
	if (mb_Print)
	{
		QString ls_Duration = getTimeAsString();
		*mk_OutputStream_ << ms_Message.arg(ls_Duration);
	}
}


QString k_StopWatch::getTimeAsString(double ad_Time)
{
	QString ls_Duration;

	if (ad_Time < 1.0)
		ls_Duration = QString("%1 milliseconds").arg(ad_Time * 1000.0, 0, 'f', 1);
	else if (ad_Time < 60.0)
		ls_Duration += QString("%1 seconds").arg(ad_Time, 0, 'f', 1);
	else
	{
		int li_Time = (int)ad_Time;
		int li_Minutes = li_Time / 60;
		li_Time -= li_Minutes * 60;
		ls_Duration += QString("%1 minutes and %2 seconds").arg(li_Minutes).arg(li_Time);
	}

	return ls_Duration;
}


QString k_StopWatch::getTimeAsString()
{
	return getTimeAsString(this->get_Time());
}


double k_StopWatch::get_Time()
{
	return get_AbsoluteTime() - md_StartTime;
}


void k_StopWatch::reset()
{
	md_StartTime = this->get_AbsoluteTime();
}


void k_StopWatch::setExitMessage(QString as_Message)
{
	ms_Message = as_Message;
}


double k_StopWatch::get_AbsoluteTime()
{
#ifdef _WIN32
	LARGE_INTEGER ll_Time;
	::QueryPerformanceCounter(&ll_Time);
	return (double)ll_Time.QuadPart / (double)ml_Frequency.QuadPart;
#else
	struct timeval lr_Time;
	struct timezone lr_TimeZone;
	gettimeofday(&lr_Time, &lr_TimeZone);
	return (double)lr_Time.tv_sec + (double)lr_Time.tv_usec / 1000000.0;
#endif
}


