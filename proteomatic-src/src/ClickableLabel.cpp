#include "ClickableLabel.h"


k_ClickableLabel::k_ClickableLabel(QWidget* parent, Qt::WindowFlags f)
	: QLabel(parent, f)
{
}


k_ClickableLabel::k_ClickableLabel(const QString& text, QWidget* parent, Qt::WindowFlags f)
	: QLabel(text, parent, f)
{
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
