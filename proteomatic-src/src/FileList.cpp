#include "FileList.h"


k_FileList::k_FileList(QWidget* ak_Parent_, bool ab_ReallyRemoveItems)
	: QListWidget(ak_Parent_)
	, mb_ReallyRemoveItems(ab_ReallyRemoveItems)
{
	setSelectionMode(QAbstractItemView::ExtendedSelection);
	connect(this, SIGNAL(itemSelectionChanged()), this, SLOT(selectionChanged()));
}


k_FileList::~k_FileList()
{
}


void k_FileList::forceRemove(QList<QListWidgetItem *> ak_List)
{
	emit remove(ak_List);
}


void k_FileList::removeSelection()
{
	int li_Row = currentRow();

	emit remove(selectedItems());

	if (mb_ReallyRemoveItems)
		foreach (QListWidgetItem* lk_Item_, selectedItems())
			delete lk_Item_;

	if (li_Row < count())
		setCurrentRow(li_Row);
}


void k_FileList::keyPressEvent(QKeyEvent* ak_Event_)
{
	if (ak_Event_->key() == Qt::Key_Delete)
	{
		ak_Event_->accept();
		removeSelection();
	}
	if (ak_Event_->key() == Qt::Key_Up)
	{
		ak_Event_->accept();
		if (currentRow() > 0)
			setCurrentRow(currentRow() - 1);
	}
	if (ak_Event_->key() == Qt::Key_Down)
	{
		ak_Event_->accept();
		if (currentRow() < count() - 1)
			setCurrentRow(currentRow() + 1);
	}
	if (ak_Event_->key() == Qt::Key_A && ((ak_Event_->modifiers() & Qt::ControlModifier) != 0))
	{
		ak_Event_->accept();
		selectAll();
	}
}


void k_FileList::mouseDoubleClickEvent(QMouseEvent* event)
{
	event->accept();
	emit doubleClick();
}


void k_FileList::selectionChanged()
{
	emit selectionChanged(!selectedItems().empty());
}
