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

#pragma once

#include "libyaml/ly_yaml.h"


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
