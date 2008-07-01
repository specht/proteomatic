#pragma once

#include <QtCore>
#include <QtGui>
#include "ClickableLabel.h"
#include "RefPtr.h"


class k_FoldedHeader: public QWidget
{
	Q_OBJECT
public:
	k_FoldedHeader(QWidget* ak_Buddy_ = NULL, QWidget* parent = 0, Qt::WindowFlags f = 0);
	k_FoldedHeader(const QString& text, QWidget* ak_Buddy_ = NULL, QWidget* parent = 0, Qt::WindowFlags f = 0);
	~k_FoldedHeader();

	void hideBuddy();
	void showBuddy();
	bool buddyVisible();
	void toggleBuddy();

signals:
	void clicked();
	void enter();
	void leave();
	
protected slots:
	void update();	

protected:
	void init();
	virtual void mousePressEvent(QMouseEvent* event);
	virtual void enterEvent(QMouseEvent* event);
	virtual void leaveEvent(QMouseEvent* event);
	
	QWidget* mk_Buddy_;
	k_ClickableLabel mk_Label;
	k_ClickableLabel mk_Icon;
	QList<RefPtr<QPixmap> > mk_FoldedIcons;
	QTimer mk_Timer;
	int mi_CurrentIndex;
	bool mb_Increasing;
};
