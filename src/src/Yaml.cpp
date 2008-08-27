#include "Yaml.h"
#include "YamlParser.h"
#include "YamlEmitter.h"


k_Yaml::k_Yaml()
{
}


k_Yaml::~k_Yaml()
{
}


QVariant k_Yaml::parseFromFile(QString as_Path)
{
	k_YamlParser lk_Parser;
	return lk_Parser.parseFromFile(as_Path);
}


void k_Yaml::emitToFile(QVariant ak_Node, QString as_Path)
{
	k_YamlEmitter lk_Emitter;
	lk_Emitter.emitToFile(ak_Node, as_Path);
}
