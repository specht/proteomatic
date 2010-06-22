/*
Copyright (c) 2010 Michael Specht

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

#include "ConsoleTextEdit.h"


k_ConsoleTextEdit::k_ConsoleTextEdit(k_Proteomatic& ak_Proteomatic, QWidget* parent)
    : QTextEdit(parent)
    , mk_Proteomatic(ak_Proteomatic)
{
    this->initialize();
}


k_ConsoleTextEdit::k_ConsoleTextEdit(const QString& text, k_Proteomatic& ak_Proteomatic, QWidget* parent)
    : QTextEdit(text, parent)
    , mk_Proteomatic(ak_Proteomatic)
{
    this->initialize();
}


k_ConsoleTextEdit::~k_ConsoleTextEdit()
{
}


void k_ConsoleTextEdit::append(const QString& as_Text)
{
    QString ls_Text = as_Text;
    
    // process the first and last line the slow way, but canonicalize all lines in between
    int li_Offset = 0;
    int li_NewLineIndex;
    bool lb_FirstLine = true;
    QString ls_CanonicalBlock;
    while ((li_NewLineIndex = ls_Text.indexOf('\n', li_Offset)) >= 0)
    {
        // we're not yet at the last line, but maybe it's the first
        if (lb_FirstLine)
            appendTheSlowWay(ls_Text.mid(li_Offset, li_NewLineIndex - li_Offset + 1));
        else
        {
            QString ls_CanonicalLine;
            QString ls_Line = ls_Text.mid(li_Offset, li_NewLineIndex - li_Offset);
            // ls_Line is a whole line without the trailing \n
            // now canonicalize the line and insert it plus a \n
            int li_CarriageReturnIndex = 0;
            int li_End = ls_Line.length();
            while (true)
            {
                li_CarriageReturnIndex = ls_Line.lastIndexOf('\r', li_CarriageReturnIndex - 1);
                
                if (li_CarriageReturnIndex < 0)
                    break;
                
                QString ls_Bit = ls_Line.mid(li_CarriageReturnIndex + 1, li_End - li_CarriageReturnIndex - 1);
                li_End = li_CarriageReturnIndex;
                ls_Bit = ls_Bit.mid(ls_CanonicalLine.length());
                ls_CanonicalLine += ls_Bit;
                
                if (li_CarriageReturnIndex < 1)
                    break;
            }
            QString ls_Bit = ls_Line.mid(li_CarriageReturnIndex + 1, li_End - li_CarriageReturnIndex - 1);
            ls_Bit = ls_Bit.mid(ls_CanonicalLine.length());
            ls_CanonicalLine += ls_Bit;
            
            ls_CanonicalLine += '\n';
            ls_CanonicalBlock += ls_CanonicalLine;
        }
        lb_FirstLine = false;
        li_Offset = li_NewLineIndex + 1;
    }
    textCursor().insertText(ls_CanonicalBlock);
    appendTheSlowWay(ls_Text.mid(li_Offset));
    
    ensureCursorVisible();
}


void k_ConsoleTextEdit::initialize()
{
    this->setReadOnly(true);
    this->setFont(mk_Proteomatic.consoleFont());
    this->setAcceptRichText(false);
    document()->setMaximumBlockCount(1000);
}


void k_ConsoleTextEdit::appendTheSlowWay(const QString& as_Text)
{
    // now insert text char by char :TODO: speed this up, but seems complicated
    for (int i = 0; i < as_Text.length(); ++i)
    {
        QChar lc_Char = as_Text[i];
        if (lc_Char == '\r')
        {
            moveCursor(QTextCursor::StartOfLine);
        }
        else if (lc_Char == '\n')
        {
            moveCursor(QTextCursor::EndOfLine);
            textCursor().insertText(lc_Char);
        }
        else
        {
            if (!textCursor().atEnd())
                textCursor().deleteChar();
            textCursor().insertText(lc_Char);
        }
    }
}
