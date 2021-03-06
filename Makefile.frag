
# provide headers in builddir, so they do not end up in /usr/include/ext/psi/src

PHP_PSI_HEADERS := $(addprefix $(PHP_PSI_BUILDDIR)/,$(PHP_PSI_HEADERS))
PHP_PSI_SOURCES := $(addprefix $(PHP_PSI_SRCDIR)/,$(PHP_PSI_SOURCES))
PHP_PSI_GENERATED:=$(addprefix $(PHP_PSI_SRCDIR)/,$(PHP_PSI_GENERATED))

$(PHP_PSI_BUILDDIR)/types:
	mkdir -p $@
$(PHP_PSI_BUILDDIR)/types/%.h: $(PHP_PSI_SRCDIR)/src/types/%.h | $(PHP_PSI_BUILDDIR)/types
	@cat >$@ <$<
$(PHP_PSI_BUILDDIR)/%.h: $(PHP_PSI_SRCDIR)/src/%.h
	@cat >$@ <$<

$(PHP_PSI_SRCDIR)/src/parser_proc.h: $(PHP_PSI_SRCDIR)/src/parser_proc.c

$(PHP_PSI_SRCDIR)/src/parser_proc.c: $(PHP_PSI_SRCDIR)/src/parser_proc_grammar.y
	# trickery needed for relative #line directives
	cd $(PHP_PSI_SRCDIR) && bison -Wall -v -d -o $(patsubst $(PHP_PSI_SRCDIR)/%,%,$@) $(patsubst $(PHP_PSI_SRCDIR)/%,%,$<)

$(PHP_PSI_SRCDIR)/src/parser.re: $(PHP_PSI_SRCDIR)/src/parser_proc.h
	touch $@
$(PHP_PSI_SRCDIR)/src/parser.c: $(PHP_PSI_SRCDIR)/src/parser.re
	# trickery needed for relative #line directives
	cd $(PHP_PSI_SRCDIR) && $(RE2C) -o $(patsubst $(PHP_PSI_SRCDIR)/%,%,$@) $(patsubst $(PHP_PSI_SRCDIR)/%,%,$<)

$(PHP_PSI_SRCDIR)/src/calc/%.h: $(PHP_PSI_SRCDIR)/scripts/gen_calc_%.php
	$(PHP_EXECUTABLE) $< >$@

$(PHP_PSI_SRCDIR)/src/calc.h: | $(PHP_PSI_SRCDIR)/src/calc/basic.h $(PHP_PSI_SRCDIR)/src/calc/bin.h $(PHP_PSI_SRCDIR)/src/calc/bool.h $(PHP_PSI_SRCDIR)/src/calc/cast.h $(PHP_PSI_SRCDIR)/src/calc/cmp.h $(PHP_PSI_SRCDIR)/src/calc/oper.h

.PHONY: psi-generated
psi-generated: $(PHP_PSI_GENERATED)

PHP_PSI_DEPEND = $(patsubst $(PHP_PSI_SRCDIR)/%,$(PHP_PSI_BUILDDIR)/%,$(PHP_PSI_SOURCES:.c=.dep))

$(PHP_PSI_BUILDDIR)/%.dep: $(PHP_PSI_SRCDIR)/%.c | $(PHP_PSI_GENERATED)
	$(CC) -MM -MG -MF $@ -MT $(patsubst $(PHP_PSI_SRCDIR)/%,$(PHP_PSI_BUILDDIR)/%,$(@:.dep=.lo)) \
		$(CPPFLAGS) $(DEFS) $(INCLUDES) $< \
			|| touch $@

php_psi_stdinc.h:
php_psi_posix.h:

ifneq ($(findstring clean,$(MAKECMDGOALS)),clean)
ifneq ($(PSI_DEPS),)
-include $(PHP_PSI_DEPEND)
endif
endif

.PHONY: psi-build-headers
psi-build-headers: $(PHP_PSI_HEADERS)

.PHONY: psi-clean-headers
psi-clean-headers:
	-rm -f $(PHP_PSI_HEADERS)

.PHONY: psi-clean
psi-clean: psi-clean-headers

.PHONY: psi-clean-objects
psi-clean-objects:
	-rm -f $(PHP_PSI_BUILDDIR)/src/*o
	-rm -f $(PHP_PSI_BUILDDIR)/src/types/*o

psi-clean: psi-clean-objects

.PHONY: psi-clean-generated
psi-clean-generated:
	-rm -f $(PHP_PSI_GENERATED)

.PHONY: psi-clean-aux
psi-clean-aux: psi-clean-aux-bin psi-clean-aux-src

.PHONY: psi-clean-aux-src
psi-clean-aux-src:
	-rm -f $(PHP_PSI_BUILDDIR)/lempar.c $(PHP_PSI_BUILDDIR)/lemon.c

.PHONY: psi-clean-aux-bin
psi-clean-aux-bin:
	-rm -f $(PHP_PSI_BUILDDIR)/lemon

psi-clean: psi-clean-aux

.PHONY: psi-clean-depend
psi-clean-depend:
	-rm -f $(PHP_PSI_DEPEND)

psi-clean: psi-clean-depend

install-headers: psi-build-headers
clean: psi-clean-headers psi-clean-aux
ifneq ($(PSI_DEPS),)
clean: psi-clean-depend
endif
