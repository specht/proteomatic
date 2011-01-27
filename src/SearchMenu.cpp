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

#define MAX_SEARCH_RESULTS 10


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
    mk_pHintLineEdit->setHintVisibleWhenFocused(true);
    mk_pSearchWidgetAction->setDefaultWidget(mk_pHintLineEdit.data());
    connect(mk_pHintLineEdit.data(), SIGNAL(textChanged(const QString&)), this, SLOT(addNewSearchResults(const QString&))/*, Qt::QueuedConnection*/);
    connect(this, SIGNAL(aboutToShow()), this, SLOT(aboutToShowSlot()));
    
    this->addSeparator();
    this->addAction(mk_pSearchWidgetAction.data());
    this->setDefaultAction(NULL);
    for (int i = 0; i < MAX_SEARCH_RESULTS; ++i)
    {
        QAction* lk_Action_ = new QAction("", this);
        lk_Action_->setText(QString());
        this->addAction(lk_Action_);
        mk_SearchResults << lk_Action_;
    }
}


void k_SearchMenu::setInputFilenames(QStringList ak_Paths)
{
    mk_InputFilenames = ak_Paths;
    mk_AllInputSuffixes = QSet<QString>();
    // always add the 'any' file suffix
    if (!ak_Paths.empty())
        mk_AllInputSuffixes << "";
    foreach (QString ls_Path, ak_Paths)
    {
        QString ls_Suffix = QFileInfo(ls_Path).completeSuffix().toLower();
        QStringList lk_SuffixList = ls_Suffix.split(".");
        for (int i = 0; i < lk_SuffixList.size(); ++i)
        {
            QString ls_SubSuffix = QStringList(lk_SuffixList.mid(lk_SuffixList.size() - 1 - i, i + 1)).join(".");
            mk_AllInputSuffixes << "." + ls_SubSuffix;
        }
    }
}


void k_SearchMenu::addNewSearchResults(const QString& as_String)
{
    QApplication::processEvents();
    setUpdatesEnabled(false);
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
    QMultiMap<QString, QString> lk_TargetsSorted;
    foreach (QString ls_Target, lk_Targets.keys())
        lk_TargetsSorted.insert(QString("%1-%2").arg(lk_Targets[ls_Target], 16, 10, QChar('0')).arg(ls_Target), ls_Target);
    int li_Count = 0;
    if (!lk_TargetsSorted.empty())
    {
        QMultiMap<QString, QString>::const_iterator lk_Iter = lk_TargetsSorted.constEnd();
        do
        {
            --lk_Iter;
            
            QString ls_ScriptPath = lk_Iter.value();
            QString ls_Title;
            ls_Title = mk_Proteomatic.scriptInfo(ls_ScriptPath)["title"].toString();
            if (!ls_Title.isEmpty())
            {
                if (mk_SearchResults[li_Count]->text() != ls_Title)
                {
                    QTextDocument doc;
                    doc.setHtml(mk_Proteomatic.scriptInfo(ls_ScriptPath)["description"].toString());
                    doc.setHtml(doc.toPlainText());
                    mk_SearchResults[li_Count]->setStatusTip(doc.toPlainText());
                    mk_SearchResults[li_Count]->setText(ls_Title);
                    QApplication::processEvents();
                }
                if (!(mk_AllInputSuffixes.empty() || (!(mk_AllInputSuffixes & mk_Proteomatic.mk_ExtensionsForScriptPath[ls_ScriptPath]).empty())))
                    mk_SearchResults[li_Count]->setIcon(QIcon(":icons/proteomatic-disabled.png"));
                else
                    mk_SearchResults[li_Count]->setIcon(QIcon(":icons/proteomatic.png"));
                mk_SearchResults[li_Count]->setData(ls_ScriptPath);
                mk_SearchResults[li_Count]->setVisible(true);
                QApplication::processEvents();
                ++li_Count;
            }
        } while ((lk_Iter != lk_TargetsSorted.constBegin()) && (li_Count < MAX_SEARCH_RESULTS));
    }
    while (li_Count < MAX_SEARCH_RESULTS)
    {
        mk_SearchResults[li_Count]->setVisible(false);
        QApplication::processEvents();
        ++li_Count;
    }
    
    setUpdatesEnabled(true);
    QApplication::processEvents();
}


void k_SearchMenu::aboutToShowSlot()
{
    if (mk_pHintLineEdit.data())
        mk_pHintLineEdit->clear();
    
    addNewSearchResults(QString());
    QApplication::processEvents();

    if (mk_pHintLineEdit.data())
    {
        mk_pHintLineEdit->setFocus();
        setActiveAction(mk_pSearchWidgetAction.data());
    }
}


void k_SearchMenu::initialize()
{
    mk_WordSplitter = QRegExp("\\W+");
}
