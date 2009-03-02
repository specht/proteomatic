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
#include "IScript.h"
#include "Proteomatic.h"
#include "Yaml.h"


class k_EditProfileDialog: public QDialog
{
	Q_OBJECT
	
public:
	k_EditProfileDialog(k_Proteomatic& ak_Proteomatic, IScript* ak_CurrentScript_, tk_YamlMap ak_OldProfile = tk_YamlMap(), QWidget * parent = 0, Qt::WindowFlags f = 0);
	virtual ~k_EditProfileDialog();
	
	tk_YamlMap getProfile();
	
protected slots:
	void applyClicked();
	void copyFromCurrentScript();
	
protected:
	bool mb_CreateNewMode;
	k_Proteomatic& mk_Proteomatic;
	IScript* mk_CurrentScript_;
	QString ms_TargetScriptUri;
	RefPtr<IScript> mk_pScript;
	QString ms_WindowTitle;
	QLineEdit* mk_ProfileTitle_;
	QLineEdit* mk_ProfileDescription_;
	QLabel* mk_ProfileDescriptionText_;
	tk_YamlMap mk_NonApplicableParameters;
	tk_YamlMap mk_NonApplicableParametersVerbose;
	QString ms_NonApplicableParametersDescription;
};
