/*
Copyright (c) 2007-2011 Michael Specht

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

#include <QtCore>
#include "Yaml.h"
#include "YamlParser.h"


k_YamlParser::k_YamlParser()
{
}


k_YamlParser::~k_YamlParser()
{
}


QVariant k_YamlParser::parseFromString(QString as_Yaml)
{
    std::basic_istringstream<char> lk_InfoStream(as_Yaml.toStdString());
    try {
        YAML::Parser lk_Parser(lk_InfoStream);
        YAML::Node lk_Document;
        lk_Parser.GetNextDocument(lk_Document);
        return parseAny(&lk_Document);
    }
    catch (YAML::ParserException& e)
    {
        //there was a parsing error!
        return QVariant();
    }
}


QVariant k_YamlParser::parseAny(const YAML::Node* ak_Node_)
{
    switch (ak_Node_->Type())
    {
    case YAML::NodeType::Scalar:
        return parseScalar(ak_Node_);
        break;
    case YAML::NodeType::Sequence:
        return parseSequence(ak_Node_);
        break;
    case YAML::NodeType::Map:
        return parseMap(ak_Node_);
        break;
    case YAML::NodeType::Null:
        return QVariant();
        break;
    }
    return QVariant();
}


QVariant k_YamlParser::parseScalar(const YAML::Node* ak_Node_)
{
    std::string ls_Scalar;
    *ak_Node_ >> ls_Scalar;
    return QVariant(ls_Scalar.c_str());
}


QVariant k_YamlParser::parseSequence(const YAML::Node* ak_Node_)
{
    tk_YamlSequence lk_Result;
    for (YAML::Iterator lk_Iter = ak_Node_->begin(); lk_Iter != ak_Node_->end(); ++lk_Iter)
        lk_Result << parseAny(&(*lk_Iter));
    return QVariant(lk_Result);
}


QVariant k_YamlParser::parseMap(const YAML::Node* ak_Node_)
{
    tk_YamlMap lk_Result;
    for (YAML::Iterator lk_Iter = ak_Node_->begin(); lk_Iter != ak_Node_->end(); ++lk_Iter)
        lk_Result[parseScalar(&lk_Iter.first()).toString()] = parseAny(&lk_Iter.second());
    return QVariant(lk_Result);
}
