#include "ClickableLabel.h"


k_ClickableLabel::k_ClickableLabel(QWidget* parent, Qt::WindowFlags f)
	: QLabel(parent, f)
{
	this->setFocusPolicy(Qt::TabFocus);
}


k_ClickableLabel::k_ClickableLabel(const QString& text, QWidget* parent, Qt::WindowFlags f)
	: QLabel(text, parent, f)
{
	this->setFocusPolicy(Qt::TabFocus);
}


k_ClickableLabel::~k_ClickableLabel()
{
}


void k_ClickableLabel::mousePressEvent(QMouseEvent* event)
{
	event->accept();
	emit clicked();
}


void k_ClickableLabel::enterEvent(QMouseEvent* event)
{
	emit enter();
}


void k_ClickableLabel::leaveEvent(QMouseEvent* event)
{
	emit leave();
}


void k_ClickableLabel::focusInEvent(QFocusEvent* event)
{
	//this->setBackgroundRole(QPalette::AlternateBase);
}


void k_ClickableLabel::focusOutEvent(QFocusEvent* event)
{
	//this->setBackgroundRole(QPalette::NoRole);
}

