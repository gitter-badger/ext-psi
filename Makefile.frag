# provide headers in builddir, so they do not end up in /usr/include/ext/psi/src

PHP_PSI_HEADERS := $(addprefix $(PHP_PSI_BUILDDIR)/,$(PHP_PSI_HEADERS))
PHP_PSI_SOURCES := $(addprefix $(PHP_PSI_SRCDIR)/,$(PHP_PSI_SOURCES))

$(PHP_PSI_BUILDDIR)/types/%.h: $(PHP_PSI_SRCDIR)/src/types/%.h
	@cat >$@ <$<
$(PHP_PSI_BUILDDIR)/%.h: $(PHP_PSI_SRCDIR)/src/%.h
	@cat >$@ <$<

install-headers: psi-build-headers
clean: psi-clean

.PHONY: psi-build-headers
psi-build-headers: $(PHP_PSI_HEADERS)

.PHONY: psi-clean-headers
psi-clean-headers:
	-rm -f $(PHP_PSI_HEADERS)

.PHONY: psi-clean-sources
psi-clean-sources:
	-rm -f $(PHP_PSI_BUILDDIR)/src/*o
	-rm -f $(PHP_PSI_BUILDDIR)/src/types/*o
	-rm -f $(PHP_PSI_SRCDIR)/src/parser.c $(PHP_PSI_SRCDIR)/src/parser_proc.c $(PHP_PSI_SRCDIR)/src/parser_proc.y

.PHONY: psi-clean-aux
psi-clean-aux:
	-rm -f lempar.c lemon.c lemon

.PHONY: psi-clean
psi-clean: psi-clean-headers psi-clean-sources psi-clean-aux
	
		
lempar.c:
	curl -sSo $@ "http://www.sqlite.org/src/raw/tool/lempar.c?name=3ec1463a034b37d87d782be5f6b8b10a3b1ecbe7"

lemon.c:
	curl -sSo $@ "http://www.sqlite.org/src/raw/tool/lemon.c?name=799e73e19a33b8dd7767a7fa34618ed2a9c2397d"

./lemon: lemon.c | lempar.c
	$(CC) -o $@ $<

$(PHP_PSI_SRCDIR)/src/parser_proc.h: $(PHP_PSI_SRCDIR)/src/parser_proc.c

$(PHP_PSI_SRCDIR)/src/parser_proc.inc:
$(PHP_PSI_SRCDIR)/src/parser_proc.y: $(PHP_PSI_SRCDIR)/src/parser_def.h $(PHP_PSI_SRCDIR)/src/parser_proc.inc
	cat $(PHP_PSI_SRCDIR)/src/parser_proc.inc >$@
	$(CPP) -P -DGENERATE $< >>$@
$(PHP_PSI_SRCDIR)/src/parser_proc.c: $(PHP_PSI_SRCDIR)/src/parser_proc.y $(LEMON)
	$(LEMON) -c $<

$(PHP_PSI_SRCDIR)/src/parser.re: $(PHP_PSI_SRCDIR)/src/parser_proc.h
	touch $@
$(PHP_PSI_SRCDIR)/src/parser.c: $(PHP_PSI_SRCDIR)/src/parser.re
	$(RE2C) -o $@ $<

$(PHP_PSI_SRCDIR)/src/types/decl.c: $(PHP_PSI_SRCDIR)/php_psi_macros.h $(PHP_PSI_SRCDIR)/php_psi_redirs.h
$(PHP_PSI_SRCDIR)/src/context.c: $(PHP_PSI_SRCDIR)/php_psi_consts.h $(PHP_PSI_SRCDIR)/php_psi_decls.h $(PHP_PSI_SRCDIR)/php_psi_fn_decls.h $(PHP_PSI_SRCDIR)/php_psi_structs.h $(PHP_PSI_SRCDIR)/php_psi_types.h $(PHP_PSI_SRCDIR)/php_psi_unions.h $(PHP_PSI_SRCDIR)/php_psi_va_decls.h
