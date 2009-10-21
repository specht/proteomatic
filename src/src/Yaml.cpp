/*
Copyright (c) 2007-2008 Michael Specht

This file is part of Proteomatic.

Proteomatic is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Proteomatic is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Proteomatic.  If not, see <http://www.gnu.org/licenses/>.
*/

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


QVariant k_Yaml::parseFromString(QString as_Yaml)
{
    k_YamlParser lk_Parser;
    return lk_Parser.parseFromString(as_Yaml);
}


void k_Yaml::emitToFile(QVariant ak_Node, QString as_Path)
{
	k_YamlEmitter lk_Emitter;
	lk_Emitter.emitToFile(ak_Node, as_Path);
}
