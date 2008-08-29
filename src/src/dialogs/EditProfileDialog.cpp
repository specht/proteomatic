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
	
	// determine incoming parameters that are non-applicable to the current script
	QStringList lk_ScriptKeys = mk_pScript->getParameterKeys();
	foreach (QString ls_OldProfileKey, ak_OldProfile["settings"].toMap().uniqueKeys())
	{
		if (!lk_ScriptKeys.contains(ls_OldProfileKey))
		{
			mk_NonApplicableParameters[ls_OldProfileKey] = ak_OldProfile["settings"].toMap()[ls_OldProfileKey];
			mk_NonApplicableParametersVerbose[ls_OldProfileKey] = ak_OldProfile["verbose"].toMap()[ls_OldProfileKey];
		}
	}
	
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
	
	QScrollArea* lk_DescriptionScrollArea_ = new QScrollArea(this);
	lk_DescriptionScrollArea_->setWidgetResizable(true);
	lk_DescriptionScrollArea_->setFrameStyle(QFrame::NoFrame);
	QBoxLayout* lk_ScrollLayout_ = new QVBoxLayout(this);

	if (!mk_NonApplicableParametersVerbose.empty())
	{
		QStringList lk_Keys = mk_NonApplicableParametersVerbose.uniqueKeys();
		qSort(lk_Keys);
		QStringList lk_Items;
		ms_NonApplicableParametersDescription = "<i>Note: Although these parameters cannot be changed in the context of the current script, they will be preserved when you change the profile.</i><br />";
		foreach (QString ls_Key, lk_Keys)
		{
			lk_Items.push_back(QString("%1: %2")
				.arg(mk_NonApplicableParametersVerbose[ls_Key].toMap()["key"].toString())
				.arg(mk_NonApplicableParametersVerbose[ls_Key].toMap()["value"].toString()));
		}
		ms_NonApplicableParametersDescription += lk_Items.join("<br />");
		QLabel* lk_Info_ = new QLabel(ms_NonApplicableParametersDescription, this);
		k_FoldedHeader* lk_Header_ = new k_FoldedHeader("<b>Non-applicable profile parameters<b>", lk_Info_, this);
		lk_ScrollLayout_->addWidget(lk_Header_);
		lk_ScrollLayout_->addWidget(lk_Info_);
		lk_Info_->setWordWrap(true);
		lk_Header_->hideBuddy();
	}
	
	mk_ProfileDescriptionText_ = new QLabel(this);
	mk_ProfileDescriptionText_->setWordWrap(true);
	k_FoldedHeader* lk_Header_ = new k_FoldedHeader("<b>Profile parameters<b>", mk_ProfileDescriptionText_, this);
	lk_ScrollLayout_->addWidget(lk_Header_);
	lk_ScrollLayout_->addWidget(mk_ProfileDescriptionText_);
	lk_ScrollLayout_->addStretch();
	lk_ScrollLayout_->setContentsMargins(0, 0, 0, 0);
	lk_Header_->showBuddy();
	
	QWidget* lk_ScrollLayoutWidget_ = new QWidget(this);
	lk_ScrollLayoutWidget_->setLayout(lk_ScrollLayout_);
	lk_DescriptionScrollArea_->setWidget(lk_ScrollLayoutWidget_);
	lk_VLayout_->addWidget(lk_DescriptionScrollArea_);
	
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
	mk_ProfileDescriptionText_->setText(mk_pScript->profileDescription());
	connect(mk_pScript.get_Pointer(), SIGNAL(profileDescriptionChanged(const QString&)), mk_ProfileDescriptionText_, SLOT(setText(const QString&)));

	lk_HSplitter_->setStretchFactor(0, 4);
	lk_HSplitter_->setStretchFactor(1, 5);
	resize(800, 400);
}


k_EditProfileDialog::~k_EditProfileDialog()
{
}


tk_YamlMap k_EditProfileDialog::getProfile()
{
	tk_YamlMap lk_Profile;
	lk_Profile["title"] = mk_ProfileTitle_->text();
	lk_Profile["description"] = mk_ProfileDescription_->text();
	
	tk_YamlMap lk_Settings = mk_pScript->getProfile();
	// add all non-applicable parameters that came in, so that they are not lost
	foreach (QString ls_Key, mk_NonApplicableParameters.uniqueKeys())
		lk_Settings[ls_Key] = mk_NonApplicableParameters[ls_Key];
	
	lk_Profile["settings"] = lk_Settings;
	
	tk_YamlMap lk_Verbose;
	
	foreach (QString ls_Key, lk_Profile["settings"].toMap().uniqueKeys())
	{
		tk_YamlMap lk_Entry;
		lk_Entry["key"] = mk_pScript->getHumanReadableParameterKey(ls_Key);
		lk_Entry["value"] = mk_pScript->getHumanReadableParameterValue(ls_Key);
		lk_Verbose[ls_Key] = lk_Entry;
	}
	// add all non-applicable parameters that came in, so that they are not lost
	foreach (QString ls_Key, mk_NonApplicableParametersVerbose.uniqueKeys())
		lk_Verbose[ls_Key] = mk_NonApplicableParametersVerbose[ls_Key];
		
	lk_Profile["verbose"] = lk_Verbose;
	
		
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