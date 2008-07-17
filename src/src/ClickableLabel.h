#pragma once

#include <QtCore>
#include <QtGui>


class k_ClickableLabel: public QLabel
{
	Q_OBJECT
public:
	k_ClickableLabel(QWidget* parent = 0, Qt::WindowFlags f = 0);
	k_ClickableLabel(const QString& text, QWidget* parent = 0, Qt::WindowFlags f = 0);
	~k_ClickableLabel();

signals:
	void clicked();
	void enter();
	void leave();

protected:
	virtual void mousePressEvent(QMouseEvent* event);
	virtual void enterEvent(QMouseEvent* event);
	virtual void leaveEvent(QMouseEvent* event);
};
