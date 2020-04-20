#include "assembler.h"

#include <stdbool.h>
#include <stdint.h>

bool stringdb_memory_setup(
	struct stringdb_memory * __restrict const stringdb)
{
	stringdb->memory_space =
		myy_vector_utf8_init(8192*1024);
	if (!myy_vector_utf8_is_valid(&stringdb->memory_space))
		goto no_memory_space;

	stringdb->mappings = myy_vector_stringdb_mappings_init(
		8192
		* 1024
		/ sizeof(struct stringdb_mappings));

	if (!myy_vector_stringdb_mappings_is_valid(&stringdb->mappings))
		goto no_mappings;

	return true;

no_mappings:
	myy_vector_utf8_free_content(stringdb->memory_space);
no_memory_space:
	return false;
}

uint64_t strings_add_c_string(
	global_state_t * __restrict const state,
	char const * __restrict const string)
{
	struct stringdb_memory * __restrict const stringdb =
		&state->strings;

	myy_vector_utf8 * __restrict const strings =
		&stringdb->memory_space;

	uint32_t const string_size = strlen(string);
	size_t const last_index = myy_vector_utf8_length(strings);

	myy_vector_utf8_add(strings, string_size, (uint8_t const *) string);
	myy_vector_utf8_add(strings, 1, (uint8_t const *) "\0");

	struct stringdb_mappings mapping = {
		.id = global_unique_id(state),
		.index = last_index,
		.size  = string_size
	};

	LOG("[STRINGS_ADD_C_STRING] Added id : %lu\n", mapping.id);

	myy_vector_stringdb_mappings_add(
		&stringdb->mappings, 1, &mapping);

	return mapping.id;
}

static char const * invalid_string = "";

bool string_data_valid(
	string_data_t * __restrict const data)
{
	return 
		(data->text != invalid_string)
		& (data->size != 0);
}

static int64_t index_of_string_mapping(
	global_state_t * __restrict const state,
	string_id_t const string_id)
{
	struct stringdb_memory * __restrict const stringdb =
		&state->strings;

	myy_vector_stringdb_mappings * __restrict const mappings_vector =
		&stringdb->mappings;

	uint64_t const n_mappings =
		myy_vector_stringdb_mappings_length(mappings_vector);

	struct stringdb_mappings const * __restrict current_mapping =
		myy_vector_stringdb_mappings_data(mappings_vector);

	for (uint64_t i = 0; i < n_mappings; i++, current_mapping++) {
		if (current_mapping->id == string_id) return i;
	}

	return -1;
}

string_data_t strings_get(
	global_state_t * __restrict const state,
	string_id_t const string_id)
{
	LOG("global_state_t : %p - string_id : %lu\n", state, string_id);
	int64_t mapping_idx =
		index_of_string_mapping(state, string_id);

	if (mapping_idx == -1) {
		LOG("Invalid string ID : %" PRIu64 , string_id);
		string_data_t ret_data = {invalid_string, 0};
		return ret_data;
	}

	struct stringdb_memory * __restrict const stringdb =
		&state->strings;

	struct stringdb_mappings const mapping =
		myy_vector_stringdb_mappings_at(
			&state->strings.mappings, mapping_idx);

	uint8_t const * __restrict const strings =
		myy_vector_utf8_data(&stringdb->memory_space);

	string_data_t string = {
		(char const *) strings+mapping.index,
		mapping.size
	};

	return string;
}

void strings_remove(
	global_state_t * __restrict const state,
	string_id_t const string_id)
{
	int64_t mapping_idx =
		index_of_string_mapping(state, string_id);

	if (mapping_idx == -1) {
		LOG("Cannot remove unknown string ID : %" PRIu64 , string_id);
		return;
	}

	myy_vector_stringdb_mappings_delete(
		&state->strings.mappings, mapping_idx);

}
