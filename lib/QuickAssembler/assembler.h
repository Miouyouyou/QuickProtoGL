#ifndef MYY_ASSEMBLER_ASSEMBLER_H
#define MYY_ASSEMBLER_ASSEMBLER_H 1

#include "myy/myy.h"

#include <stdint.h>
#include <stdbool.h>

struct global_state;
struct string_data;
struct instruction;
struct instruction_field;
struct instruction_field_fill;
struct instructions_collection;

typedef struct global_state global_state_t;
typedef uint64_t string_id_t;
typedef struct string_data string_data_t;
typedef struct instructions_collection instructions_collection_t;
typedef struct instruction instruction_t;
typedef struct instruction_field instruction_field_t;
typedef struct instruction_field_fill instruction_field_fill_t;


enum instruction_field_types {
	instruction_field_type_invalid,
	instruction_field_type_fill,
	instruction_field_type_count
};

struct instruction_field {
	uint64_t id;
	string_id_t name_id; // To replace by a string id
	uint32_t type;
	uint8_t infos[12];
};

struct instruction_field_fill {
	uint64_t id;
	string_id_t name_id;
	uint32_t type;
	uint16_t pos;
	uint16_t size;
	uint8_t infos[8];
};

myy_vector_template(instruction_field, instruction_field_t)

/* TODO Fixed size array for fields. 15 fields should be enough, no ?
 * But let's fix our attention on the real thing first */
struct instruction {
	uint64_t id;
	string_id_t name_id; // To change
	uint64_t base_value;
	myy_vector_instruction_field fields;
};

myy_vector_template(instruction, instruction_t)

struct instructions_collection {
	uint64_t id;
	string_id_t name_id; // TO change
	myy_vector_instruction instructions;
};

myy_vector_template(instructions_collection, instructions_collection_t)

struct string_data {
	char const * __restrict const text;
	uint32_t size;
};

struct stringdb_mappings {
	uint64_t id;
	uint32_t index;
	uint32_t size;
};

myy_vector_template(stringdb_mappings, struct stringdb_mappings)

myy_vector_template(uint64, uint64_t)

struct stringdb_memory {
	myy_vector_utf8 memory_space;
	myy_vector_stringdb_mappings mappings;
};

/* Q: How to remove a field if we can't get it first ? */
/* A: We will first list them, when displaying them,
 *    and get their references from there.
 *    And then relist when adding an item.
 *    This should avoid dealing with too much corner
 *    cases.
 *    We're getting near the goal !
 */

struct global_state {
	uint64_t id;
	myy_vector_instructions_collection collections;
	struct stringdb_memory strings;
};

bool global_state_setup(
	global_state_t * __restrict const state,
	uint64_t start_id);

uint64_t global_unique_id(
	global_state_t * __restrict const state);

instructions_collection_t * instruction_collections_add(
	global_state_t * __restrict const state,
	char const * __restrict const name);

bool stringdb_memory_setup(
	struct stringdb_memory * __restrict const stringdb);

uint64_t strings_add_c_string(
	global_state_t * __restrict const state,
	char const * __restrict const string);

void strings_remove(
	global_state_t * __restrict const state,
	string_id_t const string_id);

bool string_data_valid(
	string_data_t * __restrict const data);

string_data_t strings_get(
	global_state_t * __restrict const state,
	string_id_t const string_id);

void instructions_collection_setup(
	instructions_collection_t * __restrict const added_collection,
	global_state_t * __restrict const state,
	char const * __restrict const name);

instructions_collection_t * instructions_collections_add(
	myy_vector_instructions_collection * __restrict const collections,
	global_state_t * __restrict const state,
	char const * __restrict const name);

void instructions_collections_destroy(
	myy_vector_instructions_collection * __restrict const collection,
	global_state_t * __restrict const state);

void instructions_collection_destroy(
	instructions_collection_t * __restrict const collection,
	global_state_t * __restrict const state);

instruction_t * instructions_add(
	myy_vector_instruction * __restrict const instructions,
	global_state_t * __restrict const state,
	char const * __restrict const name,
	uint64_t const value);

instruction_t instruction_new(
	global_state_t * __restrict const state,
	string_id_t const mnemonic);

void instruction_setup(
	instruction_t * __restrict const added_instruction,
	global_state_t * __restrict const state,
	char const * __restrict const name,
	uint64_t const base_value);

void instruction_dump_infos_with_values(
	instruction_t * __restrict const instruction,
	global_state_t * __restrict const state,
	myy_vector_uint64 * __restrict const fields_values);

void instruction_destroy(
	instruction_t * __restrict const instruction,
	global_state_t * __restrict const state);

instruction_field_t * instruction_add_field(
	instruction_t * __restrict const instruction,
	global_state_t * __restrict const state,
	char const * __restrict const name,
	uint16_t const pos, uint8_t size);

void instruction_update_field_fill(
	instruction_t const * __restrict const instruction,
	instruction_field_fill_t * __restrict const field,
	uint8_t const pos, uint8_t const size);

bool instruction_remove_field(
	instruction_t * __restrict const instruction,
	instruction_field_t const * __restrict const field);

uint64_t instruction_field_fill_assemble(
	instruction_field_fill_t * __restrict const field,
	uint64_t instruction_bits,
	uint64_t field_value);

static void instruction_field_fill_setup(
	instruction_field_t * __restrict const field,
	global_state_t * __restrict const state,
	char const * __restrict const name,
	uint16_t const pos, uint16_t const size)
{
	field->id   = global_unique_id(state);
	field->type = instruction_field_type_fill;
	field->name_id = strings_add_c_string(state, name);
	instruction_field_fill_t * __restrict const fill_field =
		(instruction_field_fill_t * __restrict) field;
	fill_field->pos  = pos;
	fill_field->size = size;
}

void instruction_field_destroy(
	instruction_field_t * __restrict const field,
	global_state_t * __restrict const state);

uint64_t instruction_assemble(
	instruction_t * __restrict const instruction,
	global_state_t * __restrict const state,
	myy_vector_uint64 * __restrict const fields_values);

#endif
