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
    
    // preprocess \r where possible
    // the thing is: if we do this cursor thing too often, it
    // will take 30 seconds to print 10000 lines of "Hello\rThis is a test.\n"
    // but the terminal can do it in 5 seconds, which is what we want 
    // observation: the output comes in big chunks, and if we remove
    // \r where we can, the overall output should be much faster

    /*
    "Hello!\rThis is a test.\nHello!\rThis is a test.\n"
    becomes:
    "This is a test.\nThis is a test.\n"
    */
    
    int li_Position = -1;
    int li_Offset = 0;
    while ((li_Position = ls_Text.indexOf("\r", li_Offset)) >= 0)
    {
        int li_PreviousNewLine = ls_Text.lastIndexOf("\n", li_Position - 1);
        if (li_PreviousNewLine >= 0)
        {
            // if we see the previous newline, we can resolve this \r !!!
            int li_NextNewLine = ls_Text.indexOf("\n", li_Position + 1);
            if (li_NextNewLine < 0)
                li_NextNewLine = ls_Text.length();
            int li_LeftLength = li_Position - li_PreviousNewLine;
            int li_RightLength = li_NextNewLine - li_Position;
            int li_DeleteLength = qMin(li_LeftLength, li_RightLength);
            // remove text before \r and also the \r itself
            ls_Text.remove(li_PreviousNewLine + 1, li_DeleteLength);
        }
        li_Offset = li_Position + 1;
    }
    
    // now print the text
    
    li_Position = -1;
    li_Offset = 0;
    while ((li_Position = ls_Text.indexOf("\r", li_Offset)) >= 0)
    {
        // insert text before \r
        int li_Length = li_Position - li_Offset;
        // insert text in overwrite mode
        QString ls_Bit = ls_Text.mid(li_Offset, li_Length);
        int li_DeleteLength = qMin(document()->characterCount() - textCursor().position(), ls_Bit.length());
        for (int i = 0; i < li_DeleteLength; ++i)
            textCursor().deleteChar();
        textCursor().insertText(ls_Bit);
        // move to start of line because of \r
        moveCursor(QTextCursor::StartOfLine);
        li_Offset = li_Position + 1;
    }
    // insert remaining text
    // insert text in overwrite mode
    QString ls_Bit = ls_Text.mid(li_Offset);
    int li_DeleteLength = qMin(document()->characterCount() - textCursor().position(), ls_Bit.length());
    for (int i = 0; i < li_DeleteLength; ++i)
        textCursor().deleteChar();
    textCursor().insertText(ls_Bit);
    ensureCursorVisible();
}


void k_ConsoleTextEdit::initialize()
{
    this->setReadOnly(true);
    this->setFont(mk_Proteomatic.consoleFont());
    this->setAcceptRichText(false);
    document()->setMaximumBlockCount(1000);
}
