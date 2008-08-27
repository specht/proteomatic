#pragma once

#include <QtCore>


class k_Yaml
{
public:
	k_Yaml();
	virtual ~k_Yaml();
	
	static QVariant parseFromFile(QString as_Path);
	static void emitToFile(QVariant ak_Node, QString as_Path);
};
