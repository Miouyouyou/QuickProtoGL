#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#include <stdarg.h>

#include <sys/mman.h>

#include "global_include.h"

/**
 * List widgets :
 * - myy_text_area : Text input widget
 *   * Editing is done with an editor module.
 *     The whole idea is that the module implementation
 *     differs from platform to platform.
 *     (e.g. Android text module editor is completely
 *      different from the X11 one).
 * -
 */


struct myy_shaders_db myy_programs;
struct myy_user_state state;

int myy_init(
	myy_states * __restrict const states,
	int argc,
	char **argv,
	struct myy_window_parameters * __restrict const window)
{
	states->user_state = &state;
	window->height = 900;
	window->width  = 1600;
	window->title  = "Meow";

	return 0;
};

static void init_text_atlas(
	myy_states * __restrict states,
	uintreg_t surface_width,
	uintreg_t surface_height)
{
	myy_shaders_pack_load_all_programs_from_file(
		"data/shaders.pack",
		(uint8_t * __restrict) &myy_programs);
	LOG("myy_programs location : %p\n", &myy_programs);


	struct gl_text_infos * __restrict const loaded_infos =
		gl_text_atlas_addr_from(states);
	
	struct myy_sampler_properties properties =
		myy_sampler_properties_default();

	glActiveTexture(GL_TEXTURE4);
	myy_packed_fonts_load(
		"data/font_pack_meta.dat", loaded_infos, NULL, &properties);

	glClearColor(0.2f, 0.5f, 0.8f, 1.0f);

	float inv_tex_width  = 1.0f/loaded_infos->tex_width;
	float inv_tex_height = 1.0f/loaded_infos->tex_height;

	glUseProgram(myy_programs.text_id);
	glUniform1i(
		myy_programs.text_unif_fonts_texture,
		4);
	glUniform2f(
		myy_programs.text_unif_texture_projection,
		inv_tex_width,
		inv_tex_height);

	glEnableVertexAttribArray(text_xy);
	glEnableVertexAttribArray(text_in_st);

}

static void init_projections(
	myy_states * __restrict state,
	uintreg_t surface_width,
	uintreg_t surface_height)
{

	union myy_4x4_matrix matrix;
	myy_matrix_4x4_ortho_layered_window_coords(
		&matrix, surface_width, surface_height, 64);

	glUseProgram(myy_programs.text_id);
	glUniformMatrix4fv(
		myy_programs.text_unif_projection,
		1,
		GL_FALSE,
		matrix.raw_data);
	glUseProgram(myy_programs.lines_id);
	glUniformMatrix4fv(
		myy_programs.lines_unif_projection,
		1,
		GL_FALSE,
		matrix.raw_data);
	glUseProgram(0);
}

myy_vector_template(int16, int16_t)

static GLuint lines_buffer;
static uint32_t n_lines;
static int32_t offset_x = 0, offset_y = 0;
static int32_t offset_current_x = 0, offset_current_y = 0;

static void lines_init(
	myy_states * __restrict state,
	uintreg_t const surface_width,
	uintreg_t const surface_height)
{
	myy_vector_int16 coords = myy_vector_int16_init(512);
	{
		int16_t line_left[2]  = {-128, 0};
		int16_t line_right[2] = {(int16_t) (surface_width+128), 0};
		int16_t const limit = surface_height+128;
		for (int16_t i = -128; i < limit; i += 32)
		{
			line_left[1]  = i; // { -128, i }
			line_right[1] = i; // { surface_width+128, i }
			myy_vector_add(&coords, 2, line_left);
			myy_vector_add(&coords, 2, line_right);
		}
		LOG("n_elements after lines : %zu\n",
			myy_vector_int16_length(&coords));
	}
	{
		int16_t column_up[2]   = {0, -128};
		int16_t column_down[2] = {0, (int16_t) (surface_height+128)};
		int16_t const limit = surface_width+128;
		for (int16_t i = -128; i < limit; i += 32)
		{
			column_up[0]   = i;
			column_down[0] = i;
			myy_vector_add(&coords, 2, column_up);
			myy_vector_add(&coords, 2, column_down);
		}
		LOG("n_elements after lines : %zu\n",
			myy_vector_length(&coords));
	}

	/* Infinite lines */
	LOG("n_elements : %zu (%zu lines)\n",
		myy_vector_length(&coords),
		myy_vector_length(&coords)/2);
	glGenBuffers(1, &lines_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, lines_buffer);
	glBufferData(GL_ARRAY_BUFFER,
		myy_vector_allocated_used(&coords),
		myy_vector_data(&coords),
		GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	n_lines = myy_vector_length(&coords) / 2;

	myy_vector_free_content(coords);
	glUseProgram(myy_programs.lines_id);
	glEnableVertexAttribArray(lines_xy);
	glUseProgram(0);
}

static void lines_draw()
{
	glUseProgram(myy_programs.lines_id);
	glBindBuffer(GL_ARRAY_BUFFER, lines_buffer);
	glUniform2f(myy_programs.lines_unif_global_offset,
		(float) ((offset_x + offset_current_x) & 31),
		(float) ((offset_y + offset_current_y) & 31));
	glVertexAttribPointer(
		lines_xy,
		2,
		GL_SHORT,
		GL_FALSE,
		2*sizeof(int16_t),
		(void *) (0x0));
	glDrawArrays(GL_LINES, 0, n_lines);
	glUseProgram(0);
}

void myy_init_drawing(
	myy_states * __restrict state,
	uintreg_t const surface_width,
	uintreg_t const surface_height)
{
    LOG("Init drawing");
	init_text_atlas(state, surface_width, surface_height);
	init_projections(state, surface_width, surface_height);

	LOG("width : %lu, height : %lu\n", surface_width, surface_height);
	lines_init(state, surface_width, surface_height);

	glEnable(GL_DEPTH_TEST);
}

void myy_draw(
	myy_states * __restrict state,
	uintreg_t i,
	uint64_t last_frame_delta_ns)
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);

	myy_draw_functions_draw(
		&myy_user_state_from(state)->draw_functions,
		state,
		i);
	lines_draw();
}


/* Question : How does that work, from the first interaction ? */
void myy_editor_finished(
	myy_states * __restrict const states,
	uint8_t const * __restrict const string,
	size_t const string_size)
{
	/* Still ugly, IMHO */
	myy_user_state_t * __restrict const user_state =
		myy_user_state_from(states);
	myy_states_text_edit_module_deal_with(states, (char *) string);
	user_state->editing = false;
}

void myy_input(
	myy_states * __restrict state,
	enum myy_input_events const event_type,
	union myy_input_event_data * __restrict const event)
{
	static uint32_t moving = 0;
	static uint32_t start_x = 0, start_y = 0;
	switch(event_type) {
		case myy_input_event_invalid:
			break;
		case myy_input_event_mouse_moved_absolute:
			if (moving) {
				offset_current_x = (event->mouse_move_absolute.x - start_x);
				offset_current_y = (event->mouse_move_absolute.y - start_y);
			}
			break;
		case myy_input_event_mouse_moved_relative:
			break;
		case myy_input_event_mouse_button_pressed:
			if (event->mouse_button.button_number == 1) {
				start_x = event->mouse_button.x;
				start_y = event->mouse_button.y;
				moving = 1;
			}
			break;
		case myy_input_event_mouse_button_released:
			if (event->mouse_button.button_number == 1) {
				moving = 0;
				offset_x += offset_current_x;
				offset_y += offset_current_y;
				offset_current_x = 0;
				offset_current_y = 0;
			}
			break;
		case myy_input_event_touch_pressed:
			start_x = event->touch.x;
			start_y = event->touch.y;
			moving = 1;
			break;
		case myy_input_event_touch_move:
			if (moving) {
				offset_current_x = (event->touch.x - start_x);
				offset_current_y = (event->touch.y - start_y);
			}
			break;
		case myy_input_event_touch_released:
			moving = 0;
			offset_x += (event->touch.x - start_x);
			offset_y += (event->touch.y - start_y);
			offset_current_x = 0;
			offset_current_y = 0;
			break;
		case myy_input_event_keyboard_key_released:
			break;
		case myy_input_event_keyboard_key_pressed:
			LOG("KEY: %d\n", event->key.raw_code);
			if (event->key.raw_code == 1) { myy_user_quit(state); }
			break;
		/* TODO We need a myy_input_event_chars_received to differentiate
		 * an entire text block to set in a buffer
		 * from chars to append to a buffer.
		 */
		case myy_input_event_text_received:
			LOG("TEXT: %s\n", event->text.data);
			//myy_states_text_edit_module_deal_with(state, event->text.data);
			break;
		case myy_input_event_editor_finished:
			LOG("Editor finished with : %s !\n", event->text.data);
			myy_editor_finished(state,
				(uint8_t const *) event->text.data,
				event->text.length);
		case myy_input_event_surface_size_changed:
			myy_display_initialised(
				state,
				event->surface.width, event->surface.height);
			break;
		case myy_input_event_window_destroyed:
			myy_user_quit(state);
			break;
		default:
			break;
	}
}
