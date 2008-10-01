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

#pragma once

#include <QtCore>
#include <QtGui>
#include "FoldedHeader.h"
#include "Proteomatic.h"

class k_Script;


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
	k_ProfileManager(k_Proteomatic& ak_Proteomatic, k_Script* ak_CurrentScript_ = NULL, QWidget * parent = 0, Qt::WindowFlags f = 0);
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
	k_Script* mk_CurrentScript_;
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
	QHash<QString, QStringList> mk_ProfileMixParameterKeys;
};
