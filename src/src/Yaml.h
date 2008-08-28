#pragma once

#include <QtCore>


typedef QMap<QString, QVariant> tk_YamlMap;
typedef QList<QVariant> tk_YamlSequence;


class k_Yaml
{
public:
	k_Yaml();
	virtual ~k_Yaml();
	
	static QVariant parseFromFile(QString as_Path);
	static void emitToFile(QVariant ak_Node, QString as_Path);
};
