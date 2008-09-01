#include "CiListWidgetItem.h"
#include "FoldedHeader.h"
#include "ProfileManager.h"
#include "RefPtr.h"
#include "Script.h"
#include "dialogs/EditProfileDialog.h"


k_ProfileManager::k_ProfileManager(k_Proteomatic& ak_Proteomatic, QString as_TargetScriptUri, QStringList ak_TargetScriptParameterKeys, QWidget * parent, Qt::WindowFlags f)
	: QDialog(parent, f)
	, mk_Proteomatic(ak_Proteomatic)
	, ms_TargetScriptUri(as_TargetScriptUri)
	, mk_TargetScriptParameterKeys(ak_TargetScriptParameterKeys)
	, mk_SelectedItem_(NULL)
{
	setWindowTitle("Profile Manager");
	setModal(true);
	
	QToolBar* lk_ToolBar_ = new QToolBar("Profile actions", this);
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
	lk_VLayout_ = new QVBoxLayout(this);
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
	
	mk_DescriptionLabel_ = new QTextEdit(this);
	mk_DescriptionLabel_->setFrameStyle(QFrame::NoFrame);
	mk_DescriptionLabel_->setReadOnly(true);
	mk_DescriptionLabel_->setStyleSheet("QTextEdit { background-color: none; }");
	mk_DescriptionLabel_->setContentsMargins(8, 0, 0, 0);
	
	if (ms_TargetScriptUri.isEmpty())
		mk_DescriptionLabel_->hide();
	else
	{
		lk_VSplitter_->addWidget(mk_DescriptionLabel_);
		lk_VSplitter_->setStretchFactor(0, 1);
		lk_VSplitter_->setStretchFactor(1, 1);
	}
	
	QBoxLayout* lk_HLayout_ = new QHBoxLayout(this);
	lk_HLayout_->addStretch();
	QPushButton* lk_CloseButton_ = new QPushButton(QIcon(":/icons/dialog-ok.png"), "Close", this);
	connect(lk_CloseButton_, SIGNAL(clicked()), this, SLOT(accept()));
	lk_HLayout_->addWidget(lk_CloseButton_);
	lk_VLayout_->addLayout(lk_HLayout_);
	updateProfileList();
	resize(500, 300);
	toggleUi();
	
	// if any folded header is open, close all other folded headers
	if (mk_ApplicableProfilesWidget_->count() != 0)
	{
		mk_PartlyApplicableProfilesHeader_->hideBuddy();
		mk_NonApplicableProfilesHeader_->hideBuddy();
	}
	else if (mk_PartlyApplicableProfilesWidget_->count() != 0)
		mk_NonApplicableProfilesHeader_->hideBuddy();
}


k_ProfileManager::~k_ProfileManager()
{
}


void k_ProfileManager::toggleUi()
{
	mk_NewAction_->setEnabled(!ms_TargetScriptUri.isEmpty());
	mk_EditAction_->setEnabled((!ms_TargetScriptUri.isEmpty()) && mk_SelectedItem_ != NULL);
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
	//if (!mk_ApplicableProfilesHeader_->buddyVisble() && !mk_PartlyApplicableProfilesHeader_->buddyVisble()
}


void k_ProfileManager::updateDescription()
{
	if (mk_AppliedProfiles.empty())
		mk_DescriptionLabel_->setText("<i>(no profile applied)</i>");
	else
	{
		//mk_DescriptionLabel_->setText(mk_AppliedProfiles.join(", "));
		QString ls_Label;
		//foreach (QString ls_AppliedProfiles
		QStringList lk_ConflictingParameterKeys;
		foreach (QString ls_Key, mk_ProfileMixParameterKeys.keys())
			if (mk_ProfileMixParameterKeys[ls_Key].size() != 1)
				lk_ConflictingParameterKeys.push_back(ls_Key);
				
		if (!lk_ConflictingParameterKeys.empty())
		{
			// currently applied profile mix produced conflicts!
			ls_Label += "<b>Conflicts:</b><br />";
			qSort(lk_ConflictingParameterKeys);
			foreach (QString ls_Key, lk_ConflictingParameterKeys)
			{
				ls_Label += ls_Key + "<br />";
			}
		}
		
		ls_Label += "<b>Settings:</b><br />";
		
		foreach (QString ls_Key, mk_ProfileMixParameterKeys.keys())
		{
			ls_Label += QString("%1: %2").arg(ls_Key).arg(mk_ProfileMixParameterKeys[ls_Key].size());
		}
		mk_DescriptionLabel_->setText(ls_Label);
	}
}


void k_ProfileManager::newProfile()
{
	k_EditProfileDialog lk_Dialog(mk_Proteomatic, ms_TargetScriptUri, tk_YamlMap(), this);
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
	if (mk_SelectedItem_ == NULL)
		return;
	QString ls_OldTitle = mk_SelectedItem_->text();
	k_EditProfileDialog lk_Dialog(mk_Proteomatic, ms_TargetScriptUri, mk_Proteomatic.getConfiguration(CONFIG_PROFILES).toMap()[ls_OldTitle].toMap(), this);
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
	QString ls_Filename = QFileDialog::getOpenFileName(this, "Import profile", mk_Proteomatic.getConfiguration(CONFIG_REMEMBER_PROFILE_PATH).toString(), "Proteomatic Profile (*.pp)");
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
	QString ls_Filename = QFileDialog::getSaveFileName(this, "Export profile", mk_Proteomatic.getConfiguration(CONFIG_REMEMBER_PROFILE_PATH).toString() + "/" + ls_Title + ".pp", "Proteomatic Profile (*.pp)");
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
	
	updateDescription();
	toggleUi();
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

	this->toggleUi();
}


void k_ProfileManager::profileClicked(QListWidgetItem* ak_Item_)
{
	if (mk_ProfileCheckState[ak_Item_->text()] == ak_Item_->checkState())
		return;
	
	mk_ProfileCheckState[ak_Item_->text()] = ak_Item_->checkState();
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
	
	mk_ProfileMixParameterKeys = QHash<QString, QStringList>();
	
	foreach (QString ls_ProfileTitle, mk_AppliedProfiles)
	{
		tk_YamlMap lk_Settings = mk_Proteomatic.getConfiguration(CONFIG_PROFILES).toMap()[ls_ProfileTitle].toMap()["settings"].toMap();
		foreach (QString ls_Key, lk_Settings.uniqueKeys())
		{
			if (!mk_ProfileMixParameterKeys.contains(ls_Key))
				mk_ProfileMixParameterKeys[ls_Key] = QStringList();
			mk_ProfileMixParameterKeys[ls_Key].push_back(ls_ProfileTitle);
		}
	}
	
	this->updateDescription();
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
