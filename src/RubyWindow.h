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

#include <QtCore>
#include "ConsoleTextEdit.h"
#include "Proteomatic.h"


class k_RubyWindow: public QObject
{
    Q_OBJECT
public:
    k_RubyWindow(k_Proteomatic& ak_Proteomatic, QStringList ak_Arguments, QString as_Title = "Ruby script", QString ms_IconPath = ":/icons/proteomatic.png");
    ~k_RubyWindow();
        
    bool exec();
    
protected slots:
    void processStarted();
    void processFinished(int ai_ExitCode, QProcess::ExitStatus ak_ExitStatus);
    void processReadyRead();

protected:
    k_Proteomatic& mk_Proteomatic;
    QStringList mk_Arguments;
    QString ms_Title;
    
    QDialog* mk_Dialog_;
    k_ConsoleTextEdit* mk_Output_;
    QPushButton* mk_AbortButton_;
    QPushButton* mk_CloseButton_;
    bool mb_ScriptFinishedFine;
    
    void addOutput(QString as_Text);
};
