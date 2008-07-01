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
{
	mk_Label.setText(text);
	init();
}


k_FoldedHeader::~k_FoldedHeader()
{
}


void k_FoldedHeader::hideBuddy()
{
	if (mk_Buddy_ == NULL)
		return;
	mk_Buddy_->setVisible(false);
	mb_Increasing = false;
	mk_Timer.start();
	//mk_Label.setForegroundRole(QPalette::Text);
	//mk_Icon.setPixmap(mk_FoldedIcon);
	
}


void k_FoldedHeader::showBuddy()
{
	if (mk_Buddy_ == NULL)
		return;
	mk_Buddy_->setVisible(true);
	mb_Increasing = true;
	mk_Timer.start();
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


void k_FoldedHeader::mousePressEvent(QMouseEvent* event)
{
	event->accept();
	emit clicked();
}


void k_FoldedHeader::enterEvent(QMouseEvent* event)
{
	emit enter();
}


void k_FoldedHeader::leaveEvent(QMouseEvent* event)
{
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
	connect(&mk_Label, SIGNAL(clicked()), this, SIGNAL(clicked()));
	connect(&mk_Label, SIGNAL(enter()), this, SIGNAL(enter()));
	connect(&mk_Label, SIGNAL(leave()), this, SIGNAL(leave()));
	connect(&mk_Icon, SIGNAL(clicked()), this, SIGNAL(clicked()));
	connect(&mk_Icon, SIGNAL(enter()), this, SIGNAL(enter()));
	connect(&mk_Icon, SIGNAL(leave()), this, SIGNAL(leave()));
	connect(&mk_Timer, SIGNAL(timeout()), this, SLOT(update()));
}
