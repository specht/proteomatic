#pragma once

#include <QtGui>
#include "Desktop.h"
#include "Proteomatic.h"


class k_PipelineMainWindow: public QMainWindow
{
	Q_OBJECT
public:
	k_PipelineMainWindow(QWidget* ak_Parent_, k_Proteomatic& ak_Proteomatic);
	virtual ~k_PipelineMainWindow();

public slots:
	void mouseModeButtonClicked();

protected:
	k_Desktop mk_Desktop;
	QToolButton* mk_MouseMoveButton_;
	QToolButton* mk_MouseArrowButton_;
	k_Proteomatic& mk_Proteomatic;
};
