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

#include "YamlEmitter.h"


k_YamlEmitter::k_YamlEmitter()
{
}


k_YamlEmitter::~k_YamlEmitter()
{
}


QString k_YamlEmitter::emitToString(QVariant ak_Node)
{
    emitAny(ak_Node);
    return QString(mk_Emitter.c_str());
}


void k_YamlEmitter::emitAny(QVariant ak_Node)
{
    if (ak_Node.canConvert<tk_YamlMap>())
        emitMap(ak_Node.toMap());
    else if (ak_Node.canConvert<tk_YamlSequence>())
        emitSequence(ak_Node.toList());
    else if (ak_Node.canConvert<QString>())
        emitScalar(ak_Node.toString());
    else
        mk_Emitter << YAML::Null;
}


void k_YamlEmitter::emitScalar(QString as_Node)
{
    mk_Emitter << as_Node.toStdString();
}


void k_YamlEmitter::emitSequence(tk_YamlSequence ak_Node)
{
    if (ak_Node.size() == 0)
        mk_Emitter << YAML::Flow;
    mk_Emitter << YAML::BeginSeq;
    foreach (QVariant lk_Item, ak_Node)
        emitAny(lk_Item);
    mk_Emitter << YAML::EndSeq;
    mk_Emitter << YAML::Block;
}


void k_YamlEmitter::emitMap(tk_YamlMap ak_Node)
{
    if (ak_Node.size() == 0)
        mk_Emitter << YAML::Flow;
    mk_Emitter << YAML::BeginMap;
    tk_YamlMap::const_iterator lk_Iter = ak_Node.begin();
    for (; lk_Iter != ak_Node.end(); ++lk_Iter)
    {
        mk_Emitter << YAML::Key;
        emitScalar(lk_Iter.key());
        mk_Emitter << YAML::Value;
        emitAny(lk_Iter.value());
    }
    mk_Emitter << YAML::EndMap;
    mk_Emitter << YAML::Block;
}
