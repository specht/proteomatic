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

#include "FoldedHeader.h"


k_FoldedHeader::k_FoldedHeader(QWidget* ak_Buddy_, QWidget* parent, Qt::WindowFlags f)
	: QWidget(parent, f)
	, mk_Buddy_(ak_Buddy_)
{
	init();
}


k_FoldedHeader::k_FoldedHeader(const QString& text, QWidget* ak_Buddy_, QWidget* parent, Qt::WindowFlags f)
	: QWidget(parent, f)
	, mk_Buddy_(ak_Buddy_)
	, ms_Text(text)
{
	mk_Label.setText(ms_Text);
	init();
}


k_FoldedHeader::~k_FoldedHeader()
{
}


void k_FoldedHeader::hideBuddy()
{
	if (mk_Buddy_ == NULL)
		return;
	mb_Increasing = false;
	mk_Timer.start();
	mk_Buddy_->setVisible(false);
	emit hidingBuddy();
	//mk_Label.setForegroundRole(QPalette::Text);
	//mk_Icon.setPixmap(mk_FoldedIcon);
	
}


void k_FoldedHeader::showBuddy()
{
	if (mk_Buddy_ == NULL)
		return;
	mb_Increasing = true;
	mk_Timer.start();
	mk_Buddy_->setVisible(true);
	emit showingBuddy();
	//mk_Label.setForegroundRole(QPalette::Text);
	//mk_Icon.setPixmap(mk_UnfoldedIcon);
}


bool k_FoldedHeader::buddyVisible()
{
	if (mk_Buddy_ == NULL)
		return false;
	return mk_Buddy_->isVisible();
}


void k_FoldedHeader::toggleBuddy()
{
	if (buddyVisible())
		hideBuddy();
	else
		showBuddy();
}


void k_FoldedHeader::setText(const QString& as_Text)
{
    mk_Label.setText(as_Text);
}


void k_FoldedHeader::setSuffix(QString as_Text)
{
	mk_Label.setText(ms_Text + " " + as_Text);
}


void k_FoldedHeader::mousePressEvent(QMouseEvent* event)
{
	event->accept();
	emit clicked();
}


void k_FoldedHeader::mouseReleaseEvent(QMouseEvent* event)
{
    /*
    event->accept();
    emit clicked();
    */
    event->ignore();
}


void k_FoldedHeader::enterEvent(QMouseEvent* event)
{
	(void)event;
	emit enter();
}


void k_FoldedHeader::leaveEvent(QMouseEvent* event)
{
	(void)event;
	emit leave();
}


void k_FoldedHeader::update()
{
	if (mb_Increasing)
	{
		if (mi_CurrentIndex < 6)
			++mi_CurrentIndex;
		else 
			mk_Timer.stop();
	}
	else
	{
		if (mi_CurrentIndex > 0)
			--mi_CurrentIndex;
		else 
			mk_Timer.stop();
	}
	mk_Icon.setPixmap(*mk_FoldedIcons[mi_CurrentIndex].get_Pointer());
}


void k_FoldedHeader::init()
{
	for (int i = 0; i < 7; ++i)
		mk_FoldedIcons.push_back(RefPtr<QPixmap>(new QPixmap(QString(":/icons/folded-%1.png").arg(i))));
		
	mi_CurrentIndex = 0;
	mb_Increasing = true;
	mk_Icon.setPixmap(*mk_FoldedIcons[mi_CurrentIndex].get_Pointer());
	QBoxLayout* lk_Layout_ = new QHBoxLayout(this);
	lk_Layout_->addWidget(&mk_Icon);
	lk_Layout_->addWidget(&mk_Label);
	lk_Layout_->addStretch();
	lk_Layout_->setContentsMargins(0, 0, 0, 0);
	setLayout(lk_Layout_);
	setCursor(Qt::PointingHandCursor);
	mk_Timer.setInterval(20);
	mk_Timer.setSingleShot(false);
	connect(&mk_Label, SIGNAL(released()), this, SIGNAL(clicked()));
	connect(&mk_Icon, SIGNAL(released()), this, SIGNAL(clicked()));
	connect(&mk_Timer, SIGNAL(timeout()), this, SLOT(update()));
    connect(&mk_Label, SIGNAL(clicked()), this, SLOT(toggleBuddy()));
    connect(&mk_Icon, SIGNAL(clicked()), this, SLOT(toggleBuddy()));
}
