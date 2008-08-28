#include "ProfileManager.h"
#include "RefPtr.h"
#include "Script.h"
#include "dialogs/EditProfileDialog.h"


k_ProfileManager::k_ProfileManager(k_Proteomatic& ak_Proteomatic, QString as_TargetScriptUri, QWidget * parent, Qt::WindowFlags f)
	: QDialog(parent, f)
	, mk_Proteomatic(ak_Proteomatic)
	, ms_TargetScriptUri(as_TargetScriptUri)
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
	mk_ListWidget_ = new QListWidget(this);
	mk_ListWidget_->setSortingEnabled(true);
	mk_ListWidget_->setSelectionMode(QAbstractItemView::SingleSelection);

	lk_VSplitter_->addWidget(mk_ListWidget_);
	
	QListWidgetItem* lk_Item_;
	int li_Index = 0;
	foreach (QVariant lk_ProfileVariant, mk_Proteomatic.getConfiguration(CONFIG_PROFILES).toList())
	{
		QMap<QString, QVariant> lk_Profile = lk_ProfileVariant.toMap();
		lk_Item_ = new QListWidgetItem(lk_Profile["title"].toString(), mk_ListWidget_);
		lk_Item_->setData(Qt::UserRole, li_Index);
		lk_Item_->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		lk_Item_->setCheckState(Qt::Unchecked);
		++li_Index;
	}
	
	connect(mk_ListWidget_, SIGNAL(currentRowChanged(int)), this, SLOT(updateDescription()));
	connect(mk_ListWidget_, SIGNAL(currentRowChanged(int)), this, SLOT(toggleUi()));
	mk_DescriptionLabel_ = new QLabel("", this);
	mk_DescriptionLabel_->setWordWrap(true);
	mk_DescriptionLabel_->setContentsMargins(8, 0, 0, 0);
	mk_DescriptionLabel_->setAlignment(Qt::AlignLeft | Qt::AlignTop);
	lk_VSplitter_->addWidget(mk_DescriptionLabel_);
	lk_VSplitter_->setStretchFactor(0, 1);
	lk_VSplitter_->setStretchFactor(1, 1);
	
	QBoxLayout* lk_HLayout_ = new QHBoxLayout(this);
	lk_HLayout_->addStretch();
	QPushButton* lk_CloseButton_ = new QPushButton(QIcon(":/icons/dialog-ok.png"), "Close", this);
	connect(lk_CloseButton_, SIGNAL(clicked()), this, SLOT(accept()));
	lk_HLayout_->addWidget(lk_CloseButton_);
	lk_VLayout_->addLayout(lk_HLayout_);
	updateProfileList();
	toggleUi();
}


k_ProfileManager::~k_ProfileManager()
{
}


void k_ProfileManager::toggleUi()
{
	mk_NewAction_->setEnabled(true);
	mk_EditAction_->setEnabled(mk_ListWidget_->currentRow() != -1);
	mk_DeleteAction_->setEnabled(mk_ListWidget_->currentRow() != -1);
	mk_ImportAction_->setEnabled(true);
	mk_ExportAction_->setEnabled(mk_ListWidget_->currentRow() != -1);
}


void k_ProfileManager::updateDescription()
{
	int li_CurrentRow = mk_ListWidget_->currentRow();
	if (li_CurrentRow == -1)
		mk_DescriptionLabel_->setText("<i>(no profile selected)</i>");
	else
	{
		QString ls_Description = mk_Proteomatic.getConfiguration(CONFIG_PROFILES).toMap()[mk_ListWidget_->item(li_CurrentRow)->text()].toMap()["description"].toString();
		if (ls_Description.isEmpty())
			ls_Description = "<i>(no description)</i>";
		mk_DescriptionLabel_->setText(ls_Description);
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
	if (mk_ListWidget_->currentRow() == -1)
		return;
	QString ls_OldTitle = mk_ListWidget_->item(mk_ListWidget_->currentRow())->text();
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
	if (mk_ListWidget_->currentRow() == -1)
		return;
	
	QString ls_OldTitle = mk_ListWidget_->item(mk_ListWidget_->currentRow())->text();
	if (mk_Proteomatic.showMessageBox("Warning", QString("Are you sure you want to delete %1?").arg(ls_OldTitle), ":/icons/dialog-cancel.png", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
	{
		tk_YamlMap lk_Profiles = mk_Proteomatic.getConfiguration(CONFIG_PROFILES).toMap();
		lk_Profiles.remove(ls_OldTitle);
		mk_Proteomatic.getConfigurationRoot()[CONFIG_PROFILES] = lk_Profiles;
		mk_Proteomatic.saveConfiguration();
		this->updateProfileList();
	}
}


void k_ProfileManager::exportProfile()
{
	if (mk_ListWidget_->currentRow() == -1)
		return;
	QString ls_Title = mk_ListWidget_->item(mk_ListWidget_->currentRow())->text();
	QString ls_Filename = QFileDialog::getSaveFileName(this, "Export profile", QDir::homePath() + "/" + ls_Title + ".pp", "Proteomatic Profile (*.pp)");
	if (ls_Filename != "")
	{
		tk_YamlMap lk_Profile = mk_Proteomatic.getConfiguration(CONFIG_PROFILES).toMap()[ls_Title].toMap();
		k_Yaml::emitToFile(lk_Profile, ls_Filename);
	}
}


void k_ProfileManager::updateProfileList(QString as_SelectedItem)
{
	tk_YamlMap lk_Profiles = mk_Proteomatic.getConfiguration(CONFIG_PROFILES).toMap();
	mk_ListWidget_->clear();
	foreach (QString ls_Title, lk_Profiles.uniqueKeys())
	{
		QListWidgetItem* lk_Item_ = new QListWidgetItem(ls_Title, mk_ListWidget_);
		lk_Item_->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		lk_Item_->setCheckState(Qt::Unchecked);
		if (ls_Title == as_SelectedItem)
			mk_ListWidget_->setCurrentItem(lk_Item_);
	}
	updateDescription();
	toggleUi();
}
