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

#include "SearchMenu.h"
#include "HintLineEdit.h"
#include "Proteomatic.h"
#include "PipelineMainWindow.h"


k_SearchMenu::k_SearchMenu(k_Proteomatic& ak_Proteomatic, QWidget* parent)
    : QMenu(parent)
    , mk_Proteomatic(ak_Proteomatic)
{
    this->initialize();
}


k_SearchMenu::k_SearchMenu(k_Proteomatic& ak_Proteomatic, const QString& title, QWidget* parent)
    : QMenu(title, parent)
    , mk_Proteomatic(ak_Proteomatic)
{
    this->initialize();
}


k_SearchMenu::~k_SearchMenu()
{
//     mk_pSearchWidgetAction = QSharedPointer<QWidgetAction>(NULL);
}


void k_SearchMenu::addSearchField()
{
    mk_pSearchWidgetAction = QSharedPointer<QWidgetAction>(new QWidgetAction(NULL));
    mk_pHintLineEdit = QSharedPointer<k_HintLineEdit>(new k_HintLineEdit());
    mk_pHintLineEdit->setHint("Enter search term");
    mk_pSearchWidgetAction->setDefaultWidget(mk_pHintLineEdit.data());
    connect(mk_pHintLineEdit.data(), SIGNAL(textEdited(const QString&)), this, SLOT(searchFieldPopup(const QString&)), Qt::QueuedConnection);
    connect(this, SIGNAL(addNewSearchResultsSignal(const QString&)), this, SLOT(addNewSearchResults(const QString&)), Qt::QueuedConnection);
    connect(this, SIGNAL(clearOldSearchResultsSignal()), this, SLOT(clearOldSearchResults()), Qt::QueuedConnection);
    
    this->addSeparator();
    this->addAction(mk_pSearchWidgetAction.data());
}


void k_SearchMenu::searchFieldPopup(const QString& as_String)
{
    mk_DeleteTheseActions.append(mk_pSearchResultActions);
    mk_pSearchResultActions.clear();
    emit addNewSearchResultsSignal(as_String);
    emit clearOldSearchResultsSignal();
}

void k_SearchMenu::addNewSearchResults(const QString& as_String)
{
    QApplication::processEvents();
    QString ls_String = as_String.toLower();
    QStringList lk_Tags = ls_String.split(mk_WordSplitter, QString::SkipEmptyParts);
    QHash<QString, QStringList>& lk_Keywords = mk_Proteomatic.mk_ScriptKeywords;
    QHash<QString, int> lk_Targets;
    foreach (QString ls_Tag, lk_Tags)
    {
        foreach (QString ls_Keyword, lk_Keywords.keys())
        {
            if (ls_Keyword.startsWith(ls_Tag))
            {
                foreach (QString ls_Target, lk_Keywords[ls_Keyword])
                {
                    if (ls_Target.startsWith("script/"))
                    {
                        ls_Target.replace("script/", "");
                        QString ls_Class = ls_Target.left(ls_Target.indexOf("/"));
                        int li_ClassScore = 0;
                        if (ls_Class == "title")
                            li_ClassScore = 100;
                        else if (ls_Class == "group")
                            li_ClassScore = 10;
                        else if (ls_Class == "description")
                            li_ClassScore = 1;
                        ls_Target.replace(ls_Class + "/", "");
                        if (!lk_Targets.contains(ls_Target))
                            lk_Targets[ls_Target] = 0;
                        lk_Targets[ls_Target] += li_ClassScore;
                    }
                }
            }
        }
    }
    QMultiMap<int, QString> lk_TargetsSorted;
    foreach (QString ls_Target, lk_Targets.keys())
        lk_TargetsSorted.insert(lk_Targets[ls_Target], ls_Target);
    
    if (!lk_TargetsSorted.empty())
    {
        QMultiMap<int, QString>::const_iterator lk_Iter = lk_TargetsSorted.constEnd();
        int li_Count = 0;
        do
        {
            --lk_Iter;
            
            QString ls_ScriptPath = lk_Iter.value();
            QString ls_Title = mk_Proteomatic.scriptInfo(ls_ScriptPath)["title"].toString();
            if (!ls_Title.isEmpty())
            {
                QAction* lk_Action_ = new QAction(QIcon(":icons/proteomatic.png"), ls_Title, NULL);
                lk_Action_->setData(ls_ScriptPath);
                mk_pSearchResultActions.push_back(QSharedPointer<QAction>(lk_Action_));
                this->addAction(lk_Action_);
                ++li_Count;
            }
        } while ((lk_Iter != lk_TargetsSorted.constBegin()) && (li_Count < 10));
    }
}


void k_SearchMenu::clearOldSearchResults()
{
    QApplication::processEvents();
    mk_DeleteTheseActions.clear();
}


void k_SearchMenu::initialize()
{
    mk_WordSplitter = QRegExp("\\W+");
}


void k_SearchMenu::showEvent(QShowEvent* event)
{
    if (mk_pHintLineEdit.data())
        mk_pHintLineEdit->clear();
    
    mk_pSearchResultActions.clear();
    mk_DeleteTheseActions.clear();
    
    QMenu::showEvent(event);
    if (mk_pHintLineEdit.data())
    {
        mk_pHintLineEdit->setFocus();
        setActiveAction(mk_pSearchWidgetAction.data());
    }
}


void k_SearchMenu::hideEvent(QHideEvent* event)
{
    QMenu::hideEvent(event);
}


void k_SearchMenu::keyPressEvent(QKeyEvent* event)
{
    QMenu::keyPressEvent(event);
    QApplication::processEvents();
}
