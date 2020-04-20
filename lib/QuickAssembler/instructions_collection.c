#include "assembler.h"

#include <stdint.h>
#include <stdbool.h>

void instructions_collection_setup(
	instructions_collection_t * __restrict const added_collection,
	global_state_t * __restrict const state,
	char const * __restrict const name)
{
	added_collection->id = global_unique_id(state);
	added_collection->name_id = strings_add_c_string(state, name);
	added_collection->instructions = myy_vector_instruction_init(256);
}

instructions_collection_t * instructions_collections_add(
	myy_vector_instructions_collection * __restrict const collections,
	global_state_t * __restrict const state,
	char const * __restrict const name)
{
	instructions_collection_t * __restrict added_collection =
		NULL;
	{
		instructions_collection_t collection = {0};
		bool const ret = myy_vector_instructions_collection_add(
			collections, 1, &collection);
		if (ret == false)
			goto out;
	}

	added_collection = 
		myy_vector_instructions_collection_last(collections);
	instructions_collection_setup(added_collection, state, name);

out:
	return added_collection;
}

/* ??? */
instruction_t * instructions_add(
	myy_vector_instruction * __restrict const collection,
	global_state_t * __restrict const state,
	char const * __restrict const name,
	uint64_t const base_value)
{
	instruction_t * __restrict added_instruction =
		NULL;
	{
		instruction_t instruction = {0};
		bool const ret = myy_vector_instruction_add(
			collection, 1, &instruction);
		if (ret == false)
			goto out;
	}

	added_instruction = 
		myy_vector_instruction_last(collection);
	instruction_setup(added_instruction, state, name, base_value);

out:
	return added_instruction;
}

void instructions_collection_destroy(
	instructions_collection_t * __restrict const collection,
	global_state_t * __restrict const state)
{
	myy_vector_instruction * __restrict const instructions_vector =
		&collection->instructions;
		
	myy_vector_for_each_ptr(
        instruction_t, instruction, in, instructions_vector,
		{ instruction_destroy(instruction, state); });
	myy_vector_instruction_free_content(collection->instructions);
	strings_remove(state, collection->name_id);
}

void instructions_collections_destroy(
	myy_vector_instructions_collection * __restrict const collections,
	global_state_t * __restrict const state)
{
	myy_vector_for_each_ptr(
		instructions_collection_t, collection, in, collections,
		{ instructions_collection_destroy(collection, state); }
	);
	myy_vector_instructions_collection_free_content(*collections);
}
