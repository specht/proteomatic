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
