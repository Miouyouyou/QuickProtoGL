#include "assembler.h"

#include <inttypes.h>

instruction_t instruction_new(
	global_state_t * __restrict const state,
	string_id_t const mnemonic)
{
	instruction_t instruction = {
		.id = global_unique_id(state),
		.name_id = mnemonic, /* The saved data will only be the ID. The string will be stored for display purpose only. */
		.fields = myy_vector_instruction_field_init(16)
	};

	return instruction;
}

void instruction_setup(
	instruction_t * __restrict const added_instruction,
	global_state_t * __restrict const state,
	char const * __restrict const name,
	uint64_t const base_value)
{
	added_instruction->id = global_unique_id(state);
	added_instruction->name_id = strings_add_c_string(state, name);
	added_instruction->fields = myy_vector_instruction_field_init(16);
	added_instruction->base_value = base_value;
}

void instruction_destroy(
	instruction_t * __restrict const instruction,
	global_state_t * __restrict const state)
{
	myy_vector_instruction_field * __restrict const
		fields_vector = &instruction->fields;
	myy_vector_for_each_ptr(
		instruction_field_t, instruction_field, in, fields_vector,
		{ instruction_field_destroy(instruction_field, state); }
	);
	myy_vector_instruction_field_free_content(instruction->fields);
	strings_remove(state, instruction->name_id);
}

instruction_field_t * instruction_add_field(
	instruction_t * __restrict const instruction,
	global_state_t * __restrict const state,
	char const * __restrict const name,
	uint16_t const pos, uint8_t size)
{
	/* Add to the vector first, ensuring we have enough space
	 * for the operations.
	 * If the field is added without issues :
	 * - Get back its address
	 * - Fill it accordingly
	 * - Return it
	 * 
	 */
	instruction_field_t * __restrict added_field = NULL;
	{
		instruction_field_t field = {0};
		bool const ret = myy_vector_instruction_field_add(
			&instruction->fields, 1, &field);
		if (ret == false)
			goto out;
	}

	added_field = myy_vector_instruction_field_last(&instruction->fields);
	instruction_field_fill_setup(added_field, state, name, pos, size);

out:
	return added_field;
}

void instruction_update_field_fill(
	instruction_t const * __restrict const instruction,
	instruction_field_fill_t * __restrict const field,
	uint8_t const pos, uint8_t const size)
{
	field->pos  = pos;
	field->size = size;
}

static bool cb_instruction_field_same_id(
	instruction_field_t const * __restrict const a,
	instruction_field_t const * __restrict const b)
{
	return a->id == b->id;
}

/* TODO Implement "Delete if" */
bool instruction_remove_field(
	instruction_t * __restrict const instruction,
	instruction_field_t const * __restrict const field)
{
	return myy_vector_instruction_field_delete_if(
		&instruction->fields,
		field,
		cb_instruction_field_same_id);
}

void instruction_dump_infos_with_values(
	instruction_t * __restrict const instruction,
	global_state_t * __restrict const state,
	myy_vector_uint64 * __restrict const fields_values)
{
	myy_vector_instruction_field * __restrict const fields_vector =
		(&instruction->fields);
	uint64_t const n_fields =
		myy_vector_instruction_field_length(fields_vector);
	uint64_t const n_values = 
		myy_vector_uint64_length(fields_values);

	if (n_values < n_fields) {
	LOG(
			"[%s]\n"
			"Expected at least %" PRIu64 " fields values but "
			"only received %" PRIu64 ".\n"
			"(%" PRIu64 " < %" PRIu64 ")\n",
			strings_get(state, instruction->name_id).text,
			n_fields, n_values,
			n_values, n_fields);
		return;
	}


	string_data_t get_result =
		strings_get(state, instruction->name_id);
	if (!string_data_valid(&get_result)) {
		LOG("Could not retrieve the name of the instruction !?\n");
		return;
	}


	printf("Instruction : %s\n", get_result.text);
	printf("Fields : %" PRIu64 "\n", n_fields);


	instruction_field_t const * __restrict field =
		myy_vector_instruction_field_data(fields_vector);
	uint64_t const * __restrict field_value =
		myy_vector_uint64_data(fields_values);

	for (uint64_t i = 0; i < n_fields; i++, field++, field_value++)
	{
		string_data_t field_name =
			strings_get(state, field->name_id);
		if (!string_data_valid(&field_name)) {
			LOG("Could not retrieve field %" PRIu64 " ID\n", i);
			return;
		}

		instruction_field_fill_t const * __restrict const fill_field =
			(instruction_field_fill_t const * __restrict)
			field;
		printf("\t[%02d:%02d] %s (0x%" PRIx64 ")\n",
			fill_field->pos+fill_field->size,
			fill_field->pos,
			field_name.text,
			*field_value);
	}

}

uint64_t instruction_assemble(
	instruction_t * __restrict const instruction,
	global_state_t * __restrict const state,
	myy_vector_uint64 * __restrict const fields_values)
{
	myy_vector_instruction_field * __restrict const
		fields_vector = &instruction->fields;

	uint64_t const n_fields =
		myy_vector_instruction_field_length(fields_vector);

	uint64_t const n_values = 
		myy_vector_uint64_length(fields_values);

	uint64_t assembled_instruction =
		instruction->base_value;

	if (n_values < n_fields) {
		LOG(
			"[%s]\n"
			"Expected at least %" PRIu64 " arguments but "
			"received only %" PRIu64 " arguments.\n"
			"(%" PRIu64 " < %" PRIu64 ")\n",
			strings_get(state, instruction->name_id).text,
			n_fields, n_values,
			n_values, n_fields);
		return instruction->base_value;
	}

	instruction_field_t const * __restrict current_field =
		myy_vector_instruction_field_data(fields_vector);
	uint64_t const * __restrict current_field_value =
		myy_vector_uint64_data(fields_values);

	for (uint64_t f = 0; f < n_fields; f++, current_field++, current_field_value++)
	{
		switch(current_field->type) {
			case instruction_field_type_fill:
				assembled_instruction = instruction_field_fill_assemble(
					(instruction_field_fill_t * )current_field,
					assembled_instruction,
					*current_field_value);
				break;
			case instruction_field_type_invalid:
			case instruction_field_type_count:
			default:
				LOG(
					"Field %" PRIu64 " has unexpected type :\n"
					"%" PRIu32 "\n",
					f,
					current_field->type);
				break;
		}
	}

	return assembled_instruction;
	
}
