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

#pragma once

#include <QtCore>

#ifdef __win32__
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

#ifdef __win32__
    LARGE_INTEGER ml_Frequency;
#endif

    double md_StartTime;
    QString ms_Message;
    QTextStream mk_StdOutputStream;
    QTextStream* mk_OutputStream_;
    bool mb_Print;
};
