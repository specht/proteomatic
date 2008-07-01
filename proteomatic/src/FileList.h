#pragma once

#include <QtGui>


class k_FileList: public QListWidget
{
	Q_OBJECT
public:
	k_FileList(QWidget* ak_Parent_, bool ab_ReallyRemoveItems);
	~k_FileList();
	void forceRemove(QList<QListWidgetItem *> ak_List);

signals:
	void remove(QList<QListWidgetItem *>);
	void selectionChanged(bool);
	void doubleClick();

public slots:
	void removeSelection();
	void selectionChanged();

protected:
	virtual void keyPressEvent(QKeyEvent* ak_Event_);
	virtual void mouseDoubleClickEvent(QMouseEvent* event);

private:
	bool mb_ReallyRemoveItems;
};
