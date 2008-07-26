#pragma once

#include <QtCore>
#include <QtGui>


class k_ProfileManager: public QDialog
{
	Q_OBJECT
	
public:
	k_ProfileManager(QWidget * parent = 0, Qt::WindowFlags f = 0);
	virtual ~k_ProfileManager();
	
protected slots:
	void toggleUi();
	void updateDescription();
	
protected:
	QAction* mk_NewAction_;
	QAction* mk_EditAction_;
	QAction* mk_DeleteAction_;
	QAction* mk_ImportAction_;
	QAction* mk_ExportAction_;
	QLabel* mk_DescriptionLabel_;
	QListWidget* mk_ListWidget_;
};
