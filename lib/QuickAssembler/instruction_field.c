#include "assembler.h"

static inline uint64_t instruction_field_fill_mask(
	instruction_field_fill_t * __restrict const field)
{
	return (uint64_t) ((1 << (field->size)) - 1);
}

uint64_t instruction_field_fill_assemble(
	instruction_field_fill_t * __restrict const field,
	uint64_t instruction_bits,
	uint64_t field_value)
{
	if (field->size == 0)
		return instruction_bits;

	uint64_t const mask = instruction_field_fill_mask(field);

	return (
		instruction_bits
		| ((field_value & mask) << field->pos)
	);
}

void instruction_field_destroy(
	instruction_field_t * __restrict const field,
	global_state_t * __restrict const state)
{
	strings_remove(state, field->name_id);
}
