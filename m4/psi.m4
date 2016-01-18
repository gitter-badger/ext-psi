dnl Generated headers with pre-defined types, structs, consts and decls.
PSI_STDINC=$PHP_PSI_SRCDIR/php_psi_stdinc.h
PSI_TYPES=$PHP_PSI_SRCDIR/php_psi_types.h
PSI_STRUCTS=$PHP_PSI_SRCDIR/php_psi_structs.h
PSI_CONSTS=$PHP_PSI_SRCDIR/php_psi_consts.h
PSI_REDIRS=$PHP_PSI_SRCDIR/php_psi_redirs.h
PSI_MACROS=$PHP_PSI_SRCDIR/php_psi_macros.h
PSI_DECLS=$PHP_PSI_SRCDIR/php_psi_decls.h
PSI_VA_DECLS=$PHP_PSI_SRCDIR/php_psi_va_decls.h

dnl PSI_CONFIG_INIT()
dnl Creates stubs of the headers with pre-defined types etc.
dnl These headers are included by src/context.c.
dnl This macro must be called prior any checks for a type, struct, decl etc.
AC_DEFUN(PSI_CONFIG_INIT, [
	cat >$PSI_TYPES <<EOF
static struct psi_predef_type {
	token_t type_tag;
	const char *type_name;
	const char *alias;
} psi_predef_types@<:@@:>@ = {
EOF
	cat >$PSI_STRUCTS <<EOF
static struct psi_predef_struct {
	token_t type_tag;
	const char *type_name;
	const char *var_name;
	size_t offset;
	size_t size;
	size_t pointer_level;
	size_t array_size;
} psi_predef_structs@<:@@:>@ = {
EOF
	cat >$PSI_CONSTS <<EOF
static struct psi_predef_const {
	token_t type_tag;
	const char *type_name;
	const char *var_name;
	const char *val_text;
	token_t val_type_tag;
} psi_predef_consts@<:@@:>@ = {
EOF
	cat >$PSI_REDIRS <<EOF
static struct psi_func_redir {
	const char *name;
	void (*func)(void);
} psi_func_redirs@<:@@:>@ = {
EOF
	cat >$PSI_MACROS </dev/null
	cat >$PSI_DECLS <<EOF
static struct psi_predef_decl {
	token_t type_tag;
	const char *type_name;
	const char *var_name;
	size_t pointer_level;
	size_t array_size;
} psi_predef_decls@<:@@:>@ = {
EOF
	cat >$PSI_VA_DECLS <<EOF
static struct psi_predef_decl psi_predef_vararg_decls@<:@@:>@ = {
EOF
])

dnl PSI_CONFIG_DONE()
dnl Finish the headers with the pre-defined types etc.
AC_DEFUN(PSI_CONFIG_DONE, [
	cat >$PSI_STDINC <<EOF
PSI_INCLUDES
EOF
	for i in $PSI_TYPES $PSI_STRUCTS $PSI_CONSTS $PSI_REDIRS $PSI_DECLS $PSI_VA_DECLS; do
		cat >>$i <<EOF
	{0}
};
EOF
	done
])

dnl PSI_INCLUDES()
dnl Expands to a complete list of include statements including
dnl AC_INCLUDES_DEFAULT().
AC_DEFUN(PSI_INCLUDES, [AC_INCLUDES_DEFAULT()
#ifdef HAVE_ERRNO_H
# include <errno.h>
#endif
#ifdef HAVE_GLOB_H
# include <glob.h>
#endif
#ifdef HAVE_LOCALE_H
# include <locale.h>
#endif
#ifdef HAVE_XLOCALE_H
# include <xlocale.h>
#endif
#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif
#ifdef HAVE_NETINET_TCP_H
# include <netinet/tcp.h>
#endif
#ifdef HAVE_ARPA_NAMESER_H
# include <arpa/nameser.h>
#endif
#ifdef HAVE_NETDB_H
# include <netdb.h>
#endif
#ifdef HAVE_POLL_H
# include <poll.h>
#endif
#ifdef HAVE_RESOLV_H
# include <resolv.h>
#endif
#ifdef HAVE_SYS_SELECT_H
# include <sys/select.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
#ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
#endif
#ifdef HAVE_SYS_TIMES_H
# include <sys/times.h>
#endif
#ifdef HAVE_SYS_UIO_H
# include <sys/uio.h>
#endif
#ifdef HAVE_SYS_UTSNAME_H
# include <sys/utsname.h>
#endif
#ifdef HAVE_TIME_H
# include <time.h>
#endif
#ifdef HAVE_SYSLOG_H
# include <syslog.h>
#endif
#ifdef HAVE_WCHAR_H
# include <wchar.h>
#endif
#ifdef HAVE_WCTYPE_H
# include <wctype.h>
#endif
])

dnl PSI_LEMON()
dnl Declare $LEMON precious, and check for a `lemon` in $PATH.
AC_DEFUN(PSI_LEMON, [
	AC_ARG_VAR(LEMON, The lemon parser generator of the SQLite project)
	AC_PATH_PROG(LEMON, lemon, ./lemon)
	PHP_SUBST(LEMON)
])

dnl PSI_PKG_CONFIG()
dnl Check for `pkg-config` and add possible libjit and libffi directories to
dnl $PKG_CONFIG_PATH, because those libs often ship with headers etc. in 
dnl arch-dependent locations.
AC_DEFUN([PSI_PKG_CONFIG], [
	if test -z "$PKG_CONFIG"
	then
		AC_PATH_PROG(PKG_CONFIG, pkg-config, false)
	fi
	export PKG_CONFIG_PATH="$PHP_PSI_LIBFFI/lib/pkgconfig:$PHP_PSI_LIBJIT/lib/pkgconfig:$PKG_CONFIG_PATH"
])

dnl PSI_SH_TEST_SIZEOF(type)
dnl `if` condition to test if $ac_cv_sizeof_$1 is greater than 0.
AC_DEFUN([PSI_SH_TEST_SIZEOF], [test -n "$AS_TR_SH([ac_cv_sizeof_]$1)" && test "$AS_TR_SH([ac_cv_sizeof_]$1)" -gt 0])

dnl PSI_CHECK_SIZEOF(type, special-includes)
dnl AC_CHECK_SIZEOF wrapper with PSI_INCLUDES
dnl Defines psi\\SIZEOF_<TYPE> pre-defined constant in $PSI_CONSTS.
AC_DEFUN(PSI_CHECK_SIZEOF, [
	AC_CHECK_SIZEOF($1, [], PSI_INCLUDES
		$2)
	if PSI_SH_TEST_SIZEOF($1); then
		psi_add_int_const "AS_TR_CPP([SIZEOF_]$1)" "$AS_TR_SH([ac_cv_sizeof_]$1)"
	fi
])

dnl PSI_CHECK_OFFSETOF(struct, element)
dnl Check the offset of a struct element, implemented in the similar manner
dnl like AC_CHECK_SIZEOF.
dnl AC_DEFINEs OFFSETOF_<STRUCT>_<ELEMENT>.
AC_DEFUN(PSI_CHECK_OFFSETOF, [
	_AC_CACHE_CHECK_INT(
		[offset of $2 in $1],
		[AS_TR_SH([ac_cv_offsetof_$1_$2])],
		[(long int) (offsetof ($1, $2))],
		[PSI_INCLUDES],
		[AC_MSG_FAILURE([cannot compute offsetof ($1, $2)])]
	)
	AC_DEFINE_UNQUOTED(
		AS_TR_CPP(offsetof_$1_$2),
		$AS_TR_SH([ac_cv_offsetof_$1_$2]),
		[The offset of `$2' in `$1', as computed by offsetof.]
	)
])

dnl PSI_COMPUTE_STR(variable, string or expression)
dnl Compute a string constant value in a similar manner like AC_COMPUTE_INT.
AC_DEFUN(PSI_COMPUTE_STR, [
	AC_TRY_RUN(
		PSI_INCLUDES
		[int main() {
			return EOF == fputs($2, fopen("conftest.out", "w"));
		}
	], [
		eval $1=\\\"`cat conftest.out`\\\"
	])
])

dnl PSI_CHECK_LIBJIT()
dnl Check for libjit in $PHP_PSI_LIBJIT or standard locations
dnl AC_DEFINEs HAVE_LIBJIT.
AC_DEFUN(PSI_CHECK_LIBJIT, [
	AC_CACHE_CHECK(for libjit, psi_cv_libjit_dir, [
		for psi_cv_libjit_dir in $PHP_PSI_LIBJIT {/usr{,/local},/opt}{,/libjit}
		do
			if test -e $psi_cv_libjit_dir/include/jit/jit.h
			then
				break
			fi
			psi_cv_libjit_dir=
		done
	])
	if test -n "$psi_cv_libjit_dir"
	then
		PHP_ADD_INCLUDE($psi_cv_libjit_dir/include)
		PHP_ADD_LIBRARY_WITH_PATH(jit, $psi_cv_libjit_dir/$PHP_LIBDIR, PSI_SHARED_LIBADD)
		AC_DEFINE(HAVE_LIBJIT, 1, Have libjit)
	else
		AC_MSG_WARN([Could not find libjit, please provide the base install path])
	fi
])

dnl PSI_CHECK_LIBFFI()
dnl Check for libffi with `pkg-config`. If that fails, `configure` looks into
dnl $PHP_PSI_LIBFFI or standard locations to find libjit deps.
dnl Checks for availability of recent closure API: 
dnl \ffi_closure_alloc and \ffi_prep_closure.
dnl Checks for availability of recent vararg API:
dnl \ffi_prep_cif_var.
dnl AC_DEFINEs HAVE_LIBFFI, PSI_HAVE_FFI_CLOSURE_ALLOC, 
dnl PSI_HAVE_FFI_PREP_CLOSURE and PSI_HAVE_FFO_PREP_VIF_VAR.
AC_DEFUN(PSI_CHECK_LIBFFI, [
	AC_REQUIRE([PSI_PKG_CONFIG])dnl

	AC_CACHE_CHECK(for libffi through pkg-config, psi_cv_libffi, [
	if $PKG_CONFIG --exists libffi
	then
		psi_cv_libffi=true
	else
		psi_cv_libffi=false
	fi])
	
	if $psi_cv_libffi
	then
		AC_MSG_CHECKING(for libffi)
		psi_cv_libffi_dir=`$PKG_CONFIG --variable=prefix libffi`
		AC_MSG_RESULT($psi_cv_libffi_dir)
		PHP_EVAL_INCLINE(`$PKG_CONFIG --cflags libffi`)
		PHP_EVAL_LIBLINE(`$PKG_CONFIG --libs libffi`, PSI_SHARED_LIBADD)
		AC_DEFINE(HAVE_LIBFFI, 1, Have libffi)
	else
		AC_CACHE_CHECK(for libffi, psi_cv_libffi_dir, [
		for psi_cv_libffi_dir in $PHP_PSI_LIBFFI {/usr{,/local},/opt}{,/libffi}
		do
			if test -e $psi_cv_libffi_dir/include/ffi/ffi.h
			then
				break
			fi
			psi_cv_libffi_dir=
		done])
		if test -n "$psi_cv_libffi_dir"
		then
			PHP_ADD_INCLUDE($psi_cv_libffi_dir/include/ffi)
			PHP_ADD_LIBRARY_WITH_PATH(ffi, $psi_cv_libffi_dir/$PHP_LIBDIR, PSI_SHARED_LIBADD)
			AC_DEFINE(HAVE_LIBFFI, 1, Have libffi)
		else
			AC_MSG_WARN([Could not find libffi, please provide the base install path])
		fi
	fi
	PHP_CHECK_LIBRARY(ffi, ffi_closure_alloc, [
		PHP_CHECK_LIBRARY(ffi, ffi_prep_closure_loc, [
			AC_DEFINE(PSI_HAVE_FFI_PREP_CLOSURE_LOC, 1, [ ])
		], [], -L$psi_cv_libffi_dir/$PHP_LIBDIR)
		AC_DEFINE(PSI_HAVE_FFI_CLOSURE_ALLOC, 1, [ ])
	], [
		PHP_CHECK_LIBRARY(ffi, ffi_prep_closure, [
			AC_CHECK_HEADERS(sys/mman.h)
			PHP_CHECK_FUNC(mmap)
			AC_DEFINE(PSI_HAVE_FFI_PREP_CLOSURE, 1, [ ])
		], [
		], -L$psi_cv_libffi_dir/$PHP_LIBDIR)
	], -L$psi_cv_libffi_dir/$PHP_LIBDIR)
	PHP_CHECK_LIBRARY(ffi, ffi_prep_cif_var, [
		AC_DEFINE(PSI_HAVE_FFI_PREP_CIF_VAR, 1, [ ])
	], [
	], -L$psi_cv_libffi_dir/$PHP_LIBDIR)
])
