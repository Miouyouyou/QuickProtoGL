/* L'affectation d'ID unique peut poser problème pour l'export.
 * -> Durant l'export, quel serait les problèmes causés par la
 *    réafffectation de nouveaux ID ?
 *    -> Impossibilité de fournir du code basé sur ce set
 *       d'instructions -> Problèmes de coopération.
 *       -> Utilisation d'un ID local et d'un SHA256 permettrait
 *          d'assurer une meilleure coopération... ?
 */

struct instruction {
	uint64_t id;
	string_id_t name_id; // To change
	uint64_t base_value;
	myy_vector_instruction_field fields;
};

struct instruction_field {
	uint64_t id;
	string_id_t name_id; // To replace by a string id
	uint32_t type;
	uint8_t infos[12];
};

Currently_edited:
	instruction_t instruction
	hitbox_actions_t hitboxes;
	

void instruction_editor_init_new(
	struct menu * __restrict const menu)
{
	menu->state =
		editor_state_s(instruction_new_unique(global_state_ref()));
	hitbox_actions_init(hitboxes);
	
}

void instruction_editor_
