#pragma once

#include <QtGui>


class k_CiListWidgetItem: public QObject, public QListWidgetItem
{
	Q_OBJECT
public:
	k_CiListWidgetItem(const QString & text, QListWidget * parent = 0, int type = UserType) 
		: QListWidgetItem(text, parent, type) 
	{ }

	k_CiListWidgetItem(const QListWidgetItem& other) 
		: QListWidgetItem(other) 
	{ }

	virtual ~k_CiListWidgetItem() { }

	virtual bool operator<(const QListWidgetItem &other) const
	{
		return text().compare(other.text(), Qt::CaseInsensitive) < 0;
	}
};
