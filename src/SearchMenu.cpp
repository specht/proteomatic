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
    connect(mk_pHintLineEdit.data(), SIGNAL(textEdited(const QString&)), this, SLOT(searchFieldPopup(const QString&)));
    
    this->addSeparator();
    this->addAction(mk_pSearchWidgetAction.data());
    
    mk_pSearchResultWidgetAction = QSharedPointer<QWidgetAction>(new QWidgetAction(NULL));
    mk_pSearchResultList = QSharedPointer<QListWidget>(new QListWidget());
    connect(mk_pSearchResultList.data(), SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(itemClickedSlot(QListWidgetItem*)));
    connect(mk_pSearchResultList.data(), SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(itemClickedSlot(QListWidgetItem*)));
    mk_pSearchResultWidgetAction->setDefaultWidget(mk_pSearchResultList.data());
    mk_pSearchResultList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mk_pSearchResultList->setMaximumHeight(100);
    mk_pSearchResultList->setSelectionMode(QAbstractItemView::NoSelection);
    
    mk_pSearchResultList->hide();
}


void k_SearchMenu::searchFieldPopup(const QString& as_String)
{
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
    
    if (lk_TargetsSorted.empty())
    {
        if (mb_ResultListInserted)
        {
            this->removeAction(mk_pSearchResultWidgetAction.data());
            mb_ResultListInserted = false;
        }
        return;
    }
    
    mk_pSearchResultList->clear();
    QMultiMap<int, QString>::const_iterator lk_Iter = lk_TargetsSorted.constEnd();
    do
    {
        --lk_Iter;
        
        QString ls_ScriptPath = lk_Iter.value();
        QString ls_Title = mk_Proteomatic.scriptInfo(ls_ScriptPath)["title"].toString();
        if (!ls_Title.isEmpty())
        {
            QListWidgetItem* lk_Item_ = new QListWidgetItem(QIcon(":src/icons/proteomatic.png"), ls_Title, mk_pSearchResultList.data());
            lk_Item_->setData(Qt::UserRole, ls_ScriptPath);

        }
    } while (lk_Iter != lk_TargetsSorted.constBegin());
    if (!mb_ResultListInserted)
    {
        this->addAction(mk_pSearchResultWidgetAction.data());
        mb_ResultListInserted = true;
    }
}


void k_SearchMenu::itemClickedSlot(QListWidgetItem* ak_Item_)
{
    if (mi_GotTicket >= mi_Ticket)
        return;
    mi_GotTicket = mi_Ticket;
    QString ls_Uri = ak_Item_->data(Qt::UserRole).toString();
    mk_Proteomatic.pipelineMainWindow()->addScript(ls_Uri);
}


void k_SearchMenu::initialize()
{
    mk_WordSplitter = QRegExp("\\W+");
    mb_ResultListInserted = false;
    mi_Ticket = 0;
    mi_GotTicket = 0;
}


void k_SearchMenu::showEvent(QShowEvent* event)
{
    if (mk_pHintLineEdit.data())
        mk_pHintLineEdit->clear();
    
    if (mk_pSearchResultList.data())
        mk_pSearchResultList->clear();
    
    if (mb_ResultListInserted)
    {
        this->removeAction(mk_pSearchResultWidgetAction.data());
        mb_ResultListInserted = false;
    }
    
    mi_Ticket += 1;
    
    QMenu::showEvent(event);
    if (mk_pHintLineEdit.data())
        mk_pHintLineEdit->clearFocus();
    setActiveAction(NULL);
}


void k_SearchMenu::hideEvent(QHideEvent* event)
{
    QMenu::hideEvent(event);
}


void k_SearchMenu::keyPressEvent(QKeyEvent* event)
{
    QMenu::keyPressEvent(event);
}
