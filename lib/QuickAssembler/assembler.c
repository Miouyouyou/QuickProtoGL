#include "assembler.h"

int main(int argc, char **argv) {
	bool ret = false;

	global_state_t state;
	bool const valid_collections = 
		global_state_setup(&state, 1);

	if (!valid_collections)
		goto invalid_collections;

	instructions_collection_t * __restrict const collection =
		instructions_collections_add(
			&state.collections, &state, "ARM");

	if (collection == NULL)
		goto invalid_collection;

	myy_vector_instruction * __restrict const instructions =
		&collection->instructions;
	instruction_t * __restrict const mov = instructions_add(
		instructions,
		&state,
		"MOV",
		0);
	if (mov == NULL)
		goto invalid_instruction;

	instruction_field_t * __restrict const src =
		instruction_add_field(mov, &state, "Register SRC", 4, 5);
	
	instruction_field_t * __restrict const dst =
		instruction_add_field(mov, &state, "Register DST", 13, 5);

	instruction_field_t * __restrict const shift =
		instruction_add_field(mov, &state, "Shift", 0, 2);

	uint64_t field_values[] = {27, 31, 3};
	myy_vector_uint64 arguments = myy_vector_uint64_init(4);
	myy_vector_uint64_add(
		&arguments,
		sizeof(field_values)/sizeof(uint64_t),
		field_values);
	uint64_t assembled_instruction =
		instruction_assemble(mov, &arguments);

	string_data_t name = strings_get(&state, mov->name_id);

	instruction_dump_infos_with_values(
		mov,
		&state,
		&arguments);

	LOG("\nAssembled instruction : %08" PRIx64 "\n", assembled_instruction);
	
	
	//instructions_remove(instructions, mov);
invalid_instruction:
	//collections_remove(collections, collection);
invalid_collection:
	//myy_vector_free(collections);
invalid_collections:
	return 0;
}


