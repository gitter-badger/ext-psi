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

struct psi_decl_abi *psi_decl_abi_init(const char *convention)
{
	struct psi_decl_abi *abi = calloc(1, sizeof(*abi));
	abi->convention = strdup(convention);
	return abi;
}

void psi_decl_abi_free(struct psi_decl_abi **abi_ptr)
{
	if (*abi_ptr) {
		struct psi_decl_abi *abi = *abi_ptr;

		*abi_ptr = NULL;
		if (abi->token) {
			free(abi->token);
		}
		free(abi->convention);
		free(abi);
	}
}

void psi_decl_abi_dump(int fd, struct psi_decl_abi *abi)
{
	dprintf(fd, "%s", abi->convention);
}

static const char * const abi_ccs[] = {
		"default", /* \                 */
		"extern",  /*  > - all the same */
		"cdecl",   /* /                 */
		"stdcall",
		"fastcall",
};

bool psi_decl_abi_validate(struct psi_data *data, struct psi_decl_abi *abi)
{
	size_t i;

	for (i = 0; i < sizeof(abi_ccs) / sizeof(char *); ++i) {
		if (strcasecmp(abi->convention, abi_ccs[i])) {
			return true;
		}
	}
	return false;
}
