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

#include "CiListWidgetItem.h"
#include "FoldedHeader.h"
#include "ProfileManager.h"
#include "Script.h"
#include "dialogs/EditProfileDialog.h"


k_ProfileManager::k_ProfileManager(k_Proteomatic& ak_Proteomatic, 
                                   IScript* ak_CurrentScript_,
                                   QWidget * parent, Qt::WindowFlags f)
    : QDialog(parent, f)
    , mk_Proteomatic(ak_Proteomatic)
    , mk_CurrentScript_(ak_CurrentScript_)
    , ms_TargetScriptUri()
    , mk_TargetScriptParameterKeys()
    , mk_SelectedItem_(NULL)
{
    if (mk_CurrentScript_)
    {
        ms_TargetScriptUri = mk_CurrentScript_->uri();
        mk_TargetScriptParameterKeys = mk_CurrentScript_->parameterKeys();
    }
    
    setWindowTitle("Profile Manager");
    setWindowIcon(QIcon(":/icons/proteomatic.png"));

    setModal(true);
    
    QToolBar* lk_ToolBar_ = new QToolBar("Profile actions", this);
    lk_ToolBar_->setIconSize(QSize(24, 24));
    lk_ToolBar_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    mk_NewAction_ = lk_ToolBar_->addAction(QIcon(":/icons/list-add.png"), "&New");
    connect(mk_NewAction_, SIGNAL(triggered()), this, SLOT(newProfile()));
    mk_EditAction_ = lk_ToolBar_->addAction(QIcon(":/icons/preferences-system.png"), "&Edit");
    connect(mk_EditAction_, SIGNAL(triggered()), this, SLOT(editProfile()));
    mk_DeleteAction_ = lk_ToolBar_->addAction(QIcon(":/icons/dialog-cancel.png"), "&Delete");
    connect(mk_DeleteAction_, SIGNAL(triggered()), this, SLOT(deleteProfile()));
    lk_ToolBar_->addSeparator();
    mk_ImportAction_ = lk_ToolBar_->addAction(QIcon(":/icons/document-open.png"), "&Import");
    connect(mk_ImportAction_, SIGNAL(triggered()), this, SLOT(importProfile()));
    mk_ExportAction_ = lk_ToolBar_->addAction(QIcon(":/icons/document-save.png"), "E&xport");
    connect(mk_ExportAction_, SIGNAL(triggered()), this, SLOT(exportProfile()));
    
    QBoxLayout* lk_VLayout_ = new QVBoxLayout(this);
    lk_VLayout_->setContentsMargins(0, 0, 0, 0);
    lk_VLayout_->setSpacing(0);
    lk_VLayout_->addWidget(lk_ToolBar_);
    QFrame* lk_Frame_ = new QFrame(this);
    lk_Frame_->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    lk_VLayout_->addWidget(lk_Frame_);
    
    // Attention: we are overwriting the outer vertical layout here!
    QBoxLayout* lk_OuterLayout_ = lk_VLayout_;
    lk_VLayout_ = new QVBoxLayout();
    lk_VLayout_->setContentsMargins(8, 8, 8, 8);
    lk_OuterLayout_->addLayout(lk_VLayout_);
    
    QSplitter* lk_VSplitter_ = new QSplitter(this);
    lk_VSplitter_->setStyle(new QPlastiqueStyle());
    lk_VSplitter_->setOrientation(Qt::Horizontal);
    lk_VSplitter_->setHandleWidth(4);
    lk_VSplitter_->setChildrenCollapsible(false);
    lk_VLayout_->addWidget(lk_VSplitter_);
    
    QBoxLayout* lk_IVLayout_ = new QVBoxLayout();
    
    mk_ApplicableProfilesWidget_ = new QListWidget(this);
    mk_ApplicableProfilesWidget_->setSortingEnabled(true);
    mk_ApplicableProfilesWidget_->setSelectionMode(QAbstractItemView::SingleSelection);
    mk_PartlyApplicableProfilesWidget_ = new QListWidget(this);
    mk_PartlyApplicableProfilesWidget_->setSortingEnabled(true);
    mk_PartlyApplicableProfilesWidget_->setSelectionMode(QAbstractItemView::SingleSelection);
    mk_NonApplicableProfilesWidget_ = new QListWidget(this);
    mk_NonApplicableProfilesWidget_->setSortingEnabled(true);
    mk_NonApplicableProfilesWidget_->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(mk_ApplicableProfilesWidget_, SIGNAL(currentRowChanged(int)), this, SLOT(currentProfileChanged()));
    connect(mk_PartlyApplicableProfilesWidget_, SIGNAL(currentRowChanged(int)), this, SLOT(currentProfileChanged()));
    connect(mk_NonApplicableProfilesWidget_, SIGNAL(currentRowChanged(int)), this, SLOT(currentProfileChanged()));
    connect(mk_ApplicableProfilesWidget_, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(profileClicked(QListWidgetItem*)));
    connect(mk_PartlyApplicableProfilesWidget_, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(profileClicked(QListWidgetItem*)));
    connect(mk_NonApplicableProfilesWidget_, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(profileClicked(QListWidgetItem*)));
    connect(mk_ApplicableProfilesWidget_, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(profileClicked(QListWidgetItem*)));
    connect(mk_PartlyApplicableProfilesWidget_, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(profileClicked(QListWidgetItem*)));
    connect(mk_NonApplicableProfilesWidget_, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(profileClicked(QListWidgetItem*)));

    lk_IVLayout_->addWidget(new QLabel("Please check the profiles you want to apply:", this));
    
    mk_ApplicableProfilesHeader_ = new k_FoldedHeader("Applicable profiles", mk_ApplicableProfilesWidget_, this);
    mk_ApplicableProfilesHeader_->showBuddy();
    lk_IVLayout_->addWidget(mk_ApplicableProfilesHeader_);
    lk_IVLayout_->addWidget(mk_ApplicableProfilesWidget_);
    
    mk_PartlyApplicableProfilesHeader_ = new k_FoldedHeader("Partially applicable profiles", mk_PartlyApplicableProfilesWidget_, this);
    lk_IVLayout_->addWidget(mk_PartlyApplicableProfilesHeader_);
    lk_IVLayout_->addWidget(mk_PartlyApplicableProfilesWidget_);
    
    mk_NonApplicableProfilesHeader_ = new k_FoldedHeader("Currently non-applicable profiles", mk_NonApplicableProfilesWidget_, this);
    lk_IVLayout_->addWidget(mk_NonApplicableProfilesHeader_);
    lk_IVLayout_->addWidget(mk_NonApplicableProfilesWidget_);
    
    mk_HeaderForList[mk_ApplicableProfilesWidget_] = mk_ApplicableProfilesHeader_;
    mk_HeaderForList[mk_PartlyApplicableProfilesWidget_] = mk_PartlyApplicableProfilesHeader_;
    mk_HeaderForList[mk_NonApplicableProfilesWidget_] = mk_NonApplicableProfilesHeader_;
    
    lk_IVLayout_->setContentsMargins(0, 0, 4, 8);
    lk_IVLayout_->addStretch();
    QWidget* lk_IVLayoutWidget_ = new QWidget(this);
    lk_IVLayoutWidget_->setLayout(lk_IVLayout_);
    QScrollArea* lk_ScrollArea_ = new QScrollArea(this);
    lk_ScrollArea_->setWidgetResizable(true);
    lk_ScrollArea_->setFrameStyle(QFrame::NoFrame);
    lk_ScrollArea_->setWidget(lk_IVLayoutWidget_);
    lk_VSplitter_->addWidget(lk_ScrollArea_);
    
    mk_DescriptionLabel_ = new QLabel(this);
    mk_DescriptionLabel_->setWordWrap(true);
    //mk_DescriptionLabel_->setFrameStyle(QFrame::NoFrame);
    //mk_DescriptionLabel_->setReadOnly(true);
    //mk_DescriptionLabel_->setStyleSheet("QTextEdit { background-color: none; }");
    mk_DescriptionLabel_->setContentsMargins(8, 0, 0, 0);
    
    /*
    if (!mk_CurrentScript_)
        mk_DescriptionLabel_->hide();
    else
        */
    {
        QScrollArea* lk_ScrollArea_ = new QScrollArea(this);
        lk_ScrollArea_->setWidgetResizable(true);
        lk_ScrollArea_->setFrameStyle(QFrame::NoFrame);
        QWidget* lk_LabelVLayoutWidget_ = new QWidget(this);
        QBoxLayout* lk_LabelVLayout_ = new QVBoxLayout(lk_LabelVLayoutWidget_);
        lk_LabelVLayout_->addWidget(mk_DescriptionLabel_);
        lk_LabelVLayout_->addStretch();
        lk_LabelVLayoutWidget_->setLayout(lk_LabelVLayout_);
        lk_LabelVLayoutWidget_->setContentsMargins(0, 0, 0, 0);
        lk_LabelVLayout_->setContentsMargins(0, 0, 0, 0);
        lk_ScrollArea_->setWidget(lk_LabelVLayoutWidget_);
        lk_VSplitter_->addWidget(lk_ScrollArea_);
    }
    
    QBoxLayout* lk_HLayout_ = new QHBoxLayout();
    lk_HLayout_->setSpacing(10);
    lk_HLayout_->addStretch();
    mk_ApplyButton_ = new QPushButton(QIcon(":/icons/dialog-ok.png"), "Apply", this);
    connect(mk_ApplyButton_, SIGNAL(clicked()), this, SLOT(applyClicked()));
    lk_HLayout_->addWidget(mk_ApplyButton_);
    QPushButton* lk_CloseButton_ = new QPushButton(QIcon(":/icons/dialog-cancel.png"), "Close", this);
    connect(lk_CloseButton_, SIGNAL(clicked()), this, SLOT(reject()));
    lk_HLayout_->addWidget(lk_CloseButton_);
    lk_VLayout_->addLayout(lk_HLayout_);
    updateProfileList();
    resize(700, 300);
    toggleUi();
    
    // if any folded header is open, close all other folded headers
    if (mk_ApplicableProfilesWidget_->count() != 0)
    {
        mk_PartlyApplicableProfilesHeader_->hideBuddy();
        mk_NonApplicableProfilesHeader_->hideBuddy();
    }
    else if (mk_PartlyApplicableProfilesWidget_->count() != 0)
        mk_NonApplicableProfilesHeader_->hideBuddy();
        
    lk_VSplitter_->setStretchFactor(0, 1);
    lk_VSplitter_->setStretchFactor(1, 1);
    lk_VSplitter_->setSizes(QList<int>() << 350 << 350);
}


k_ProfileManager::~k_ProfileManager()
{
}


void k_ProfileManager::reset()
{
    mk_GoodProfileMix = QHash<QString, QString>();
    foreach (QListWidget* lk_ListWidget_, mk_HeaderForList.keys())
    {
        for (int i = 0; i < lk_ListWidget_->count(); ++i)
            lk_ListWidget_->item(i)->setCheckState(Qt::Unchecked);
    }
    foreach (QString ls_Key, mk_ProfileCheckState.keys())
        mk_ProfileCheckState[ls_Key] = Qt::Unchecked;
    this->updateProfileMix();
}


QHash<QString, QString> k_ProfileManager::getGoodProfileMix()
{
    return mk_GoodProfileMix;
}


void k_ProfileManager::toggleUi()
{
    mk_ApplyButton_->setEnabled(!mk_GoodProfileMixKeys.empty());
    mk_NewAction_->setEnabled(mk_CurrentScript_ && mk_CurrentScript_->hasParameters());
    mk_EditAction_->setEnabled(mk_CurrentScript_ && mk_SelectedItem_);
    mk_DeleteAction_->setEnabled(mk_SelectedItem_ != NULL);
    mk_ImportAction_->setEnabled(true);
    mk_ExportAction_->setEnabled(mk_SelectedItem_ != NULL);
    
    if (mk_ApplicableProfilesWidget_->count() == 0)
    {
        mk_ApplicableProfilesHeader_->hideBuddy();
        mk_ApplicableProfilesHeader_->setEnabled(false);
    }
    else
        mk_ApplicableProfilesHeader_->setEnabled(true);
        
    if (mk_PartlyApplicableProfilesWidget_->count() == 0)
    {
        mk_PartlyApplicableProfilesHeader_->hideBuddy();
        mk_PartlyApplicableProfilesHeader_->setEnabled(false);
        mk_PartlyApplicableProfilesHeader_->hide();
    }
    else
    {
        mk_PartlyApplicableProfilesHeader_->show();
        mk_PartlyApplicableProfilesWidget_->setEnabled(true);
    }
    
    if (mk_NonApplicableProfilesWidget_->count() == 0)
    {
        mk_NonApplicableProfilesHeader_->hideBuddy();
        mk_NonApplicableProfilesHeader_->setEnabled(false);
    }
    else
        mk_NonApplicableProfilesHeader_->setEnabled(true);
}


void k_ProfileManager::updateDescription()
{
    if (mk_AppliedProfiles.empty())
        mk_DescriptionLabel_->setText("<i>(no profile checked)</i>");
    else
    {
        QString ls_Label;
        
        ls_Label += "<b>Settings:</b><br />";
        
        if (mk_GoodProfileMixKeys.empty())
            ls_Label += "<i>(none)</i><br />";
        
        foreach (QString ls_Key, mk_GoodProfileMixKeys)
        {
            ls_Label += QString("%1: %2<br />").arg(mk_ProfileMix[ls_Key].first().ms_HumanReadableKey).arg(mk_ProfileMix[ls_Key].first().ms_HumanReadableValue);
        }
        
        if (!mk_ConflictingProfileMixKeys.empty())
        {
            // currently applied profile mix produced conflicts!
            ls_Label += "<b>Conflicts:</b><br />";
            foreach (QString ls_Key, mk_ConflictingProfileMixKeys)
            {
                /*
                QStringList lk_AllValues;
                foreach (r_ProfileMixInfo lr_ProfileMixInfo, mk_ProfileMix[ls_Key])
                    lk_AllValues.push_back(lr_ProfileMixInfo.ms_HumanReadableValue);
                    
                ls_Label += mk_ProfileMix[ls_Key].first().ms_HumanReadableKey + ": " + lk_AllValues.join(" / ") + "<br />";
                */
                ls_Label += mk_ProfileMix[ls_Key].first().ms_HumanReadableKey + "<br />";
            }
        }
        
        mk_DescriptionLabel_->setText(ls_Label);
    }
}


void k_ProfileManager::newProfile()
{
    if (!mk_CurrentScript_)
        return;
        
    k_EditProfileDialog lk_Dialog(mk_Proteomatic, mk_CurrentScript_, tk_YamlMap(), this);
    if (lk_Dialog.exec())
    {
        tk_YamlMap lk_Profile = lk_Dialog.getProfile();
        tk_YamlMap lk_Profiles = mk_Proteomatic.getConfiguration(CONFIG_PROFILES).toMap();
        lk_Profiles[lk_Profile["title"].toString()] = lk_Profile;
        mk_Proteomatic.getConfigurationRoot()[CONFIG_PROFILES] = lk_Profiles;
        mk_Proteomatic.saveConfiguration();
        this->updateProfileList(lk_Profile["title"].toString());
    }
}


void k_ProfileManager::editProfile()
{
    if (!mk_CurrentScript_)
        return;
        
    if (!mk_SelectedItem_)
        return;
        
    if (mk_Proteomatic.getConfiguration(CONFIG_WARN_ABOUT_MIXED_PROFILES).toBool() && mk_SelectedItem_->listWidget() != mk_ApplicableProfilesWidget_)
    {
        if (mk_Proteomatic.showMessageBox("Warning", "If you edit a profile which is not or not completely applicable, you might end up with partially applicable profiles. <br />If you are unsure about what that means, please select 'No'.<br />Are you sure you want to continue?", ":/icons/dialog-warning.png", QMessageBox::Yes | QMessageBox::No, QMessageBox::No, QMessageBox::No) == QMessageBox::No)
            return;
    }
        
    QString ls_OldTitle = mk_SelectedItem_->text();
    k_EditProfileDialog lk_Dialog(mk_Proteomatic, mk_CurrentScript_, mk_Proteomatic.getConfiguration(CONFIG_PROFILES).toMap()[ls_OldTitle].toMap(), this);
    if (lk_Dialog.exec())
    {
        tk_YamlMap lk_NewProfile = lk_Dialog.getProfile();
        tk_YamlMap lk_Profiles = mk_Proteomatic.getConfiguration(CONFIG_PROFILES).toMap();
        lk_Profiles.remove(ls_OldTitle);
        lk_Profiles[lk_NewProfile["title"].toString()] = lk_NewProfile;
        mk_Proteomatic.getConfigurationRoot()[CONFIG_PROFILES] = lk_Profiles;
        mk_Proteomatic.saveConfiguration();
        this->updateProfileList(lk_NewProfile["title"].toString());
    }
}


void k_ProfileManager::deleteProfile()
{
    if (mk_SelectedItem_ == NULL)
        return;
    
    QString ls_OldTitle = mk_SelectedItem_->text();
    if (mk_Proteomatic.showMessageBox("Warning", QString("Are you sure you want to delete %1?").arg(ls_OldTitle), ":/icons/dialog-cancel.png", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
    {
        tk_YamlMap lk_Profiles = mk_Proteomatic.getConfiguration(CONFIG_PROFILES).toMap();
        lk_Profiles.remove(ls_OldTitle);
        mk_Proteomatic.getConfigurationRoot()[CONFIG_PROFILES] = lk_Profiles;
        mk_Proteomatic.saveConfiguration();
        this->updateProfileList();
    }
}


void k_ProfileManager::importProfile()
{
    QString ls_StartingPath = mk_Proteomatic.getConfiguration(CONFIG_REMEMBER_PROFILE_PATH).toString();
    if (!QFileInfo(ls_StartingPath).isDir())
        ls_StartingPath = QDir::homePath();

    QString ls_Filename = QFileDialog::getOpenFileName(this, "Import profile", ls_StartingPath, "Proteomatic Profile (*.pp)");
    if (!ls_Filename.isEmpty())
    {
        mk_Proteomatic.getConfigurationRoot()[CONFIG_REMEMBER_PROFILE_PATH] = QFileInfo(ls_Filename).absolutePath();
        bool lb_Error = true;
        tk_YamlMap lk_Profile = k_Yaml::parseFromFile(ls_Filename).toMap();
        if (lk_Profile["identifier"].toString() == "Proteomatic Profile")
        {
            // identifier is good
            int li_Revision = lk_Profile["revision"].toInt();
            if (li_Revision > 0 && li_Revision < 2)
            {
                // revision number is good
                tk_YamlMap lk_ProfileContents = lk_Profile["content"].toMap();
                QString ls_Title = lk_ProfileContents["title"].toString();
                if (!ls_Title.isEmpty())
                {
                    if (mk_Proteomatic.getConfiguration(CONFIG_PROFILES).toMap().contains(ls_Title))
                    {
                        // error: a profile with the same name already exists
                        mk_Proteomatic.showMessageBox("Error", QString("A profile with the same name (%1) already exists.").arg(ls_Title), ":/icons/dialog-warning.png");
                        return;
                    }
                    else
                    {
                        // import the profile
                        tk_YamlMap lk_Profiles = mk_Proteomatic.getConfiguration(CONFIG_PROFILES).toMap();
                        lk_Profiles[ls_Title] = lk_ProfileContents;
                        mk_Proteomatic.getConfigurationRoot()[CONFIG_PROFILES] = lk_Profiles;
                        mk_Proteomatic.saveConfiguration();
                        updateProfileList();
                        lb_Error = false;
                    }
                }
            }
        }
        if (lb_Error)
            mk_Proteomatic.showMessageBox("Error", "The profile could not be imported.", ":/icons/dialog-warning.png");
    }
}


void k_ProfileManager::exportProfile()
{
    if (mk_SelectedItem_ == NULL)
        return;
    QString ls_Title = mk_SelectedItem_->text();
    QString ls_StartingPath = mk_Proteomatic.getConfiguration(CONFIG_REMEMBER_PROFILE_PATH).toString();
    if (!QFileInfo(ls_StartingPath).isDir())
        ls_StartingPath = QDir::homePath();

    QString ls_Filename = QFileDialog::getSaveFileName(this, "Export profile", ls_StartingPath + "/" + ls_Title + ".pp", "Proteomatic Profile (*.pp)");
    if (ls_Filename != "")
    {
        tk_YamlMap lk_Profile;
        lk_Profile["identifier"] = "Proteomatic Profile";
        lk_Profile["revision"] = "1";
        lk_Profile["content"] = mk_Proteomatic.getConfiguration(CONFIG_PROFILES).toMap()[ls_Title];
        k_Yaml::emitToFile(lk_Profile, ls_Filename);
        mk_Proteomatic.getConfigurationRoot()[CONFIG_REMEMBER_PROFILE_PATH] = QFileInfo(ls_Filename).absolutePath();
    }
}


void k_ProfileManager::updateProfileList(QString as_SelectedItem)
{
    tk_YamlMap lk_Profiles = mk_Proteomatic.getConfiguration(CONFIG_PROFILES).toMap();
    
    QHash<QString, QListWidget*> lk_ProfileTargets;
    QHash<QString, r_ProfileState::Enumeration> lk_ProfileStates;
    
    // classify profiles
    foreach (QString ls_Title, lk_Profiles.uniqueKeys())
    {
        lk_ProfileStates[ls_Title] = this->classifyProfile(lk_Profiles[ls_Title].toMap()["settings"].toMap());
        switch (lk_ProfileStates[ls_Title])
        {
            case r_ProfileState::Applicable:
                lk_ProfileTargets[ls_Title] = mk_ApplicableProfilesWidget_;
                break;
            case r_ProfileState::PartlyApplicable:
                lk_ProfileTargets[ls_Title] = mk_PartlyApplicableProfilesWidget_;
                break;
            case r_ProfileState::NonApplicable:
                lk_ProfileTargets[ls_Title] = mk_NonApplicableProfilesWidget_;
                break;
        }
    }
    
    mk_ApplicableProfilesWidget_->clear();
    mk_PartlyApplicableProfilesWidget_->clear();
    mk_NonApplicableProfilesWidget_->clear();
    mk_ApplicableProfilesHeader_->setEnabled(true);
    mk_PartlyApplicableProfilesHeader_->setEnabled(true);
    mk_NonApplicableProfilesHeader_->setEnabled(true);
    
    foreach (QString ls_Title, lk_Profiles.uniqueKeys())
    {
        QListWidgetItem* lk_Item_ = new k_CiListWidgetItem(ls_Title, lk_ProfileTargets[ls_Title]);
        QString ls_Description = lk_Profiles[ls_Title].toMap()["description"].toString();
        if (ls_Description.isEmpty())
            ls_Description = "<i>(no description)</i>";
        lk_Item_->setToolTip(ls_Description);
        lk_Item_->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        Qt::CheckState lr_State = Qt::Unchecked;
        if (lk_ProfileStates[ls_Title] != r_ProfileState::NonApplicable)
        {
            lk_Item_->setFlags(lk_Item_->flags() | Qt::ItemIsUserCheckable);
            if (mk_ProfileCheckState.contains(ls_Title))
                lr_State = mk_ProfileCheckState[ls_Title];
            
            lk_Item_->setCheckState(lr_State);
        }
        mk_ProfileCheckState[ls_Title] = lr_State;
        
        if (ls_Title == as_SelectedItem)
        {
            lk_ProfileTargets[ls_Title]->setCurrentItem(lk_Item_);
            mk_HeaderForList[lk_ProfileTargets[ls_Title]]->showBuddy();
        }
    }
    
    if (mk_ApplicableProfilesWidget_->count() != 0)
        mk_ApplicableProfilesHeader_->setSuffix(QString("(%1)").arg(mk_ApplicableProfilesWidget_->count()));
    else
        mk_ApplicableProfilesHeader_->setSuffix("");
        
    if (mk_PartlyApplicableProfilesWidget_->count() != 0)
        mk_PartlyApplicableProfilesHeader_->setSuffix(QString("(%1)").arg(mk_PartlyApplicableProfilesWidget_->count()));
    else
        mk_PartlyApplicableProfilesHeader_->setSuffix("");
    
    if (mk_NonApplicableProfilesWidget_->count() != 0)
        mk_NonApplicableProfilesHeader_->setSuffix(QString("(%1)").arg(mk_NonApplicableProfilesWidget_->count()));
    else
        mk_NonApplicableProfilesHeader_->setSuffix("");
    
    updateProfileMix();
}


void k_ProfileManager::currentProfileChanged()
{
    QListWidget* lk_Sender_ = dynamic_cast<QListWidget*>(sender());
    if (lk_Sender_ == NULL)
        return;
        
    mk_SelectedItem_ = NULL;
            
    if (lk_Sender_ != mk_ApplicableProfilesWidget_)
        mk_ApplicableProfilesWidget_->setCurrentRow(-1);
    if (lk_Sender_ != mk_PartlyApplicableProfilesWidget_)
        mk_PartlyApplicableProfilesWidget_->setCurrentRow(-1);
    if (lk_Sender_ != mk_NonApplicableProfilesWidget_)
        mk_NonApplicableProfilesWidget_->setCurrentRow(-1);
        
    if (lk_Sender_ == mk_ApplicableProfilesWidget_ && mk_ApplicableProfilesWidget_->currentItem() != NULL)
    {
        mk_SelectedItem_ = mk_ApplicableProfilesWidget_->currentItem();
        mk_ApplicableProfilesHeader_->showBuddy();
    }
    else if (lk_Sender_ == mk_PartlyApplicableProfilesWidget_ && mk_PartlyApplicableProfilesWidget_->currentItem() != NULL)
    {
        mk_SelectedItem_ = mk_PartlyApplicableProfilesWidget_->currentItem();
        mk_PartlyApplicableProfilesHeader_->showBuddy();
    }
    else if (lk_Sender_ == mk_NonApplicableProfilesWidget_ && mk_NonApplicableProfilesWidget_->currentItem() != NULL)
    {
        mk_SelectedItem_ = mk_NonApplicableProfilesWidget_->currentItem();
        mk_NonApplicableProfilesHeader_->showBuddy();
    }
    else
        mk_SelectedItem_ = NULL;
        
    if (mk_SelectedItem_ && mk_ProfileCheckState[mk_SelectedItem_->text()])
        this->profileClicked(mk_SelectedItem_);

    this->toggleUi();
}


void k_ProfileManager::profileClicked(QListWidgetItem* ak_Item_)
{
    bool lb_Changed = false;
    if (mk_ProfileCheckState[ak_Item_->text()] != ak_Item_->checkState())
    {
        mk_ProfileCheckState[ak_Item_->text()] = ak_Item_->checkState();
        lb_Changed = true;
    }
    if (lb_Changed)
        this->updateProfileMix();
}


void k_ProfileManager::updateProfileMix()
{
    mk_AppliedProfiles = QStringList();
        
    // determine new applied profile mix
    for (int i = 0; i < mk_ApplicableProfilesWidget_->count(); ++i)
    {
        QListWidgetItem* lk_Item_ = mk_ApplicableProfilesWidget_->item(i);
        if (lk_Item_->checkState() == Qt::Checked)
            mk_AppliedProfiles.push_back(lk_Item_->text());
    }
    for (int i = 0; i < mk_PartlyApplicableProfilesWidget_->count(); ++i)
    {
        QListWidgetItem* lk_Item_ = mk_PartlyApplicableProfilesWidget_->item(i);
        if (lk_Item_->checkState() == Qt::Checked)
            mk_AppliedProfiles.push_back(lk_Item_->text());
    }
    
    mk_ProfileMix = QHash<QString, QList<r_ProfileMixInfo> >();
    QSet<QString> mk_ProfileKeysSet;
    
    foreach (QString ls_ProfileTitle, mk_AppliedProfiles)
    {
        tk_YamlMap lk_Settings = mk_Proteomatic.getConfiguration(CONFIG_PROFILES).toMap()[ls_ProfileTitle].toMap()["settings"].toMap();
        foreach (QString ls_Key, lk_Settings.uniqueKeys())
        {
            mk_ProfileKeysSet.insert(ls_Key);
            
            if (!mk_ProfileMix.contains(ls_Key))
                mk_ProfileMix[ls_Key] = QList<r_ProfileMixInfo>();
            
            QString ls_Value = lk_Settings[ls_Key].toString();
            QString ls_HumanReadableValue = ls_Value;
            QString ls_HumanReadableKey = ls_Key;
            
            // fetch both human readable key and value from the currently loaded script, if possible
            // else use fallback verbose values from profile, if possible
            // if everything fails, just report the plain non-human readable value
            if (mk_CurrentScript_ && mk_CurrentScript_->parameterKeys().contains(ls_Key))
            {
                ls_HumanReadableKey = mk_CurrentScript_->parameterLabel(ls_Key);
                ls_HumanReadableValue = mk_CurrentScript_->humanReadableParameterValue(ls_Key, ls_Value);
            }
            else
            {
                tk_YamlMap lk_Verbose = mk_Proteomatic.getConfiguration(CONFIG_PROFILES).toMap()[ls_ProfileTitle].toMap()["verbose"].toMap();
                if (lk_Verbose.contains(ls_Key))
                {
                    tk_YamlMap lk_Info = lk_Verbose[ls_Key].toMap();
                    if (lk_Info.contains("key"))
                        ls_HumanReadableKey = lk_Info["key"].toString();
                    if (lk_Info.contains("value"))
                        ls_HumanReadableValue = lk_Info["value"].toString();
                }
            }
            
            r_ProfileMixInfo lr_ProfileMixInfo;
            lr_ProfileMixInfo.ms_ProfileTitle = ls_ProfileTitle;
            lr_ProfileMixInfo.ms_Value = ls_Value;
            lr_ProfileMixInfo.ms_HumanReadableKey = ls_HumanReadableKey;
            lr_ProfileMixInfo.ms_HumanReadableValue = ls_HumanReadableValue;
            mk_ProfileMix[ls_Key].push_back(lr_ProfileMixInfo);
        }
    }
    
    // sort profile mix keys according to the currently loaded script, if foreign parameter by name
    mk_ProfileMixKeysSorted = QStringList();
    if (mk_CurrentScript_)
    {
        // first come the script keys, if there is a script - sort as intended by the script
        foreach (QString ls_Key, mk_CurrentScript_->parameterKeys())
        {
            if (mk_ProfileKeysSet.contains(ls_Key))
            {
                mk_ProfileKeysSet.remove(ls_Key);
                mk_ProfileMixKeysSorted.push_back(ls_Key);
            }
        }
    }
    
    // no come the remaining keys, sorted by name
    QMap<QString, QString> lk_RemainingKeys;
    foreach (QString ls_Key, mk_ProfileKeysSet)
        lk_RemainingKeys[ls_Key.toLower()] = ls_Key;
    foreach (QString ls_Key, lk_RemainingKeys)
        mk_ProfileMixKeysSorted.push_back(ls_Key);
    
    mk_GoodProfileMixKeys = QStringList();
    mk_ConflictingProfileMixKeys = QStringList();
    foreach (QString ls_Key, mk_ProfileMixKeysSorted)
    {
        if (mk_ProfileMix[ls_Key].size() == 1)
            mk_GoodProfileMixKeys.push_back(ls_Key);
        else
            mk_ConflictingProfileMixKeys.push_back(ls_Key);
    }
    
    this->updateDescription();
    this->toggleUi();
}


r_ProfileState::Enumeration k_ProfileManager::classifyProfile(tk_YamlMap ak_Profile)
{
    if (ms_TargetScriptUri.isEmpty())
        return r_ProfileState::NonApplicable;
        
    QString ls_Keys = mk_TargetScriptParameterKeys.join("/");
    
    int li_ProfileKeys = ak_Profile.size();
    int li_MatchingKeys = 0;
    foreach (QString ls_Key, ak_Profile.keys())
    {
        if (mk_TargetScriptParameterKeys.contains(ls_Key))
            ++li_MatchingKeys;
    }
    if (li_MatchingKeys == 0)
        return r_ProfileState::NonApplicable;
    else if (li_MatchingKeys == li_ProfileKeys)
        return r_ProfileState::Applicable;
    else return r_ProfileState::PartlyApplicable;
}


void k_ProfileManager::applyClicked()
{
    mk_GoodProfileMix = QHash<QString, QString>();
    foreach (QString ls_Key, mk_GoodProfileMixKeys)
        mk_GoodProfileMix[ls_Key] = mk_ProfileMix[ls_Key].first().ms_Value;
    this->accept();
}
