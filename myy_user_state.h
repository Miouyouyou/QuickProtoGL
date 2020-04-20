#ifndef MYY_USER_STATE_H
#define MYY_USER_STATE_H 1

#include <stdint.h>
#include <stdbool.h>

#include "myy/helpers/fonts/packed_fonts_parser.h"

typedef void (*myy_draw_func_ptr)(
	void * __restrict const arg,
	myy_states * __restrict const states,
	uint32_t const i);
struct myy_draw_function {
	myy_draw_func_ptr func;
	void * __restrict const arg;
};
typedef struct myy_draw_function myy_draw_function_t;

myy_vector_template(draw_functions, struct myy_draw_function);

struct myy_draw_functions {
	myy_vector_draw_functions functions;
	uint64_t padding;
};


__attribute__((unused))
static inline void myy_draw_functions_draw(
	myy_vector_draw_functions * __restrict const functions,
	myy_states * __restrict const states,
	uint32_t const i)
{
	myy_vector_for_each_ptr(
		myy_draw_function_t, draw_func, in, functions,
		{
			draw_func->func(draw_func->arg, states, i);
		}
	);
}
__attribute__((unused))
static inline void myy_draw_functions_add(
	myy_vector_draw_functions * __restrict const functions,
	myy_draw_func_ptr func,
	void * __restrict const arg)
{
	struct myy_draw_function const draw_func = {func, arg};
	myy_vector_draw_functions_add(functions, 1, &draw_func);
}

__attribute__((unused))
static inline bool myy_draw_function_equal_by_funcptr(
	struct myy_draw_function const * __restrict const stored_func,
	struct myy_draw_function const * __restrict const compared_func)
{
	return stored_func->func == compared_func->func;
}

static inline void myy_draw_functions_remove_by_funcptr(
	myy_vector_draw_functions * __restrict const functions,
	myy_draw_func_ptr func)
{
	struct myy_draw_function compared_function = { func, (void *) 0 };
	myy_vector_draw_functions_delete_if(functions, &compared_function,
		myy_draw_function_equal_by_funcptr);
}

typedef void (*myy_user_state_edited_callback)(
	void * callback_arg,
	myy_states * __restrict const states,
	char const * __restrict const text);

struct myy_user_state {
	myy_vector_draw_functions draw_functions;
	gl_text_atlas_t gl_text_atlas;
	struct {
		void * callback_arg;
		myy_user_state_edited_callback done_callback;
	} edited;
	bool editing;
};

typedef struct myy_user_state myy_user_state_t;

__attribute__((unused))
static inline
myy_user_state_t * myy_user_state_from(
	myy_states * __restrict const states)
{
	return (myy_user_state_t *) (states->user_state);
}

__attribute__((unused))
static inline
struct gl_text_infos * gl_text_atlas_addr_from(
	myy_states * __restrict const states)
{
	myy_user_state_t * __restrict const ustate =
		myy_user_state_from(states);
	return &ustate->gl_text_atlas;
}

__attribute__((unused))
static inline
bool myy_states_currently_editing(
	myy_states * __restrict const states)
{
	return (myy_user_state_from(states)->editing);
}

__attribute__((unused))
static inline
void myy_states_text_edit_module_deal_with(
	myy_states * __restrict const states,
	char const * __restrict const text)
{
	if (myy_states_currently_editing(states)) {
		myy_user_state_t * __restrict const ustate =
			myy_user_state_from(states);
		myy_user_state_edited_callback callback =
			ustate->edited.done_callback;
		if (callback) {
			ustate->edited.done_callback(
				ustate->edited.callback_arg,
				states,
				text);
		}
	}
}

__attribute__((unused))
static inline
gl_text_atlas_t * myy_states_gl_text_atlas_addr(
	myy_states * __restrict const states)
{
	return &(myy_user_state_from(states)->gl_text_atlas);
}

#endif
