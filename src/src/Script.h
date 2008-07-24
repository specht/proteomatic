#pragma once

#include <QtGui>
#include "RefPtr.h"
#include "FoldedHeader.h"
#include "Proteomatic.h"
#include "SizeWatchWidget.h"


struct r_ScriptType
{
	enum Enumeration
	{
		Local = 0,
		Remote
	};
};


class k_Script: public QObject
{
	Q_OBJECT

public:
	k_Script(r_ScriptType::Enumeration ae_Type, QString as_ScriptUri, k_Proteomatic& ak_Proteomatic, bool ab_IncludeOutputFiles = true, bool ab_ProfileMode = false);
	virtual ~k_Script();
	bool isGood();
	bool hasParameters();
	
	r_ScriptType::Enumeration type() const;
	k_SizeWatchWidget* parameterWidget();
	QString uri();
	virtual QString title();
	virtual QString description();

	void reset();

	void setPrefix(QString as_Prefix);
	QString prefix();
	QList<QString> outFiles();
	QHash<QString, QString> outFileDetails(QString as_Key);

	QString getParameterValue(QString as_Key);
	void setParameterValue(QString as_Key, QString as_Value);

	QString getConfiguration();
	void setConfiguration(QString as_Configuration);

	QStringList commandLineArguments();
	
	virtual void start(QStringList ak_Parameters) = 0;
	virtual void kill() = 0;
	virtual bool running() = 0;
	virtual QString readAll() = 0;

protected slots:
	void toggleGroup();
	void addChoiceItems();
	void removeChoiceItems(QList<QListWidgetItem *> ak_Items);
	void setOutputDirectoryButtonClicked();
	void clearOutputDirectoryButtonClicked();
	void resetDialog();
	void toggleUi();
	void toggleParameter(int ai_State);

protected:
	void addChoiceItems(QString as_Key, QStringList ak_Choices);
	void createParameterWidget(QStringList ak_Definition, bool ab_IncludeOutputFiles = true, bool ab_ProfileMode = false);

	r_ScriptType::Enumeration me_Type;
	QString ms_ScriptUri;
	QString ms_Title;
	QString ms_Description;
	RefPtr<k_SizeWatchWidget> mk_pParameterWidget;
	QHash<QString, QWidget* > mk_ParameterValueWidgets;
	QHash<QString, QWidget* > mk_ParameterDisplayWidgets;
	QHash<QString, QList<QWidget*> > mk_ParameterMultiChoiceWidgets;
	QHash<QString, QDialog* > mk_ParameterMultiChoiceDialogs;
	QList<k_FoldedHeader*> mk_FoldedHeaders;
	QHash<QString, QHash<QString, QString> > mk_OutFileDetails;
	QString ms_Prefix;
	QString ms_DefaultConfiguration;
	k_Proteomatic& mk_Proteomatic;
	bool mb_IsGood;
	bool mb_HasParameters;
	QLineEdit* mk_OutputDirectory_;
	QToolButton* mk_ClearOutputDirectory_;
	QStringList mk_InputFileDescriptionList;
};
