/*
Copyright (c) 2012 Michael Specht

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

#include <QtGui>
#include <QtNetwork>
#include "Proteomatic.h"

typedef struct
{
    QString ms_User;
    QString ms_ScriptTitle;
    QString ms_Time;
    QString ms_Host;
} r_RunCoreInfo;


typedef QHash<QString, QSet<QString> > tk_RunsByPropertyMap;
typedef QHash<QString, QListWidgetItem*> tk_ListWidgetItemsByPropertyMap;


class k_FiletrackerBrowser: public QWidget
{
    Q_OBJECT
public:
    k_FiletrackerBrowser(QWidget* ak_Parent_, k_Proteomatic& ak_Proteomatic);
    virtual ~k_FiletrackerBrowser();
    
    void initialize();
    
public slots:
    void refresh();
    
protected slots:
    void replyFinished(QNetworkReply* ak_Reply_);
    void updateRunSelection();
    void selectedRunTableSelectionChanged(int ai_Row, int, int, int);
    
protected:
    const k_Proteomatic& mk_Proteomatic;
    const QString ms_CouchUri;
    QNetworkAccessManager* mk_NetworkAccessManager_;
    
    QMap<QString, QListWidget*> mk_PropertyListWidgets;
    QTableWidget* mk_SelectedRunsWidget_;
    QMap<QString, r_RunCoreInfo> mk_CoreInfoHash;
    QHash<QString, tk_RunsByPropertyMap> mk_RunsByPropertyMaps;
    QHash<QString, tk_ListWidgetItemsByPropertyMap> mk_ListWidgetsByPropertyMaps;
    QSet<QString> mk_AllIds;
    QWidget* mk_RightPane_;
    QBoxLayout* mk_RightPaneLayout_;
    QTextEdit* mk_RightPaneScriptLabel_;
};
