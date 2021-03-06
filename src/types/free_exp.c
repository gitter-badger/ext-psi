/*******************************************************************************
 Copyright (c) 2016, Michael Wallner <mike@php.net>.
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
 this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************/

#include "php_psi_stdinc.h"
#include "data.h"
#include "call.h"

struct psi_free_exp *psi_free_exp_init(const char *func, struct psi_plist *vars)
{
	struct psi_free_exp *f = calloc(1, sizeof(*f));
	f->func = strdup(func);
	f->vars = vars;
	return f;
}

void psi_free_exp_free(struct psi_free_exp **f_ptr)
{
	if (*f_ptr) {
		struct psi_free_exp *f = *f_ptr;

		*f_ptr = NULL;
		if (f->token) {
			free(f->token);
		}
		free(f->func);
		psi_plist_free(f->vars);
		if (f->let) {
			free(f->let);
		}
		free(f);
	}
}

void psi_free_exp_dump(int fd, struct psi_free_exp *call)
{
	size_t l = 0, c = psi_plist_count(call->vars);
	struct psi_decl_var *fvar;

	dprintf(fd, "%s(", call->func);
	while (psi_plist_get(call->vars, l++, &fvar)) {
		psi_decl_var_dump(fd, fvar);
		if (l < c) {
			dprintf(fd, ", ");
		}
	}
	dprintf(fd, ")");
}

static inline struct psi_decl *locate_free_decl(struct psi_plist *decls,
		struct psi_free_exp *f)
{
	if (decls) {
		size_t i = 0;
		struct psi_decl *decl;

		while (psi_plist_get(decls, i++, &decl)) {
			if (!strcmp(decl->func->var->name, f->func)) {
				return f->decl = decl;
			}
		}
	}

	return NULL;
}

bool psi_free_exp_validate(struct psi_data *data, struct psi_free_exp *exp,
		struct psi_impl *impl)
{
	size_t i;
	struct psi_decl_var *free_var;

	/* first find the decl of the free func */
	if (!locate_free_decl(data->decls, exp)) {
		data->error(data, exp->token, PSI_WARNING,
				"Missing declaration '%s' in `free` statement"
				" of implementation '%s'", exp->func, impl->func->name);
		return false;
	}

	/* now check for known vars */
	exp->let = calloc(psi_plist_count(exp->vars), sizeof(*exp->let));
	for (i = 0; psi_plist_get(exp->vars, i, &free_var); ++i) {
		if (!psi_decl_arg_get_by_var(free_var, impl->decl->args,
				impl->decl->func)) {
			data->error(data, free_var->token, PSI_WARNING,
					"Unknown variable '%s' of `free` statement"
					" of implementation '%s'",
					free_var->name, impl->func->name);
			return false;
		}

		exp->let[i] = psi_impl_get_let(impl, free_var);
		assert(exp->let[i]);
	}

	return true;
}

void psi_free_exp_exec(struct psi_free_exp *f, struct psi_call_frame *frame)
{
	size_t i;
	void **args;
	struct psi_decl_var *dvar;
	struct psi_call_frame *free_call;
	struct psi_context *ctx = psi_call_frame_get_context(frame);

	free_call = psi_call_frame_init(ctx, f->decl, NULL);
	psi_call_frame_enter(free_call);

	args = psi_call_frame_get_arg_pointers(free_call);
	for (i = 0; psi_plist_get(f->vars, i, &dvar); ++i) {
		struct psi_call_frame_symbol *frame_sym;
		struct psi_let_exp *let = f->let[i]->exp;

		frame_sym = psi_call_frame_fetch_symbol(frame, let->var);
		args[i] = deref_impl_val(frame_sym->ptr, dvar);
	}

	psi_call_frame_do_call(free_call);
	psi_call_frame_free(free_call);
}
