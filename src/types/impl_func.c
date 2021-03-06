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

struct psi_impl_func *psi_impl_func_init(const char *name,
		struct psi_plist *args, struct psi_impl_type *type)
{
	struct psi_impl_func *func = calloc(1, sizeof(*func));

	func->name = strdup(name);
	func->args = args ? : psi_plist_init((psi_plist_dtor) psi_impl_arg_free);

	func->return_type = type;

	return func;
}

void psi_impl_func_free(struct psi_impl_func **f_ptr)
{
	if (*f_ptr) {
		struct psi_impl_func *f = *f_ptr;

		*f_ptr = NULL;
		if (f->token) {
			free(f->token);
		}

		psi_impl_type_free(&f->return_type);
		psi_plist_free(f->args);

		if (f->vararg) {
			psi_impl_arg_free(&f->vararg);
		}

		free(f->name);
		free(f);
	}
}

bool psi_impl_func_validate(struct psi_data *data, struct psi_impl_func *func)
{
	int def = 0;
	size_t i = 0;
	struct psi_impl_arg *iarg;

	while (psi_plist_get(func->args, i++, &iarg)) {
		if (iarg->def) {
			def = 1;
			if (!psi_impl_def_val_validate(data, iarg->def, iarg->type->type, iarg->type->name)) {
				return 0;
			}
		} else if (def) {
			data->error(data, func->token, PSI_WARNING,
					"Non-optional argument %zu '$%s' of implementation '%s'"
					" follows optional argument",
					i + 1,
					iarg->var->name, func->name);
			return false;
		}
	}

	return true;
}

void psi_impl_func_dump(int fd, struct psi_impl_func *func)
{
	dprintf(fd, "function %s(", func->name);
	if (func->args) {
		size_t i = 0;
		struct psi_impl_arg *iarg;

		while (psi_plist_get(func->args, i++, &iarg)) {
			if (i > 1) {
				dprintf(fd, ", ");
			}
			psi_impl_arg_dump(fd, iarg, false);
		}

		if (func->vararg) {
			dprintf(fd, ", ");
			psi_impl_arg_dump(fd, func->vararg, true);
		}
	}
	dprintf(fd, ") : %s%s", func->return_reference ? "&" : "",
			func->return_type->name);
}
