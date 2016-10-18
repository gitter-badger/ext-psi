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

#ifdef HAVE_CONFIG_H
# include "config.h"
#else
# include "php_config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "data.h"

impl_var *init_impl_var(const char *name, int is_reference) {
	impl_var *var = calloc(1, sizeof(*var));
	var->name = strdup(name);
	var->reference = is_reference;
	return var;
}

impl_var *copy_impl_var(impl_var *var) {
	impl_var *cpy = malloc(sizeof(*cpy));
	memcpy(cpy, var, sizeof(*cpy));
	cpy->name = strdup(cpy->name);
	if (cpy->token) {
		cpy->token = psi_token_copy(cpy->token);
	}
	return cpy;
}

void free_impl_var(impl_var *var) {
	if (var->token) {
		free(var->token);
	}
	free(var->name);
	free(var);
}

impl_arg *locate_impl_var_arg(impl_var *var, impl_args *args) {
	size_t i;

	for (i = 0; i < args->count; ++i) {
		impl_arg *iarg = args->args[i];

		if (!strcmp(var->name, iarg->var->name)) {
			return var->arg = iarg;
		}
	}

	return NULL;
}