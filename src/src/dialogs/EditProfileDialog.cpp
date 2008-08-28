#include "EditProfileDialog.h"
#include "ScriptFactory.h"


k_EditProfileDialog::k_EditProfileDialog(k_Proteomatic& ak_Proteomatic, QString as_TargetScriptUri, tk_YamlMap ak_OldProfile, QWidget * parent, Qt::WindowFlags f)
	: QDialog(parent, f)
	, mb_CreateNewMode(ak_OldProfile.empty())
	, mk_Proteomatic(ak_Proteomatic)
	, ms_TargetScriptUri(as_TargetScriptUri)
	, ms_WindowTitle(ak_OldProfile.empty() ? "Create new profile" : "Edit profile")
	, mk_ProfileTitle_(NULL)
	, mk_ProfileDescription_(NULL)
{
	mk_pScript = RefPtr<k_Script>(k_ScriptFactory::makeScript(ms_TargetScriptUri, mk_Proteomatic, false, true));
	this->setWindowTitle(ms_WindowTitle);
	
	QSplitter* lk_HSplitter_ = new QSplitter(this);
	lk_HSplitter_->setStyle(new QPlastiqueStyle());
	lk_HSplitter_->setOrientation(Qt::Horizontal);
	lk_HSplitter_->setHandleWidth(4);
	lk_HSplitter_->setChildrenCollapsible(false);
	QScrollArea* lk_ScrollArea_ = new QScrollArea(this);
	lk_ScrollArea_->setWidgetResizable(true);
	lk_ScrollArea_->setFrameStyle(QFrame::NoFrame);
	
	QBoxLayout* lk_MainLayout_ = new QVBoxLayout(this);
	QBoxLayout* lk_VLayout_;
	QBoxLayout* lk_HLayout_;
	lk_VLayout_ = new QVBoxLayout(this);
	lk_HLayout_ = new QHBoxLayout(this);
	lk_HLayout_->addWidget(new QLabel("Title:", this));
	mk_ProfileTitle_ = new QLineEdit(this);
	if (!mb_CreateNewMode)
		mk_ProfileTitle_->setText(ak_OldProfile["title"].toString());
	mk_ProfileTitle_->home(false);
	lk_HLayout_->addWidget(mk_ProfileTitle_);
	lk_VLayout_->addLayout(lk_HLayout_);
	lk_HLayout_ = new QHBoxLayout(this);
	lk_HLayout_->addWidget(new QLabel("Description:", this));
	mk_ProfileDescription_ = new QLineEdit(this);
	if (!mb_CreateNewMode)
		mk_ProfileDescription_->setText(ak_OldProfile["description"].toString());
	mk_ProfileDescription_->home(false);
	lk_HLayout_->addWidget(mk_ProfileDescription_);
	lk_VLayout_->addLayout(lk_HLayout_);
	
	if (!mb_CreateNewMode)
		mk_pScript->applyProfile(ak_OldProfile["settings"].toMap());
	
	QFrame* lk_Frame_ = new QFrame(this);
	lk_Frame_->setFrameStyle(QFrame::HLine | QFrame::Sunken);
	lk_VLayout_->addWidget(lk_Frame_);
	
	//lk_VLayout_->addWidget(new QLabel("Settings:", this));
	
	QTextEdit* lk_ProfileDescriptionLabel_ = new QTextEdit(this);
	lk_ProfileDescriptionLabel_->setFrameStyle(QFrame::NoFrame);
	lk_ProfileDescriptionLabel_->setBackgroundRole(QPalette::Window);
	lk_ProfileDescriptionLabel_->setReadOnly(true);
	lk_ProfileDescriptionLabel_->setStyleSheet("QTextEdit { background-color: none; }");
	lk_VLayout_->addWidget(lk_ProfileDescriptionLabel_);
	
	QWidget* lk_VLayoutWidget_ = new QWidget(this);
	lk_VLayoutWidget_->setLayout(lk_VLayout_);
	lk_HSplitter_->addWidget(lk_VLayoutWidget_);
	lk_MainLayout_->addWidget(lk_HSplitter_);
	lk_ScrollArea_->setWidget(mk_pScript->parameterWidget());
	lk_HSplitter_->addWidget(lk_ScrollArea_);
	lk_HLayout_ = new QHBoxLayout(this);
	lk_HLayout_->addStretch();
	QPushButton* lk_CancelButton_ = new QPushButton(QIcon(":/icons/dialog-cancel.png"), "&Cancel", this);
	connect(lk_CancelButton_, SIGNAL(clicked()), this, SLOT(reject()));
	lk_HLayout_->addWidget(lk_CancelButton_);
	QPushButton* lk_SaveButton_ = new QPushButton(QIcon(":/icons/dialog-ok.png"), "&Save", this);
	lk_SaveButton_->setDefault(true);
	connect(lk_SaveButton_, SIGNAL(clicked()), this, SLOT(applyClicked()));
	lk_HLayout_->addWidget(lk_SaveButton_);
	lk_MainLayout_->addLayout(lk_HLayout_);
	this->setLayout(lk_MainLayout_);
	//lk_MainLayout_->setContentsMargins(0, 0, 0, 0);
	lk_ProfileDescriptionLabel_->setText(mk_pScript->profileDescription());
	connect(mk_pScript.get_Pointer(), SIGNAL(profileDescriptionChanged(const QString&)), lk_ProfileDescriptionLabel_, SLOT(setText(const QString&)));
	lk_HSplitter_->setStretchFactor(0, 4);
	lk_HSplitter_->setStretchFactor(1, 5);
}


k_EditProfileDialog::~k_EditProfileDialog()
{
}


tk_YamlMap k_EditProfileDialog::getProfile()
{
	tk_YamlMap lk_Profile;
	lk_Profile["title"] = mk_ProfileTitle_->text();
	lk_Profile["description"] = mk_ProfileDescription_->text();
	lk_Profile["settings"] = mk_pScript->getProfile();
		
	return lk_Profile;
}


void k_EditProfileDialog::applyClicked()
{
	// check if title is defined
	QString ls_Title = mk_ProfileTitle_->text();
	if (ls_Title.isEmpty())
		mk_Proteomatic.showMessageBox("Error", "Please specify a profile title.", ":/icons/dialog-warning.png");
	else
	{
		// check if a profile with the same name already exists
		if (mb_CreateNewMode)
		{
			if (mk_Proteomatic.getConfiguration(CONFIG_PROFILES).toMap().contains(ls_Title))
				mk_Proteomatic.showMessageBox("Error", "A profile with the same title already exists.", ":/icons/dialog-warning.png");
			else
				this->accept();
		}
		else
			this->accept();
	}
}
