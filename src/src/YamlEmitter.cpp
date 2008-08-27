#include "YamlEmitter.h"


k_YamlEmitter::k_YamlEmitter()
{
}


k_YamlEmitter::~k_YamlEmitter()
{
}


QString k_YamlEmitter::emitToString(QVariant ak_Node)
{
}


void k_YamlEmitter::emitToFile(QVariant ak_Node, QString as_Path)
{
	FILE* lk_File_ = fopen(as_Path.toStdString().c_str(), "wb");
	yaml_emitter_initialize(&mk_Emitter);
	yaml_emitter_set_output_file(&mk_Emitter, lk_File_);
	
	yaml_stream_start_event_initialize(&mk_Event, YAML_UTF8_ENCODING);
	yaml_emitter_emit(&mk_Emitter, &mk_Event);
	
	printf("%d ", yaml_document_start_event_initialize(&mk_Event, NULL, NULL, NULL, 0));
	yaml_emitter_emit(&mk_Emitter, &mk_Event);
	
	this->emitAny(ak_Node);
	
	printf("%d ", yaml_document_end_event_initialize(&mk_Event, 0));
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
		yaml_scalar_event_initialize(&mk_Event, NULL, NULL, ak_Node.toString().toStdString().c_str(), ak_Node.toString().length(), 0, 1, 0);
		yaml_emitter_emit(&mk_Emitter, &mk_Event);
	}
}

void k_YamlEmitter::emitSequence(QList<QVariant> ak_Node)
{
	yaml_sequence_start_event_initialize(&mk_Event, NULL, NULL, 1, 0);
	yaml_emitter_emit(&mk_Emitter, &mk_Event);
	
	foreach (QVariant lk_Node, ak_Node)
		this->emitAny(lk_Node);
		
	yaml_sequence_end_event_initialize(&mk_Event);
	yaml_emitter_emit(&mk_Emitter, &mk_Event);
}


void k_YamlEmitter::emitMapping(QMap<QString, QVariant> ak_Node)
{
	yaml_mapping_start_event_initialize(&mk_Event, NULL, NULL, 1, 0);
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
