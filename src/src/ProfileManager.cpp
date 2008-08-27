#include "ProfileManager.h"


k_ProfileManager::k_ProfileManager(k_Proteomatic& ak_Proteomatic, QWidget * parent, Qt::WindowFlags f)
	: QDialog(parent, f)
	, mk_Proteomatic(ak_Proteomatic)
{
	setWindowTitle("Profile Manager");
	setModal(true);
	
	QToolBar* lk_ToolBar_ = new QToolBar("Profile actions", this);
	lk_ToolBar_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	mk_NewAction_ = lk_ToolBar_->addAction(QIcon(":/icons/list-add.png"), "New");
	mk_EditAction_ = lk_ToolBar_->addAction(QIcon(":/icons/preferences-system.png"), "Edit");
	mk_DeleteAction_ = lk_ToolBar_->addAction(QIcon(":/icons/dialog-cancel.png"), "Delete");
	lk_ToolBar_->addSeparator();
	mk_ImportAction_ = lk_ToolBar_->addAction(QIcon(":/icons/document-open.png"), "Import");
	mk_ExportAction_ = lk_ToolBar_->addAction(QIcon(":/icons/document-save.png"), "Export");
	
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
	mk_ListWidget_->setSelectionMode(QAbstractItemView::ExtendedSelection);

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
	updateDescription();
	toggleUi();
}


k_ProfileManager::~k_ProfileManager()
{
}


void k_ProfileManager::toggleUi()
{
	mk_NewAction_->setEnabled(true);
	mk_EditAction_->setEnabled(false);
	mk_DeleteAction_->setEnabled(false);
	mk_ImportAction_->setEnabled(true);
	mk_ExportAction_->setEnabled(false);
}


void k_ProfileManager::updateDescription()
{
	int li_CurrentRow = mk_ListWidget_->currentRow();
	if (li_CurrentRow == -1)
		mk_DescriptionLabel_->setText("<i>(no profile selected)</i>");
	else
		mk_DescriptionLabel_->setText(mk_Proteomatic.getConfiguration(CONFIG_PROFILES).toList()[mk_ListWidget_->item(li_CurrentRow)->data(Qt::UserRole).toInt()].toMap()["description"].toString());
}
