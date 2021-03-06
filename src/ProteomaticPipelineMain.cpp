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
#include <QtGui>
#include "Proteomatic.h"
#include "PipelineMainWindow.h"


int main(int ai_ArgumentCount, char** ac_Arguments__)
{
    Q_INIT_RESOURCE(Proteomatic);
    QApplication lk_App(ai_ArgumentCount, ac_Arguments__);
    QDir::setCurrent(lk_App.applicationDirPath());
    
    k_Proteomatic lk_Proteomatic(lk_App);
    
    k_PipelineMainWindow lk_MainWindow(NULL, lk_Proteomatic, lk_App);
    lk_Proteomatic.setPipelineMainWindow(&lk_MainWindow);
    lk_Proteomatic.initialize();
    lk_MainWindow.initialize();
    lk_MainWindow.show();

    // check for updates on startup
    if (lk_Proteomatic.getConfiguration(CONFIG_AUTO_CHECK_FOR_UPDATES).toBool())
    {
        lk_MainWindow.setEnabled(false);
        lk_Proteomatic.checkForUpdates();
    }
    
    if (ai_ArgumentCount > 1)
    {
        QString ls_Path = ac_Arguments__[1];
        if (ls_Path.endsWith(FILE_EXTENSION_PIPELINE))
            lk_MainWindow.loadPipeline(ls_Path);
    }

    return lk_App.exec();
}
