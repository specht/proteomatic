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


void k_YamlEmitter::emitToFile(QVariant ak_Node, QString as_Path)
{
	FILE* lk_File_ = fopen(as_Path.toStdString().c_str(), "wb");
	yaml_emitter_initialize(&mk_Emitter);
	yaml_emitter_set_output_file(&mk_Emitter, lk_File_);
	
	yaml_stream_start_event_initialize(&mk_Event, YAML_UTF8_ENCODING);
	yaml_emitter_emit(&mk_Emitter, &mk_Event);
	
	yaml_document_start_event_initialize(&mk_Event, NULL, NULL, NULL, 0);
	yaml_emitter_emit(&mk_Emitter, &mk_Event);
	
	this->emitAny(ak_Node);
	
	yaml_document_end_event_initialize(&mk_Event, 0);
	yaml_emitter_emit(&mk_Emitter, &mk_Event);

	yaml_stream_end_event_initialize(&mk_Event);
	yaml_emitter_emit(&mk_Emitter, &mk_Event);

	fclose(lk_File_);
	yaml_emitter_delete(&mk_Emitter);
}


void k_YamlEmitter::emitAny(QVariant ak_Node)
{
	if (ak_Node.type() == QVariant::Map)
		this->emitMapping(ak_Node.toMap());
	else if (ak_Node.type() == QVariant::List)
		this->emitSequence(ak_Node.toList());
	else
	{
		// emit scalar
		yaml_scalar_event_initialize(&mk_Event, NULL, NULL, (yaml_char_t*)ak_Node.toString().toStdString().c_str(), ak_Node.toString().length(), 0, 1, YAML_ANY_SCALAR_STYLE);
		yaml_emitter_emit(&mk_Emitter, &mk_Event);
	}
}

void k_YamlEmitter::emitSequence(QList<QVariant> ak_Node)
{
	yaml_sequence_start_event_initialize(&mk_Event, NULL, NULL, 1, YAML_ANY_SEQUENCE_STYLE);
	yaml_emitter_emit(&mk_Emitter, &mk_Event);
	
	foreach (QVariant lk_Node, ak_Node)
		this->emitAny(lk_Node);
		
	yaml_sequence_end_event_initialize(&mk_Event);
	yaml_emitter_emit(&mk_Emitter, &mk_Event);
}


void k_YamlEmitter::emitMapping(QMap<QString, QVariant> ak_Node)
{
	yaml_mapping_start_event_initialize(&mk_Event, NULL, NULL, 1, YAML_ANY_MAPPING_STYLE);
	yaml_emitter_emit(&mk_Emitter, &mk_Event);
	
	QMap<QString, QVariant>::const_iterator lk_Iter = ak_Node.begin();
	for (; lk_Iter != ak_Node.end(); ++lk_Iter)
	{
		this->emitAny(lk_Iter.key());
		this->emitAny(lk_Iter.value());
	}
		
	yaml_mapping_end_event_initialize(&mk_Event);
	yaml_emitter_emit(&mk_Emitter, &mk_Event);
}
