#include <QtCore>
#include "YamlParser.h"


k_YamlParser::k_YamlParser()
{
}


k_YamlParser::~k_YamlParser()
{
}


QVariant k_YamlParser::parseFromFile(QString as_Filename)
{
	mb_Error = false;
	FILE* lk_File = fopen(as_Filename.toStdString().c_str(), "rb");

	yaml_parser_initialize(&mk_Parser);

	yaml_parser_set_input_file(&mk_Parser, lk_File);
	
	QVariant lk_Result = this->parse();

	fclose(lk_File);
	
	return lk_Result;
}


QVariant k_YamlParser::parseFromString(QString as_Yaml)
{
	mb_Error = false;
	
	yaml_parser_initialize(&mk_Parser);

	yaml_parser_set_input_string(&mk_Parser, as_Yaml.toStdString().c_str(), as_Yaml.length());
	
	return this->parse();
}

QVariant k_YamlParser::parse()
{
	do 
	{
		this->fetchNextEvent(); 
		if (mb_Error)
			break;
	} while (mi_Type != YAML_DOCUMENT_START_EVENT);
	
	if (mb_Error)
		return QVariant();
	
	this->fetchNextEvent();
	if (mb_Error)
		return QVariant();
	
	QVariant lk_Node = parseAny();

	yaml_parser_delete(&mk_Parser);

	if (mb_Error)
		return QVariant();
	else
		return lk_Node;
}


void k_YamlParser::fetchNextEvent()
{
	if (!yaml_parser_parse(&mk_Parser, &mk_Event))
	{
		mb_Error = true;
	}
	mi_Type = mk_Event.type;
	ms_Value = mi_Type == YAML_SCALAR_EVENT ? QString((char*)mk_Event.data.scalar.value) : QString();
	yaml_event_delete(&mk_Event);
}


QVariant k_YamlParser::parseAny()
{
	switch (mi_Type)
	{
		case YAML_ALIAS_EVENT:
			printf("YAML_ALIAS_EVENT\n");
			break;
		case YAML_SCALAR_EVENT:
			return QVariant(ms_Value);
		case YAML_SEQUENCE_START_EVENT:
			return this->parseSequence();
		case YAML_MAPPING_START_EVENT:
			return this->parseMapping();
		default:
			printf("oops! it's a %d\n", mi_Type);
			break;
	}
	return QVariant();
}


QVariant k_YamlParser::parseMapping()
{
   	QMap<QString, QVariant> lk_Node;
	
	forever
	{
		// parse key / value pairs until end of mapping is reached
		this->fetchNextEvent();
		if (mb_Error)
			break;
		if (mi_Type == YAML_MAPPING_END_EVENT)
			break;
		QString ls_Key = ms_Value;
		
		this->fetchNextEvent();
		lk_Node[ls_Key] = parseAny();
	}
	if (mb_Error)
		return QVariant();
	return QVariant(lk_Node);
}

QVariant k_YamlParser::parseSequence()
{
   	QList<QVariant> lk_Node;
   	
	forever
	{
		// parse values until end of sequence is reached
		fetchNextEvent();
		if (mb_Error)
			break;
		if (mi_Type == YAML_SEQUENCE_END_EVENT)
			break;
			
		lk_Node.push_back(parseAny());
	}
	if (mb_Error)
		return QVariant();
	return QVariant(lk_Node);
}
