#pragma once

#include <QtCore>
#include <QtGui>
#include "FoldedHeader.h"
#include "Proteomatic.h"


struct r_ProfileState
{
	enum Enumeration
	{
		Applicable,
		PartlyApplicable,
		NonApplicable
	};
};


class k_ProfileManager: public QDialog
{
	Q_OBJECT
	
public:
	k_ProfileManager(k_Proteomatic& ak_Proteomatic, QString as_TargetScriptUri = QString(), QStringList ak_TargetScriptParameterKeys = QStringList(), QWidget * parent = 0, Qt::WindowFlags f = 0);
	virtual ~k_ProfileManager();
	
protected slots:
	void toggleUi();
	void updateDescription();
	void newProfile();
	void editProfile();
	void deleteProfile();
	void exportProfile();
	void importProfile();
	void updateProfileList(QString as_SelectedItem = QString());
	void currentProfileChanged();
	void profileClicked(QListWidgetItem* ak_Item_);
	void updateProfileMix();
	
protected:
	r_ProfileState::Enumeration classifyProfile(tk_YamlMap ak_Profile);

	k_Proteomatic& mk_Proteomatic;
	QString ms_TargetScriptUri;
	QStringList mk_TargetScriptParameterKeys;
	QAction* mk_NewAction_;
	QAction* mk_EditAction_;
	QAction* mk_DeleteAction_;
	QAction* mk_ImportAction_;
	QAction* mk_ExportAction_;
	QTextEdit* mk_DescriptionLabel_;
	k_FoldedHeader* mk_ApplicableProfilesHeader_;
	k_FoldedHeader* mk_PartlyApplicableProfilesHeader_;
	k_FoldedHeader* mk_NonApplicableProfilesHeader_;
	QListWidget* mk_ApplicableProfilesWidget_;
	QListWidget* mk_PartlyApplicableProfilesWidget_;
	QListWidget* mk_NonApplicableProfilesWidget_;
	QHash<QListWidget*, k_FoldedHeader*> mk_HeaderForList;
	QListWidgetItem* mk_SelectedItem_;
	QHash<QString, Qt::CheckState> mk_ProfileCheckState;
	QStringList mk_AppliedProfiles;
};
