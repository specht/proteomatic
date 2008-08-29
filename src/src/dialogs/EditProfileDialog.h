#pragma once

#include <QtCore>
#include <QtGui>
#include "Proteomatic.h"
#include "Script.h"
#include "Yaml.h"


class k_EditProfileDialog: public QDialog
{
	Q_OBJECT
	
public:
	k_EditProfileDialog(k_Proteomatic& ak_Proteomatic, QString as_TargetScriptUri, tk_YamlMap ak_OldProfile = tk_YamlMap(), QWidget * parent = 0, Qt::WindowFlags f = 0);
	virtual ~k_EditProfileDialog();
	
	tk_YamlMap getProfile();
	
protected slots:
	void applyClicked();
	
protected:
	bool mb_CreateNewMode;
	k_Proteomatic& mk_Proteomatic;
	QString ms_TargetScriptUri;
	RefPtr<k_Script> mk_pScript;
	QString ms_WindowTitle;
	QLineEdit* mk_ProfileTitle_;
	QLineEdit* mk_ProfileDescription_;
	QLabel* mk_ProfileDescriptionText_;
	tk_YamlMap mk_NonApplicableParameters;
	tk_YamlMap mk_NonApplicableParametersVerbose;
	QString ms_NonApplicableParametersDescription;
};
