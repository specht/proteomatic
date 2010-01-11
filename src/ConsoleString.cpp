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

#include "ConsoleString.h"


k_ConsoleString::k_ConsoleString()
    : ms_Output("")
    , ms_CurrentLine("")
    , mi_CurrentLineIndex(0)
{
}


k_ConsoleString::~k_ConsoleString()
{
}


void k_ConsoleString::clear()
{
    ms_Output = "";
    ms_CurrentLine = "";
    mi_CurrentLineIndex = 0;
}


QString k_ConsoleString::text() const
{
    return ms_Output + ms_CurrentLine;
}


void k_ConsoleString::append(QString as_Text)
{
    for (int i = 0; i < as_Text.length(); ++i)
    {
        if (as_Text.at(i) == QChar('\n'))
        {
            ms_Output += ms_CurrentLine + '\n';
            ms_CurrentLine = "";
            mi_CurrentLineIndex = 0;
        } else if (as_Text.at(i) == QChar('\r'))
        {
            mi_CurrentLineIndex = 0;
        } else
        {
            if (mi_CurrentLineIndex < ms_CurrentLine.length())
            {
                // replace char
                ms_CurrentLine.replace(mi_CurrentLineIndex, 1, as_Text.at(i));
                ++mi_CurrentLineIndex;
            }
            else 
            {
                // add char
                ms_CurrentLine += as_Text.at(i);
                ++mi_CurrentLineIndex;
            }
        }
    }
    emit changed();
}
