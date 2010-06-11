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

#pragma once
#include <QtCore>
#include <QtGui>
#include <QWebView>


class k_Preview: public QWidget
{
    Q_OBJECT
    
public:
    k_Preview(const QString& as_Url, QWidget* parent = 0, Qt::WindowFlags f = 0);
    virtual ~k_Preview();
    
protected:
    void setupLayout();
    
    QString ms_Url;
    QWebView mk_WebView;
};
