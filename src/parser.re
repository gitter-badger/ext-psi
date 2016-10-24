#ifdef HAVE_CONFIG_H
# include "config.h"
#else
# include "php_config.h"
#endif

#include <stddef.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <string.h>

#include "parser_proc.h"

#include "parser.h"

void *psi_parser_proc_Alloc(void*(unsigned long));
void psi_parser_proc_Free(void*, void(*)(void*));
void psi_parser_proc_(void *, token_t, struct psi_token *, struct psi_parser *);
void psi_parser_proc_Trace(FILE *, const char*);

struct psi_parser *psi_parser_init(struct psi_parser *P, const char *filename, psi_error_cb error, unsigned flags)
{
	FILE *fp;

	fp = fopen(filename, "r");

	if (!fp) {
		if (!(flags & PSI_PARSER_SILENT)) {
			error(NULL, NULL, PSI_WARNING, "Could not open '%s' for reading: %s",
					filename, strerror(errno));
		}
		return NULL;
	}

	if (!P) {
		P = malloc(sizeof(*P));
	}
	memset(P, 0, sizeof(*P));

	P->psi.file.fn = strdup(filename);
	P->fp = fp;
	P->col = 1;
	P->line = 1;
	P->error = error;
	P->flags = flags;
	P->proc = psi_parser_proc_Alloc(malloc);

	if (flags & PSI_PARSER_DEBUG) {
		psi_parser_proc_Trace(stderr, "PSI> ");
	}

	psi_parser_fill(P, 0);

	return P;
}

ssize_t psi_parser_fill(struct psi_parser *P, size_t n)
{
	if (P->flags & PSI_PARSER_DEBUG) {
		fprintf(stderr, "PSI> Fill: n=%zu\n", n);
	}
	if (!n) {
		P->cur = P->tok = P->lim = P->mrk = P->buf;
		P->eof = NULL;
	}

	if (!P->eof) {
		size_t consumed = P->tok - P->buf;
		size_t reserved = P->lim - P->tok;
		size_t available = BSIZE - reserved;
		size_t didread;

		if (consumed) {
			memmove(P->buf, P->tok, reserved);
			P->tok -= consumed;
			P->cur -= consumed;
			P->lim -= consumed;
			P->mrk -= consumed;
		}

		didread = fread(P->lim, 1, available, P->fp);
		P->lim += didread;
		if (didread < available) {
			P->eof = P->lim;
		}

		if (P->flags & PSI_PARSER_DEBUG) {
			fprintf(stderr, "PSI> Fill: consumed=%zu reserved=%zu available=%zu didread=%zu\n",
				consumed, reserved, available, didread);
		}
	}
	if (P->flags & PSI_PARSER_DEBUG) {
		fprintf(stderr, "PSI> Fill: avail=%td\n", P->lim - P->cur);
	}
	return P->lim - P->cur;
}

void psi_parser_parse(struct psi_parser *P, struct psi_token *T)
{
	if (T) {
		psi_parser_proc_(P->proc, T->type, T, P);
	} else {
		psi_parser_proc_(P->proc, 0, NULL, P);
	}
}

void psi_parser_dtor(struct psi_parser *P)
{
	psi_parser_proc_Free(P->proc, free);

	if (P->fp) {
		fclose(P->fp);
	}

	psi_data_dtor(PSI_DATA(P));

	memset(P, 0, sizeof(*P));
}

void psi_parser_free(struct psi_parser **P)
{
	if (*P) {
		psi_parser_dtor(*P);
		free(*P);
		*P = NULL;
	}
}

/*!max:re2c*/
#define BSIZE 256

#if BSIZE < YYMAXFILL
# error BSIZE must be greater than YYMAXFILL
#endif

#define RETURN(t) do { \
	P->num = t; \
	if (P->flags & PSI_PARSER_DEBUG) { \
		fprintf(stderr, "PSI> TOKEN: %d %.*s (EOF=%d %s:%u:%u)\n", \
				P->num, (int) (P->cur-P->tok), P->tok, P->num == PSI_T_EOF, \
				P->psi.file.fn, P->line, P->col); \
	} \
	return t; \
} while(1)

#define ADDCOLS \
	P->col += P->cur - P->tok

#define NEWLINE(label) \
	P->col = 1; \
	++P->line; \
	goto label

token_t psi_parser_scan(struct psi_parser *P)
{
	for (;;) {
		ADDCOLS;
	nextline:
		P->tok = P->cur;
		/*!re2c
		re2c:indent:top = 2;
		re2c:define:YYCTYPE = "unsigned char";
		re2c:define:YYCURSOR = P->cur;
		re2c:define:YYLIMIT = P->lim;
		re2c:define:YYMARKER = P->mrk;
		re2c:define:YYFILL = "{ if (!psi_parser_fill(P,@@)) RETURN(PSI_T_EOF); }";
		re2c:yyfill:parameter = 0;

		B = [^a-zA-Z0-9_];
		W = [a-zA-Z0-9_];
		NAME = [a-zA-Z_]W*;
		NSNAME = (NAME)? ("\\" NAME)+;
		DOLLAR_NAME = '$' W+;
		QUOTED_STRING = "\"" ([^\"])+ "\"";
		NUMBER = [+-]? [0-9]* "."? [0-9]+ ([eE] [+-]? [0-9]+)?;

		"/*" { goto comment; }
		("#"|"//") .* "\n" { NEWLINE(nextline); }
		"(" {RETURN(PSI_T_LPAREN);}
		")" {RETURN(PSI_T_RPAREN);}
		";" {RETURN(PSI_T_EOS);}
		"," {RETURN(PSI_T_COMMA);}
		":" {RETURN(PSI_T_COLON);}
		"{" {RETURN(PSI_T_LBRACE);}
		"}" {RETURN(PSI_T_RBRACE);}
		"[" {RETURN(PSI_T_LBRACKET);}
		"]" {RETURN(PSI_T_RBRACKET);}
		"=" {RETURN(PSI_T_EQUALS);}
		"*" {RETURN(PSI_T_ASTERISK);}
		"&" {RETURN(PSI_T_AMPERSAND);}
		"+" {RETURN(PSI_T_PLUS);}
		"-" {RETURN(PSI_T_MINUS);}
		"/" {RETURN(PSI_T_SLASH);}
		"..." {RETURN(PSI_T_ELLIPSIS);}
		[\r\n] { NEWLINE(nextline); }
		[\t ]+ { continue; }
		'TRUE' {RETURN(PSI_T_TRUE);}
		'FALSE' {RETURN(PSI_T_FALSE);}
		'NULL' {RETURN(PSI_T_NULL);}
		'MIXED' {RETURN(PSI_T_MIXED);}
		'CALLABLE' {RETURN(PSI_T_CALLABLE);}
		'VOID' {RETURN(PSI_T_VOID);}
		'BOOL' {RETURN(PSI_T_BOOL);}
		'CHAR' {RETURN(PSI_T_CHAR);}
		'SHORT' {RETURN(PSI_T_SHORT);}
		'INT' {RETURN(PSI_T_INT);}
		'LONG' {RETURN(PSI_T_LONG);}
		'FLOAT' {RETURN(PSI_T_FLOAT);}
		'DOUBLE' {RETURN(PSI_T_DOUBLE);}
		'INT8_T' {RETURN(PSI_T_INT8);}
		'UINT8_T' {RETURN(PSI_T_UINT8);}
		'INT16_T' {RETURN(PSI_T_INT16);}
		'UINT16_T' {RETURN(PSI_T_UINT16);}
		'INT32_T' {RETURN(PSI_T_INT32);}
		'UINT32_T' {RETURN(PSI_T_UINT32);}
		'INT64_T' {RETURN(PSI_T_INT64);}
		'UINT64_T' {RETURN(PSI_T_UINT64);}
		'UNSIGNED' {RETURN(PSI_T_UNSIGNED);}
		'SIGNED' {RETURN(PSI_T_SIGNED);}
		'STRING' {RETURN(PSI_T_STRING);}
		'ARRAY' {RETURN(PSI_T_ARRAY);}
		'OBJECT' {RETURN(PSI_T_OBJECT);}
		'CALLBACK' {RETURN(PSI_T_CALLBACK);}
		'FUNCTION' {RETURN(PSI_T_FUNCTION);}
		'TYPEDEF' {RETURN(PSI_T_TYPEDEF);}
		'STRUCT' {RETURN(PSI_T_STRUCT);}
		'UNION' {RETURN(PSI_T_UNION);}
		'ENUM' {RETURN(PSI_T_ENUM);}
		'CONST' {RETURN(PSI_T_CONST);}
		'LIB' {RETURN(PSI_T_LIB);}
		'LET' {RETURN(PSI_T_LET);}
		'SET' {RETURN(PSI_T_SET);}
		'RETURN' {RETURN(PSI_T_RETURN);}
		'FREE' {RETURN(PSI_T_FREE);}
		'TEMP' {RETURN(PSI_T_TEMP);}
		'STRLEN' {RETURN(PSI_T_STRLEN);}
		'STRVAL' {RETURN(PSI_T_STRVAL);}
		'PATHVAL' {RETURN(PSI_T_PATHVAL);}
		'INTVAL' {RETURN(PSI_T_INTVAL);}
		'FLOATVAL' {RETURN(PSI_T_FLOATVAL);}
		'BOOLVAL' {RETURN(PSI_T_BOOLVAL);}
		'ARRVAL' {RETURN(PSI_T_ARRVAL);}
		'OBJVAL' {RETURN(PSI_T_OBJVAL);}
		'ZVAL' {RETURN(PSI_T_ZVAL);}
		'COUNT' {RETURN(PSI_T_COUNT);}
		'CALLOC' {RETURN(PSI_T_CALLOC);}
		'TO_OBJECT' {RETURN(PSI_T_TO_OBJECT);}
		'TO_ARRAY' {RETURN(PSI_T_TO_ARRAY);}
		'TO_STRING' {RETURN(PSI_T_TO_STRING);}
		'TO_INT' {RETURN(PSI_T_TO_INT);}
		'TO_FLOAT' {RETURN(PSI_T_TO_FLOAT);}
		'TO_BOOL' {RETURN(PSI_T_TO_BOOL);}
		NUMBER {RETURN(PSI_T_NUMBER);}
		NAME {RETURN(PSI_T_NAME);}
		NSNAME {RETURN(PSI_T_NSNAME);}
		DOLLAR_NAME {RETURN(PSI_T_DOLLAR_NAME);}
		QUOTED_STRING {RETURN(PSI_T_QUOTED_STRING);}
		[^] {break;}
		*/

	comment:
		P->tok = P->cur;
		/*!re2c
		"\n" { NEWLINE(comment); }
		"*" "/" { continue; }
		[^] { goto comment; }
		*/
	}
	return -1;
}
