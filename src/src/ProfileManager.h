#pragma once

#include <QtCore>
#include <QtGui>
#include "Proteomatic.h"


class k_ProfileManager: public QDialog
{
	Q_OBJECT
	
public:
	k_ProfileManager(k_Proteomatic& ak_Proteomatic, QWidget * parent = 0, Qt::WindowFlags f = 0);
	virtual ~k_ProfileManager();
	
protected slots:
	void toggleUi();
	void updateDescription();
	
protected:
	k_Proteomatic& mk_Proteomatic;
	QAction* mk_NewAction_;
	QAction* mk_EditAction_;
	QAction* mk_DeleteAction_;
	QAction* mk_ImportAction_;
	QAction* mk_ExportAction_;
	QLabel* mk_DescriptionLabel_;
	QListWidget* mk_ListWidget_;
};
