/*******************************************************************************
 Copyright (c) 2017, Michael Wallner <mike@php.net>.
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

#include "cpp.h"
#include "parser.h"

#if PSI_CPP_DEBUG > 1
void psi_cpp_tokiter_dump(int fd, struct psi_cpp *cpp)
{
	size_t i = cpp->index;
	struct psi_token *T;

	if (i > 20) {
		i -= 20;
	} else {
		i = 0;
	}
	while (psi_plist_get(cpp->tokens, i, &T)) {
		dprintf(fd, "PSI: CPP tokens %5zu %c ", i, cpp->index == i ? '*' : ' ');
		psi_token_dump(fd, T);
		if (i >= cpp->index + 10) {
			dprintf(fd, "PSI: CPP tokens .....\n");
			break;
		}
		++i;
	}
}
#endif

void psi_cpp_tokiter_reset(struct psi_cpp *cpp)
{
#if PSI_CPP_DEBUG
	fprintf(stderr, "PSI: CPP reset (%zu tokens)\n", psi_plist_count(cpp->tokens));
#	if PSI_CPP_DEBUG > 1
	psi_cpp_tokiter_dump(2, cpp);
#	endif
#endif
	cpp->index = 0;
	cpp->expanded = 0;
}

bool psi_cpp_tokiter_seek(struct psi_cpp *cpp, size_t index)
{
	if (index < psi_plist_count(cpp->tokens)) {
		cpp->index = index;
		return true;
	}
	return false;
}

struct psi_token *psi_cpp_tokiter_current(struct psi_cpp *cpp)
{
	struct psi_token *current = NULL;
	bool found = psi_plist_get(cpp->tokens, cpp->index, &current);

	assert(found);

	return current;
}

size_t psi_cpp_tokiter_index(struct psi_cpp *cpp)
{
	return cpp->index;
}

void psi_cpp_tokiter_next(struct psi_cpp *cpp)
{
#if 0 && PSI_CPP_DEBUG
	fprintf(stderr, "PSI: CPP next -> index=%zu -> index=%zu\n", cpp->index, cpp->index+1);
#endif
	++cpp->index;
}

void psi_cpp_tokiter_prev(struct psi_cpp *cpp)
{
#if 0 && PSI_CPP_DEBUG
	fprintf(stderr, "PSI: CPP prev -> index=%zu -> index=%zu\n", cpp->index, cpp->index-1);
#endif
	if (cpp->index) {
		--cpp->index;
	}
}

bool psi_cpp_tokiter_valid(struct psi_cpp *cpp)
{
#if 0 && PSI_CPP_DEBUG
	fprintf(stderr, "PSI: CPP valid -> index=%zu -> %d\n", cpp->index, cpp->index < psi_plist_count(cpp->tokens));
#endif
	return cpp->index < psi_plist_count(cpp->tokens);
}

bool psi_cpp_tokiter_del_cur(struct psi_cpp *cpp, bool free_token)
{
	struct psi_token *cur = NULL;
	bool deleted = psi_plist_del(cpp->tokens, cpp->index, &cur);
	size_t count;

#if PSI_CPP_DEBUG
	fprintf(stderr, "PSI: CPP del_cur -> index=%zu, del=%d, free=%d, count=%zu ",
			cpp->index, (int) deleted, (int) free_token, psi_plist_count(cpp->tokens));
	if (cur) {
		psi_token_dump(2, cur);
	} else {
		fprintf(stderr, "NULL\n");
	}
#endif
	if (cur && free_token) {
		free(cur);
	}
	count = psi_plist_count(cpp->tokens);
	if (deleted && cpp->index >= count) {
		if (count > 0) {
			cpp->index = count - 1;
		} else {
			cpp->index = 0;
		}
	}
	return deleted;
}

bool psi_cpp_tokiter_del_range(struct psi_cpp *cpp, size_t offset, size_t num_eles, bool free_tokens)
{
	struct psi_token **ptr;
	bool deleted;

	if (free_tokens) {
		ptr = calloc(num_eles, sizeof(*ptr));
	} else {
		ptr = NULL;
	}

#if PSI_CPP_DEBUG
	fprintf(stderr, "PSI: CPP del_range -> index=%zu, offset=%zu, num_eles=%zu, count=%zu\n",
			cpp->index, offset, num_eles, psi_plist_count(cpp->tokens));
#endif

	deleted = psi_plist_del_r(cpp->tokens, offset, num_eles, (void *) ptr);

	if (deleted) {
		size_t count = psi_plist_count(cpp->tokens);

		if (cpp->index >= count) {
			if (count > 0) {
				cpp->index = count - 1;
			} else {
				cpp->index = 0;
			}
		}

		if (free_tokens) {
			while (num_eles--) {
				if (ptr[num_eles]) {
					free(ptr[num_eles]);
				}
			}
			free(ptr);
		}
	}
	return deleted;
}

bool psi_cpp_tokiter_ins_cur(struct psi_cpp *cpp, struct psi_token *tok)
{
	struct psi_plist *tokens = psi_plist_ins(cpp->tokens, cpp->index, &tok);

#if PSI_CPP_DEBUG
	fprintf(stderr, "PSI: CPP ins_cur -> index=%zu ", cpp->index);
	psi_token_dump(2, tok);
#endif

	if (!tokens) {
		return false;
	}
	cpp->tokens = tokens;
	return true;
}

bool psi_cpp_tokiter_ins_range(struct psi_cpp *cpp, size_t offset,
		size_t num_eles, void **eles)
{
	struct psi_plist *tokens;

	if (!num_eles) {
		return true;
	}

	tokens = psi_plist_ins_r(cpp->tokens, offset, num_eles, eles);

#if PSI_CPP_DEBUG
	fprintf(stderr, "PSI: CPP ins_range -> index=%zu, offset=%zu, num_eles=%zu, count=%zu\n",
			cpp->index, offset, num_eles, psi_plist_count(tokens));
#endif

	if (!tokens) {
		return false;
	}
	cpp->tokens = tokens;
	return true;
}

bool psi_cpp_tokiter_defined(struct psi_cpp *cpp)
{
	if (psi_cpp_tokiter_valid(cpp)) {
		struct psi_token *current = psi_cpp_tokiter_current(cpp);

		return psi_cpp_defined(cpp, current);
	}

	return false;
}

static size_t psi_cpp_tokiter_expand_tokens(struct psi_cpp *cpp,
		struct psi_token *target, struct psi_plist *tokens)
{
	if (tokens && psi_plist_count(tokens)) {
		size_t i = 0, n = 0;
		bool stringify = false, paste = false;
		struct psi_token *tok, **exp_tokens = calloc(psi_plist_count(tokens), sizeof(*exp_tokens));

		while (psi_plist_get(tokens, i++, &tok)) {
			struct psi_token *new_tok;

			if (tok->type == PSI_T_EOL) {
				continue;
			}
			if (tok->type == PSI_T_HASH) {
				stringify = true;
				continue;
			}
			if (tok->type == PSI_T_CPP_PASTE) {
				paste = true;
				continue;
			}

			if (paste && n > 0 && exp_tokens[n - 1]) {
				struct psi_token *tmp_tok, *old_tok = exp_tokens[n - 1];

				tmp_tok = psi_token_init(old_tok->type, "", 0,
						target->col, target->line, target->file?:"");

				new_tok = psi_token_cat(NULL, 3, tmp_tok, old_tok, tok);
				free(old_tok);
				free(tmp_tok);

				exp_tokens[n - 1] = new_tok;
			} else {
				new_tok = psi_token_init(stringify ? PSI_T_QUOTED_STRING : tok->type,
						tok->text, tok->size, target->col, target->line, target->file?:"");

				exp_tokens[n++] = new_tok;
			}

#if PSI_CPP_DEBUG
			fprintf(stderr, "PSI: CPP expand > ");
			psi_token_dump(2, tok);
#endif

			paste = false;
			stringify = false;
		}
		psi_cpp_tokiter_ins_range(cpp, psi_cpp_tokiter_index(cpp), n, (void *) exp_tokens);
		free(exp_tokens);

		return n;
	} else {
		return 0;
	}
}

static void psi_cpp_tokiter_free_call_tokens(struct psi_plist **arg_tokens_list, size_t arg_count, bool free_tokens)
{
	size_t i;

	for (i = 0; i < arg_count; ++i) {
		if (arg_tokens_list[i]) {
			if (free_tokens) {
				struct psi_token *tok;

				while (psi_plist_pop(arg_tokens_list[i], &tok)) {
					free(tok);
				}
			}
			psi_plist_free(arg_tokens_list[i]);
		}
	}
	free(arg_tokens_list);
}

static struct psi_plist **psi_cpp_tokiter_read_call_tokens(
		struct psi_cpp *cpp, size_t arg_count)
{
	size_t arg_index = 0, lparens = 1, rparens = 0;
	struct psi_plist **arg_tokens = calloc(arg_count, sizeof(*arg_tokens));
	struct psi_plist *free_tokens = psi_plist_init((psi_plist_dtor) psi_token_free);
	struct psi_token *tok;

	arg_tokens[0] = psi_plist_init(NULL);

	/* next token must be a LPAREN for a macro call */
	psi_cpp_tokiter_next(cpp);
	tok = psi_cpp_tokiter_current(cpp);
	if (!psi_cpp_tokiter_valid(cpp) || tok->type != PSI_T_LPAREN) {
		goto fail;
	}

	/* free LPAREN on success */
	free_tokens = psi_plist_add(free_tokens, &tok);

	while (lparens > rparens) {
		psi_cpp_tokiter_next(cpp);
		if (!psi_cpp_tokiter_valid(cpp)) {
			goto fail;
		}
		tok = psi_cpp_tokiter_current(cpp);

		switch (tok->type) {
		case PSI_T_LPAREN:
			++lparens;
			arg_tokens[arg_index] = psi_plist_add(arg_tokens[arg_index], &tok);
			break;
		case PSI_T_RPAREN:
			if (++rparens == lparens) {
				/* closing RPAREN */
				if (arg_index + 1 < arg_count) {
					goto fail;
				}
				free_tokens = psi_plist_add(free_tokens, &tok);
			} else {
				arg_tokens[arg_index] = psi_plist_add(arg_tokens[arg_index], &tok);
			}
			break;
		case PSI_T_COMMA:
			if (1 == (lparens - rparens)) {
				/* too many commas? */
				if (++arg_index >= arg_count) {
					goto fail;
				}
				free_tokens = psi_plist_add(free_tokens, &tok);
				/* next arg */
				arg_tokens[arg_index] = psi_plist_init(NULL);
			} else {
				arg_tokens[arg_index] = psi_plist_add(arg_tokens[arg_index], &tok);
			}
			break;
		default:
			arg_tokens[arg_index] = psi_plist_add(arg_tokens[arg_index], &tok);
		}
	}

	psi_plist_free(free_tokens);
	return arg_tokens;

fail:
	psi_cpp_tokiter_free_call_tokens(arg_tokens, arg_count, false);
	return NULL;
}

static void psi_cpp_tokiter_expand_call_tokens(struct psi_cpp *cpp,
		struct psi_token *target, struct psi_cpp_macro_decl *macro,
		struct psi_plist **arg_tokens_list)
{
	size_t i;
	struct psi_token *tok;
	struct psi_plist *tokens = psi_plist_init(NULL);

	for (i = 0; psi_plist_get(macro->tokens, i, &tok); ++i) {
		struct psi_plist *arg_tokens = NULL;

		if (tok->type == PSI_T_NAME) {
			size_t s;
			struct psi_token *arg_name;

			for (s = 0; psi_plist_get(macro->sig, s, &arg_name); ++s) {
				if (arg_name->size == tok->size) {
					if (!memcmp(arg_name->text, tok->text, tok->size)) {
						arg_tokens = arg_tokens_list[s];
						break;
					}
				}
			}
		}

		if (arg_tokens) {
			tokens = psi_plist_add_r(tokens, psi_plist_count(arg_tokens), psi_plist_eles(arg_tokens));
		} else {
			tokens = psi_plist_add(tokens, &tok);
		}
	}

	psi_cpp_tokiter_expand_tokens(cpp, target, tokens);
	psi_plist_free(tokens);
}

static bool psi_cpp_tokiter_expand_call(struct psi_cpp *cpp,
		struct psi_token *target, struct psi_cpp_macro_decl *macro)
{
	/* function-like macro
	 * #define FOO(a,b) a>b // macro->sig == {a, b}, macro->tokens = {a, >, b}
	 * # if FOO(1,2) // expands to if 1 > 2
	 */
	size_t start = psi_cpp_tokiter_index(cpp);
	struct psi_plist **arg_tokens_list;

	/* read in tokens, until we have balanced parens */
	arg_tokens_list = psi_cpp_tokiter_read_call_tokens(cpp, psi_plist_count(macro->sig));
	if (!arg_tokens_list) {
		psi_cpp_tokiter_seek(cpp, start);
		return false;
	}

	/* ditch arg tokens */
	psi_cpp_tokiter_del_range(cpp, start, psi_cpp_tokiter_index(cpp) - start + 1, false);
	psi_cpp_tokiter_seek(cpp, start);

	/* insert and expand macro tokens */
	psi_cpp_tokiter_expand_call_tokens(cpp, target, macro, arg_tokens_list);
	psi_cpp_tokiter_free_call_tokens(arg_tokens_list, psi_plist_count(macro->sig), true);

	/* back to where we took off */
	psi_cpp_tokiter_seek(cpp, start);

	free(target);
	++cpp->expanded;
	return true;
}

static bool psi_cpp_tokiter_expand_def(struct psi_cpp *cpp,
		struct psi_token *target, struct psi_cpp_macro_decl *macro)
{
	size_t index = psi_cpp_tokiter_index(cpp);

	/* delete current token from stream */
	psi_cpp_tokiter_del_cur(cpp, false);

	if (index != psi_cpp_tokiter_index(cpp)) {
		/* might have been last token */
		psi_cpp_tokiter_next(cpp);
	}
	/* replace with tokens from macro */
	psi_cpp_tokiter_expand_tokens(cpp, target, macro->tokens);

	free(target);
	++cpp->expanded;
	return true;
}

static inline int psi_cpp_tokiter_expand_cmp(struct psi_token *t,
		struct psi_cpp_macro_decl *m)
{
	if (psi_plist_count(m->tokens) == 1) {
		struct psi_token *r;

		psi_plist_get(m->tokens, 0, &r);

		return strcmp(r->text, t->text);
	}
	return -1;
}

bool psi_cpp_tokiter_expand(struct psi_cpp *cpp)
{
	if (psi_cpp_tokiter_valid(cpp)) {
		struct psi_token *current = psi_cpp_tokiter_current(cpp);

		if (current) {
			struct psi_cpp_macro_decl *macro = zend_hash_str_find_ptr(
					&cpp->defs, current->text, current->size);

			/* don't expand itself */
			if (macro && macro->token != current) {
#if PSI_CPP_DEBUG
				fprintf(stderr, "PSI: CPP expand < ");
				psi_token_dump(2, current);
#endif
				if (macro->sig) {
					return psi_cpp_tokiter_expand_call(cpp, current, macro);
				} else if (psi_cpp_tokiter_expand_cmp(current, macro)) {
					return psi_cpp_tokiter_expand_def(cpp, current, macro);
				}
			}
		}
	}
	return false;
}
