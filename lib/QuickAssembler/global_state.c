#include "assembler.h"

#include <stdint.h>
#include <stdbool.h>

bool global_state_setup(
	global_state_t * __restrict const state,
	uint64_t start_id)
{
	state->id = start_id;
	state->collections = 
		myy_vector_instructions_collection_init(8);
	stringdb_memory_setup(&state->strings);

	return myy_vector_instructions_collection_is_valid(
		&state->collections);
}

uint64_t global_unique_id(
	global_state_t * __restrict const state)
{
	uint64_t const id = state->id + 1;
	state->id = id;
	return id;
}
