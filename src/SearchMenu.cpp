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
    mk_pSearchWidgetAction = QSharedPointer<QWidgetAction>(NULL);
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
    mk_pSearchResultWidgetAction->setDefaultWidget(mk_pSearchResultList.data());
    mk_pSearchResultList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mk_pSearchResultList->setMaximumHeight(100);
    this->addAction(mk_pSearchResultWidgetAction.data());
    
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
    
    //mk_pSearchPopup = QSharedPointer<QListWidget>(new QListWidget(this));
    //mk_pSearchPopup->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
    
    if (lk_TargetsSorted.empty())
    {
        mk_pSearchResultList->hide();
        return;
    }
    
    mk_pSearchResultList->clear();
    mk_pSearchResultList->show();
    QMultiMap<int, QString>::const_iterator lk_Iter = lk_TargetsSorted.constEnd();
    do
    {
        --lk_Iter;
        
        QString ls_ScriptPath = lk_Iter.value();
        QString ls_Title = mk_Proteomatic.scriptInfo(ls_ScriptPath)["title"].toString();
        if (!ls_Title.isEmpty())
            new QListWidgetItem(QIcon(":src/icons/proteomatic.png"), ls_Title, mk_pSearchResultList.data());
    } while (lk_Iter != lk_TargetsSorted.constBegin());
    
    //mk_pSearchPopup->setWindowModality(Qt::NonModal);
/*    QWidget* lk_Sender_ = dynamic_cast<QWidget*>(sender());
    if (lk_Sender_)
    {
        mk_pSearchPopup->move(this->pos() + QPoint(0, this->height()));
        mk_pSearchPopup->resize(250, 100);
        //mk_pSearchPopup->setIconSize(QSize(16, 16));
        mk_pSearchPopup->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        mk_pSearchPopup->show();
        //connect(lk_Sender_, SIGNAL(focusOut()), mk_pSearchPopup.data(), SLOT(hide()));
        //lk_Sender_->setFocus(Qt::MouseFocusReason);
    }*/
}


void k_SearchMenu::initialize()
{
    mk_WordSplitter = QRegExp("\\W+");
}


void k_SearchMenu::showEvent(QShowEvent* event)
{
    if (mk_pHintLineEdit.data())
        mk_pHintLineEdit->clear();
    
    QMenu::showEvent(event);
/*    if (mk_pHintLineEdit.data())
        mk_pHintLineEdit->setFocus(Qt::OtherFocusReason);*/

    if (mk_pSearchResultList.data())
        mk_pSearchResultList->hide();
}


void k_SearchMenu::hideEvent(QHideEvent* event)
{
/*    if ((mk_pSearchPopup.data()) && (mk_pSearchPopup->isVisible()))
        mk_pSearchPopup->hide();*/
    QMenu::hideEvent(event);
}


void k_SearchMenu::keyPressEvent(QKeyEvent* event)
{
/*    if ((mk_pHintLineEdit.data()) && (mk_pHintLineEdit->hasFocus()) && (event->key() == Qt::Key_Down))
    {
        mk_pSearchPopup->setCurrentRow(0);
        mk_pSearchPopup->setFocus(Qt::OtherFocusReason);
        event->accept();
    }
    else*/
        QMenu::keyPressEvent(event);
}
