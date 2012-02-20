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

#include <QtGui>
#include <QtNetwork>
#include "FiletrackerBrowser.h"
#include "qjson/parser.h"


k_FiletrackerBrowser::k_FiletrackerBrowser(QWidget* ak_Parent_, 
                                           k_Proteomatic& ak_Proteomatic)
    : QWidget(ak_Parent_)
    , mk_Proteomatic(ak_Proteomatic)
    , ms_CouchUri("http://localhost:5984/filetracker/")
    , mk_NetworkAccessManager_(new QNetworkAccessManager(this))
{
    connect(mk_NetworkAccessManager_, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(replyFinished(QNetworkReply*)));
    this->initialize();
}


k_FiletrackerBrowser::~k_FiletrackerBrowser()
{
}


void k_FiletrackerBrowser::replyFinished(QNetworkReply* ak_Reply_)
{
    if (ak_Reply_->isFinished() && ak_Reply_->error() == QNetworkReply::NoError)
    {
        QJson::Parser lk_Parser;
        bool lb_Ok = false;
        QVariant lk_Response = lk_Parser.parse(ak_Reply_, &lb_Ok);
        if (lb_Ok)
        {
            QString ls_Intent = ak_Reply_->property("intent").toString();
            if (ls_Intent.startsWith("fill_column_"))
            {
                QString ls_Key = ls_Intent.replace("fill_column_", "");
                foreach (QVariant lk_Item, lk_Response.toMap()["rows"].toList())
                {
                    QMap<QString, QVariant> lk_Map = lk_Item.toMap();
                    QString ls_Label = lk_Map["key"].toString();
                    if (ls_Key == "time")
                    {
                        QList<QVariant> lk_List = lk_Map["key"].toList();
                        ls_Label = lk_List[0].toString() + "/" + lk_List[1].toString();
                    }
                    ls_Label = ls_Label.trimmed();
                    mk_PropertyListWidgets[ls_Key]->addItem(new QListWidgetItem(ls_Label + " (" + lk_Map["value"].toString() + ")"));
                }
            } 
            else if (ls_Intent == "fill_all_columns")
            {
                mk_SelectedRunsWidget_->setRowCount(lk_Response.toMap()["total_rows"].toInt());
                foreach (QVariant lk_Item, lk_Response.toMap()["rows"].toList())
                {
                    QMap<QString, QVariant> lk_Map = lk_Item.toMap();
                    QList<QVariant> lk_Values = lk_Map["value"].toList();
                    QString ls_Id = lk_Map["id"].toString();
                    mk_AllIds.insert(ls_Id);
                    QString ls_User = lk_Values[0].toString().trimmed();
                    QString ls_Host = lk_Values[1].toString().trimmed();
                    QString ls_ScriptTitle = lk_Values[2].toString().trimmed();
                    QString ls_Time = lk_Values[3].toString() + "/" + 
                        lk_Values[4].toString() + "/" + 
                        lk_Values[5].toString() + " " + 
                        lk_Values[6].toString() + ":" + 
                        lk_Values[7].toString() + ":" + 
                        lk_Values[8].toString();
                    QString ls_Time_YM = lk_Values[3].toString() + "/" + 
                        lk_Values[4].toString();
                    r_RunCoreInfo lr_RunCoreInfo;
                    lr_RunCoreInfo.ms_User = ls_User;
                    lr_RunCoreInfo.ms_Host = ls_Host;
                    lr_RunCoreInfo.ms_ScriptTitle = ls_ScriptTitle;
                    lr_RunCoreInfo.ms_Time = ls_Time;
                    mk_CoreInfoHash[ls_Id] = lr_RunCoreInfo;
                    QHash<QString, QString> lk_Items;
                    lk_Items["user"] = ls_User;
                    lk_Items["host"] = ls_Host;
                    lk_Items["script_title"] = ls_ScriptTitle;
                    lk_Items["time"] = ls_Time_YM;
                    foreach (QString ls_Key, lk_Items.keys())
                    {
                        QString ls_Value = lk_Items[ls_Key];
                        if (!mk_RunsByPropertyMaps[ls_Key].contains(ls_Value))
                            mk_RunsByPropertyMaps[ls_Key][ls_Value] = QSet<QString>();
                        mk_RunsByPropertyMaps[ls_Key][ls_Value].insert(ls_Id);
                        if (!mk_ListWidgetsByPropertyMaps[ls_Key].contains(ls_Value))
                        {
                            mk_ListWidgetsByPropertyMaps[ls_Key][ls_Value] = new QListWidgetItem(ls_Value);
                            mk_ListWidgetsByPropertyMaps[ls_Key][ls_Value]->setData(Qt::UserRole, QVariant(ls_Value));
                            mk_PropertyListWidgets[ls_Key]->addItem(mk_ListWidgetsByPropertyMaps[ls_Key][ls_Value]);
                        }
                        mk_ListWidgetsByPropertyMaps[ls_Key][ls_Value]->setText(ls_Value + QString(" (%1)").arg(mk_RunsByPropertyMaps[ls_Key][ls_Value].size()));
                    }
                }
            }
        }
    }
    ak_Reply_->deleteLater();
    updateRunSelection();
}


void k_FiletrackerBrowser::initialize()
{
    this->setWindowTitle("Filetracker Browser");
    this->resize(900, 600);
    QBoxLayout* lk_MainLayout_ = new QVBoxLayout(this);
    QSplitter* lk_Splitter_ = new QSplitter(this);
    lk_MainLayout_->addWidget(lk_Splitter_);
    lk_Splitter_->setOrientation(Qt::Vertical);
    QWidget* lk_Widget1_ = new QWidget(this);
    QBoxLayout* lk_Layout1_ = new QHBoxLayout(lk_Widget1_);
    lk_Layout1_->setContentsMargins(0, 0, 0, 0);
    
    QMap<QString, QString> lk_Items;
    lk_Items["01 time"] = "Time";
    lk_Items["02 script_title"] = "Script title";
    lk_Items["03 user"] = "User";
    lk_Items["04 host"] = "Host";
    
    foreach (QString ls_ItemKeyPre, lk_Items.keys())
    {
        QString ls_ItemKey = ls_ItemKeyPre.mid(3);
        QString ls_Item = lk_Items[ls_ItemKeyPre];
        QWidget* lk_Widget2_ = new QWidget(this);
        QBoxLayout* lk_Layout2_ = new QVBoxLayout(lk_Widget2_);
        lk_Layout2_->setContentsMargins(0, 0, 0, 0);
        lk_Layout2_->addWidget(new QLabel("<b>" + ls_Item + "</b>", this));
        mk_PropertyListWidgets[ls_ItemKey] = new QListWidget(this);
        mk_PropertyListWidgets[ls_ItemKey]->setSelectionMode(QAbstractItemView::ExtendedSelection);
        mk_PropertyListWidgets[ls_ItemKey]->setSortingEnabled(true);
        mk_PropertyListWidgets[ls_ItemKey]->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
        lk_Layout2_->addWidget(mk_PropertyListWidgets[ls_ItemKey]);
        lk_Layout1_->addWidget(lk_Widget2_);
        connect(mk_PropertyListWidgets[ls_ItemKey], SIGNAL(itemSelectionChanged()),
               this, SLOT(updateRunSelection()));
    }
    
    lk_Splitter_->addWidget(lk_Widget1_);
    mk_SelectedRunsWidget_ = new QTableWidget(this);
    mk_SelectedRunsWidget_->setColumnCount(4);
    mk_SelectedRunsWidget_->setRowCount(0);
    mk_SelectedRunsWidget_->setHorizontalHeaderLabels(QStringList() <<
        "Time" << "Script title" << "User" << "Host");
//     mk_SelectedRunsWidget_->horizontalHeader()->setStretchLastSection(true);
//     mk_SelectedRunsWidget_->horizontalHeader()->setShowSortIndicator(true);
    mk_SelectedRunsWidget_->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    mk_SelectedRunsWidget_->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    mk_SelectedRunsWidget_->setSelectionBehavior(QAbstractItemView::SelectRows);
    mk_SelectedRunsWidget_->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    lk_Splitter_->addWidget(mk_SelectedRunsWidget_);
}


void k_FiletrackerBrowser::refresh()
{
    qDebug() << "refreshing filetracker browser...";
    QNetworkReply* lk_Reply_;
    lk_Reply_ = mk_NetworkAccessManager_->get(QNetworkRequest(QUrl(ms_CouchUri + "_design/views/_view/all_scripts_core_info")));
    lk_Reply_->setProperty("intent", QVariant("fill_all_columns"));
}


void k_FiletrackerBrowser::updateRunSelection()
{
    QSet<QString> lk_Ids = mk_AllIds;
    foreach (QString ls_Key, mk_PropertyListWidgets.keys())
    {
        QSet<QString> lk_SubIds;
        if (!mk_PropertyListWidgets[ls_Key]->selectedItems().empty())
        {
            foreach (QListWidgetItem* lk_Item_, mk_PropertyListWidgets[ls_Key]->selectedItems())
            {
                QString ls_Value = lk_Item_->data(Qt::UserRole).toString();
                lk_SubIds |= mk_RunsByPropertyMaps[ls_Key][ls_Value];
            }
            lk_Ids &= lk_SubIds;
        }
    }
    mk_SelectedRunsWidget_->setRowCount(lk_Ids.size());
    int li_Row = 0;
    foreach (QString ls_Id, lk_Ids)
    {
        r_RunCoreInfo lr_Info = mk_CoreInfoHash[ls_Id];
        mk_SelectedRunsWidget_->setItem(li_Row, 0, new QTableWidgetItem(lr_Info.ms_Time));
        mk_SelectedRunsWidget_->item(li_Row, 0)->setData(Qt::UserRole, QVariant(ls_Id));
        mk_SelectedRunsWidget_->setItem(li_Row, 1, new QTableWidgetItem(lr_Info.ms_ScriptTitle));
        mk_SelectedRunsWidget_->setItem(li_Row, 2, new QTableWidgetItem(lr_Info.ms_User));
        mk_SelectedRunsWidget_->setItem(li_Row, 3, new QTableWidgetItem(lr_Info.ms_Host));
        for (int i = 0; i < 4; ++i)
            mk_SelectedRunsWidget_->item(li_Row, i)->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        ++li_Row;
    }
}
