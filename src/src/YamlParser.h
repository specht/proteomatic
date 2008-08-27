#pragma once

#include "libyaml/yaml.h"


class k_YamlParser
{
public:
	k_YamlParser();
	virtual ~k_YamlParser();
	
	QVariant parseFromFile(QString as_Filename);
	QVariant parseFromString(QString as_Yaml);
	
protected:
 	QVariant parse();
	void fetchNextEvent();
	QVariant parseAny();
	QVariant parseMapping();
	QVariant parseSequence();

	yaml_parser_t mk_Parser;
	yaml_event_t mk_Event;
	int mi_Type;
	QString ms_Value;
	bool mb_Error;
};
