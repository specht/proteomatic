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

#include <QtCore>


typedef QMap<QString, QVariant> tk_YamlMap;
typedef QList<QVariant> tk_YamlSequence;


class k_Yaml
{
public:
	k_Yaml();
	virtual ~k_Yaml();
	
	static QVariant parseFromFile(QString as_Path);
    static QVariant parseFromString(QString as_Yaml);
	static void emitToFile(QVariant ak_Node, QString as_Path);
    static QString emitToString(QVariant ak_Node);
};
