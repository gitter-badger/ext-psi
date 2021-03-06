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

#include <assert.h>

struct psi_decl_union* psi_decl_union_init(const char *name,
		struct psi_plist *args)
{
	struct psi_decl_union *u = calloc(1, sizeof(*u));
	u->name = strdup(name);
	u->args = args;
	return u;
}

void psi_decl_union_free(struct psi_decl_union **u_ptr)
{
	if (*u_ptr) {
		struct psi_decl_union *u = *u_ptr;

		*u_ptr = NULL;
		if (u->token) {
			free(u->token);
		}
		if (u->args) {
			psi_plist_free(u->args);
		}
		free(u->name);
		free(u);
	}
}

void psi_decl_union_dump(int fd, struct psi_decl_union *unn)
{
	dprintf(fd, "union %s::(%zu, %zu)", unn->name, unn->align, unn->size);
	if (psi_plist_count(unn->args)) {
		psi_decl_type_dump_args_with_layout(fd, unn->args, 0);
	} else {
		dprintf(fd, ";");
	}
}

struct psi_decl_arg *psi_decl_union_get_arg(struct psi_decl_union *u,
		struct psi_decl_var *var)
{
	if (u->args) {
		return psi_decl_arg_get_by_var(var, u->args, NULL);
	}

	return NULL;
}

bool psi_decl_union_validate(struct psi_data *data, struct psi_decl_union *u,
		struct psi_validate_stack *type_stack)
{
	size_t i, pos, len, size = 0, align;
	struct psi_decl_arg *darg;

	if (psi_validate_stack_has_union(type_stack, u->name)) {
		return true;
	}
	psi_validate_stack_add_union(type_stack, u->name, u);

	if (!u->size && !psi_plist_count(u->args)) {
		data->error(data, u->token, PSI_WARNING,
				"Cannot compute size of empty union %s", u->name);
		return false;
	}

	for (i = 0; psi_plist_get(u->args, i, &darg); ++i) {
		darg->var->arg = darg;

		if (!psi_decl_arg_validate(data, darg, type_stack)) {
			return false;
		}

		if (darg->layout && darg->layout->len) {
			pos = darg->layout->pos;

			align = psi_decl_arg_align(darg, &pos, &len);

			if (darg->layout->pos != 0) {
				data->error(data, darg->token, PSI_WARNING,
						"Offset of %s.%s should be 0", u->name,
						darg->var->name);
				darg->layout->pos = 0;
			}
			if (darg->layout->len != len) {
				data->error(data, darg->token, PSI_WARNING,
						"Computed size %zu of %s.%s does not match"
						" pre-defined size %zu of type '%s'",
						len, u->name, darg->var->name, darg->layout->len,
						darg->type->name);
			}
		} else {
			pos = 0;

			align = psi_decl_arg_align(darg, &pos, &len);

			if (darg->layout) {
				if (darg->layout->pos != 0) {
					data->error(data, darg->token, PSI_WARNING,
							"Offset of %s.%s should be 0", u->name,
							darg->var->name);
					darg->layout->pos = 0;
				}
				darg->layout->len = len;
			} else {
				darg->layout = psi_layout_init(pos, len, NULL);
			}
		}
		if (len > size) {
			size = len;
		}
		if (align > u->align) {
			u->align = align;
		}
	}

	psi_plist_sort(u->args, psi_layout_sort_cmp, NULL);

	if (u->size < size) {
		u->size = psi_align(size, u->align);
	}

	return true;
}

size_t psi_decl_union_get_align(struct psi_decl_union *u)
{
	if (!u->align) {
		u->align = psi_decl_type_get_args_align(u->args);
	}
	return u->align;
}
