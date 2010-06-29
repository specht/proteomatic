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
    , ms_LastLine(QString())
    , mi_LastLineCursorPosition(0)
{
    this->initialize();
}


k_ConsoleTextEdit::k_ConsoleTextEdit(const QString& text, k_Proteomatic& ak_Proteomatic, QWidget* parent)
    : QTextEdit(text, parent)
    , mk_Proteomatic(ak_Proteomatic)
    , ms_LastLine(QString())
    , mi_LastLineCursorPosition(0)
{
    this->initialize();
}


k_ConsoleTextEdit::~k_ConsoleTextEdit()
{
}


void k_ConsoleTextEdit::append(const QString& as_Text)
{
    // reset cursor to the last position in case somebody moved it by clicking
    setTextCursor(mk_Cursor);
    /*
    PROBLEM: \r makes the cursor jump to beginning of the line, characters
    coming after that will be overwritten, but a \n will never overwrite 
    characters. Implementing this on a character-by-character basis is way
    too slow (30 seconds for 10,000 lines instead of 5).
    CONCEPT: Handle the stream line by line, like Ruby's each_line iterator.
    */
    int li_NewLineIndex = -1;
    while (true)
    {
        int li_Start = li_NewLineIndex + 1;
        li_NewLineIndex = as_Text.indexOf('\n', li_Start);
        int li_End = (li_NewLineIndex < 0) ? (as_Text.length() - 1) : li_NewLineIndex;
        
        if (li_End < li_Start)
            break;
        
        // as_Text[li_Start, li_End - li_Start + 1] is the current line,
        // possibly including the trailing \n, now handle this line
        
        QString ls_Line = as_Text.mid(li_Start, li_End - li_Start + 1);
        
        bool lb_LineHasNewLine = ls_Line.at(ls_Line.length() - 1) == '\n';
        if (lb_LineHasNewLine)
            ls_Line = ls_Line.left(ls_Line.length() - 1);
        
        // canonicalize the line: split at \r
        
        int li_CarriageReturnIndex = -1;
        while (true)
        {
            int li_SubStart = li_CarriageReturnIndex + 1;
            li_CarriageReturnIndex = ls_Line.indexOf('\r', li_SubStart);
            int li_SubEnd = (li_CarriageReturnIndex < 0) ? (ls_Line.length() - 1) : li_CarriageReturnIndex;
            
            if (li_SubEnd < li_SubStart)
                break;
            
            // ls_Line[li_SubStart, li_SubEnd - li_SubStart + 1] is the current item,
            // possibly including the trailing \r
            
            QString ls_SubLine = ls_Line.mid(li_SubStart, li_SubEnd - li_SubStart + 1);
            
            bool lb_LineHasCarriageReturn = ls_SubLine.at(ls_SubLine.length() - 1) == '\r';
            if (lb_LineHasCarriageReturn)
                ls_SubLine = ls_SubLine.left(ls_SubLine.length() - 1);
            
            // now consider ms_LastLine and mi_LastLineCursorPosition
            if (ls_SubLine.length() > 0)
            {
                int li_DeleteLength = qMin(ls_SubLine.length(), ms_LastLine.length() - mi_LastLineCursorPosition);
                if (li_DeleteLength > 0)
                    ms_LastLine.remove(mi_LastLineCursorPosition, li_DeleteLength);
                ms_LastLine.insert(mi_LastLineCursorPosition, ls_SubLine);
                mi_LastLineCursorPosition += ls_SubLine.length();
            }
            
            if (lb_LineHasCarriageReturn)
                mi_LastLineCursorPosition = 0;
            
            if ((li_CarriageReturnIndex < 0) || (li_SubEnd == ls_Line.length() - 1))
                break;
        }
        // replace the entire last line of the text edit with ms_LastLine
        QTextCursor lk_Cursor = textCursor();
        lk_Cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
        lk_Cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        lk_Cursor.removeSelectedText();
        lk_Cursor.insertText(ms_LastLine);
        if (lb_LineHasNewLine)
        {
            lk_Cursor.insertText("\n");
            ms_LastLine.clear();
            mi_LastLineCursorPosition = 0;
        }
        if ((li_NewLineIndex < 0) || (li_End == as_Text.length() - 1))
            break;
    }
    ensureCursorVisible();
    mk_Cursor = textCursor();
}


void k_ConsoleTextEdit::initialize()
{
    this->setReadOnly(true);
    this->setFont(mk_Proteomatic.consoleFont());
    this->setAcceptRichText(false);
    document()->setMaximumBlockCount(1000);
    mk_Cursor = textCursor();
}
