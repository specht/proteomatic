#pragma once

#include <QtCore>
#include "libyaml/yaml.h"


class k_YamlEmitter
{
public:
	k_YamlEmitter();
	virtual ~k_YamlEmitter();
	
	QString emitToString(QVariant ak_Node);
	void emitToFile(QVariant ak_Node, QString as_Path);
	
protected:
	void emitAny(QVariant ak_Node);
	void emitSequence(QList<QVariant> ak_Node);
	void emitMapping(QMap<QString, QVariant> ak_Node);

	yaml_emitter_t mk_Emitter;
	yaml_event_t mk_Event;
	bool mb_Error;
};
