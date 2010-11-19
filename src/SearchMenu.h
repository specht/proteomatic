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

class k_Proteomatic;
class k_HintLineEdit;


class k_SearchMenu: public QMenu
{
    Q_OBJECT
public:
    k_SearchMenu(k_Proteomatic& ak_Proteomatic, QWidget* parent = 0);
    k_SearchMenu(k_Proteomatic& ak_Proteomatic, const QString& title, QWidget* parent = 0);
    virtual ~k_SearchMenu();
    
    void addSearchField();
    
protected slots:
    void searchFieldPopup(const QString& as_String);
    
protected:
    void initialize();
    void showEvent(QShowEvent* event);
    void hideEvent(QHideEvent* event);
    
    k_Proteomatic& mk_Proteomatic;
    QSharedPointer<QWidgetAction> mk_pSearchWidgetAction;
    QSharedPointer<k_HintLineEdit> mk_pHintLineEdit;
    QRegExp mk_WordSplitter;
    QList<QSharedPointer<QAction> > mk_pSearchResultActions;
};
