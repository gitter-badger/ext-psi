
typedef struct decl_type {
	text *name;
	token_t type;
} decl_type;

static inline decl_type *init_decl_type(token_t type, text *name) {
	decl_type *t = malloc(sizeof(*t));
	t->type = type;
	t->name = strdup(name);
	return t;
}

static inline void free_decl_type(decl_type *type) {
	free(type->name);
	free(type);
}

typedef struct decl_typedef {
	char *alias;
	decl_type *type;
} decl_typedef;

static inline decl_typedef *init_decl_typedef(text *name, decl_type *type) {
	decl_typedef *t = malloc(sizeof(*t));
	t->alias = strdup(name);
	t->type = type;
	return t;
}

static inline void free_decl_typedef(decl_typedef *t) {
	free(t->alias);
	free_decl_type(t->type);
	free(t);
}

typedef struct decl_typedefs {
	size_t count;
	decl_typedef **list;
} decl_typedefs;

static decl_typedefs *add_decl_typedef(decl_typedefs *defs, decl_typedef *def) {
	if (!defs) {
		defs = calloc(1, sizeof(*defs));
	}
	defs->list = realloc(defs->list, ++defs->count * sizeof(*defs->list));
	defs->list[defs->count-1] = def;
	return defs;
}

static void free_decl_typedefs(decl_typedefs *defs) {
	size_t i;

	for (i = 0; i < defs->count; ++i) {
		free_decl_typedef(defs->list[i]);
	}
	free(defs->list);
	free(defs);
}

typedef struct decl_var {
	text *name;
	unsigned pointer_level;
} decl_var;

static inline decl_var *init_decl_var(text *name, unsigned pl) {
	decl_var *v = malloc(sizeof(*v));
	v->name = (text *) strdup((const char *) name);
	v->pointer_level = pl;
	return v;
}

static inline void free_decl_var(decl_var *var) {
	free(var->name);
	free(var);
}

typedef struct decl_arg {
	decl_type *type;
	decl_var *var;
} decl_arg;

static inline decl_arg *init_decl_arg(decl_type *type, decl_var *var) {
	decl_arg *arg = malloc(sizeof(*arg));
	arg->type = type;
	arg->var = var;
	return arg;
}

static inline void free_decl_arg(decl_arg *arg) {
	free_decl_type(arg->type);
	free_decl_var(arg->var);
	free(arg);
}

typedef struct decl_vars {
	decl_var **vars;
	size_t count;
} decl_vars;

static inline decl_vars *init_decl_vars(decl_var *var) {
	decl_vars *vars = malloc(sizeof(*vars));
	vars->count = 1;
	vars->vars = malloc(sizeof(*vars->vars));
	vars->vars[0] = var;
	return vars;
}

static inline decl_vars *add_decl_var(decl_vars *vars, decl_var *var) {
	vars->vars = realloc(vars->vars, ++vars->count * sizeof(*vars->vars));
	vars->vars[vars->count-1] = var;
	return vars;
}

static inline void free_decl_vars(decl_vars *vars) {
	size_t i;

	for (i = 0; i < vars->count; ++i) {
		free_decl_var(vars->vars[i]);
	}
	free(vars->vars);
	free(vars);
}

typedef struct decl_args {
	decl_arg **args;
	size_t count;
} decl_args;

static inline decl_args *init_decl_args(decl_arg *arg) {
	decl_args *args = malloc(sizeof(*args));
	args->count = 1;
	args->args = malloc(sizeof(*args->args));
	args->args[0] = arg;
	return args;
}

static inline decl_args *add_decl_arg(decl_args *args, decl_arg *arg) {
	args->args = realloc(args->args, ++args->count * sizeof(*args->args));
	args->args[args->count-1] = arg;
	return args;
}

static inline void free_decl_args(decl_args *args) {
	size_t i;

	for (i = 0; i < args->count; ++i) {
		free_decl_arg(args->args[i]);
	}
	free(args->args);
	free(args);
}

typedef struct decl {
	decl_arg *func;
	decl_args *args;
} decl;

static inline decl* init_decl(decl_arg *func, decl_args *args) {
	decl *d = malloc(sizeof(*d));
	d->func = func;
	d->args = args;
	return d;
}

static inline void free_decl(decl *d) {
	free_decl_arg(d->func);
	free_decl_args(d->args);
	free(d);
}

typedef struct decls {
	size_t count;
	decl **list;
} decls;

static inline decls *add_decl(decls *decls, decl *decl) {
	if (!decls) {
		decls = calloc(1, sizeof(*decls));
	}
	decls->list = realloc(decls->list, ++decls->count * sizeof(*decls->list));
	decls->list[decls->count-1] = decl;
	return decls;
}

static inline void free_decls(decls *decls) {
	size_t i;

	for (i = 0; i < decls->count; ++i) {
		free_decl(decls->list[i]);
	}
	free(decls->list);
	free(decls);
}

typedef struct impl_type {
	text *name;
	token_t type;
} impl_type;

static inline impl_type *init_impl_type(token_t type, text *name) {
	impl_type *t = malloc(sizeof(*t));

	t->type = type;
	t->name = (text *) strdup((const char *) name);
	return t;
}

static inline void free_impl_type(impl_type *type) {
	free(type->name);
	free(type);
}

typedef struct impl_var {
	text *name;
	unsigned reference:1;
} impl_var;

static inline impl_var *init_impl_var(text *name, int is_reference) {
	impl_var *var = malloc(sizeof(*var));
	var->name = (text *) strdup((const char *) name);
	var->reference = is_reference;
	return var;
}

static inline void free_impl_var(impl_var *var) {
	free(var->name);
	free(var);
}

typedef struct impl_def_val {
	token_t type;
	union {
		int64_t digits;
		double decimals;
	} v;
	unsigned is_null:1;
} impl_def_val;

static inline impl_def_val *init_impl_def_val() {
	impl_def_val *def = malloc(sizeof(*def));
	def->type = 0;
	def->is_null = 1;
	return def;
}

static inline void free_impl_def_val(impl_def_val *def) {
	free(def);
}

typedef struct impl_arg {
	impl_type *type;
	impl_var *var;
	impl_def_val *def;
} impl_arg;

static inline impl_arg *init_impl_arg(impl_type *type, impl_var *var, impl_def_val *def) {
	impl_arg *arg = malloc(sizeof(*arg));
	arg->type = type;
	arg->var = var;
	arg->def = def;
	return arg;
}

static inline void free_impl_arg(impl_arg *arg) {
	free_impl_type(arg->type);
	free_impl_var(arg->var);
	if (arg->def) {
		free_impl_def_val(arg->def);
	}
	free(arg);
}

typedef struct impl_args {
	impl_arg **args;
	size_t count;
} impl_args;

static inline impl_args *init_impl_args(impl_arg *arg) {
	impl_args *args = malloc(sizeof(*args));
	args->args = malloc(sizeof(*args->args));
	if (arg) {
		args->count = 1;
		args->args[0] = arg;
	} else {
		args->count = 0;
		args->args = NULL;
	}
	return args;
}

static inline impl_args *add_impl_arg(impl_args *args, impl_arg *arg) {
	args->args = realloc(args->args, ++args->count * sizeof(*args->args));
	args->args[args->count-1] = arg;
	return args;
}

static inline void free_impl_args(impl_args *args) {
	size_t i;

	for (i = 0; i < args->count; ++i) {
		free_impl_arg(args->args[i]);
	}
	free(args->args);
	free(args);
}

typedef struct impl_func {
	text *name;
	impl_args *args;
	impl_type *return_type;
} impl_func;

static inline impl_func *init_impl_func(text *name, impl_args *args, impl_type *type) {
	impl_func *func = malloc(sizeof(*func));
	func->name = strdup(name);
	func->args = args ? args : init_impl_args(NULL);
	func->return_type = type;
	return func;
}

static inline void free_impl_func(impl_func *f) {
	free_impl_type(f->return_type);
	free_impl_args(f->args);
	free(f->name);
	free(f);
}

typedef struct let_func {
	token_t type;
	text *name;
} let_func;

static inline let_func *init_let_func(token_t type, text *name) {
	let_func *func = malloc(sizeof(*func));
	func->type = type;
	func->name = (text *) strdup((const char *) name);
	return func;
}

static inline void free_let_func(let_func *func) {
	free(func->name);
	free(func);
}

typedef struct let_value {
	let_func *func;
	impl_var *var;
	unsigned null_pointer_ref:1;
} let_value;

static inline let_value *init_let_value(let_func *func, impl_var *var, int null_pointer_ref) {
	let_value *val = malloc(sizeof(*val));
	val->null_pointer_ref = null_pointer_ref;
	val->func = func;
	val->var = var;
	return val;
}

static inline void free_let_value(let_value *val) {
	if (val->func) {
		free_let_func(val->func);
	}
	if (val->var) {
		free_impl_var(val->var);
	}
	free(val);
}

typedef struct let_stmt {
	decl_var *var;
	let_value *val;
} let_stmt;

static inline let_stmt *init_let_stmt(decl_var *var, let_value *val) {
	let_stmt *let = malloc(sizeof(*let));
	let->var = var;
	let->val = val;
	return let;
}

static inline void free_let_stmt(let_stmt *stmt) {
	free_decl_var(stmt->var);
	free_let_value(stmt->val);
	free(stmt);
}

typedef struct set_func {
	token_t type;
	text *name;
} set_func;

static inline set_func *init_set_func(token_t type, text *name) {
	set_func *func = malloc(sizeof(*func));
	func->type = type;
	func->name = (text *) strdup((const char *) name);
	return func;
}

static inline void free_set_func(set_func *func) {
	free(func->name);
	free(func);
}

typedef struct set_value {
	set_func *func;
	decl_vars *vars;
} set_value;

static inline set_value *init_set_value(set_func *func, decl_vars *vars) {
	set_value *val = malloc(sizeof(*val));
	val->func = func;
	val->vars = vars;
	return val;
}

static inline void free_set_value(set_value *val) {
	free_set_func(val->func);
	free_decl_vars(val->vars);
	free(val);
}

typedef struct set_stmt {
	impl_var *var;
	set_value *val;
} set_stmt;

static inline set_stmt *init_set_stmt(impl_var *var, set_value *val) {
	set_stmt *set = malloc(sizeof(*set));
	set->var = var;
	set->val = val;
	return set;
}

static inline void free_set_stmt(set_stmt *set) {
	free_impl_var(set->var);
	free_set_value(set->val);
	free(set);
}

typedef struct ret_stmt {
	set_func *func;
	decl_var *decl;
} ret_stmt;

static inline ret_stmt *init_ret_stmt(set_func *func, decl_var *decl) {
	ret_stmt *ret = malloc(sizeof(*ret));
	ret->func = func;
	ret->decl = decl;
	return ret;
}

static inline void free_ret_stmt(ret_stmt *ret) {
	free_set_func(ret->func);
	free_decl_var(ret->decl);
	free(ret);
}

typedef struct impl_stmt {
	token_t type;
	union {
		let_stmt *let;
		set_stmt *set;
		ret_stmt *ret;
		void *ptr;
	} s;
} impl_stmt;

static inline impl_stmt *init_impl_stmt(token_t type, void *ptr) {
	impl_stmt *stmt = malloc(sizeof(*stmt));
	stmt->type = type;
	stmt->s.ptr = ptr;
	return stmt;
}

static inline void free_impl_stmt(impl_stmt *stmt) {
	switch (stmt->type) {
	case PSI_T_LET:
		free_let_stmt(stmt->s.let);
		break;
	case PSI_T_SET:
		free_set_stmt(stmt->s.set);
		break;
	case PSI_T_RET:
		free_ret_stmt(stmt->s.ret);
		break;
	}
	free(stmt);
}

typedef struct impl_stmts {
	impl_stmt **stmts;
	size_t count;
} impl_stmts;

static inline impl_stmts *init_impl_stmts(impl_stmt *stmt) {
	impl_stmts *stmts = malloc(sizeof(*stmts));
	stmts->count = 1;
	stmts->stmts = malloc(sizeof(*stmts->stmts));
	stmts->stmts[0] = stmt;
	return stmts;
}

static inline impl_stmts *add_impl_stmt(impl_stmts *stmts, impl_stmt *stmt) {
	stmts->stmts = realloc(stmts->stmts, ++stmts->count * sizeof(*stmts->stmts));
	stmts->stmts[stmts->count-1] = stmt;
	return stmts;
}

static inline void free_impl_stmts(impl_stmts *stmts) {
	size_t i;

	for (i = 0; i < stmts->count; ++i) {
		free_impl_stmt(stmts->stmts[i]);
	}
	free(stmts->stmts);
	free(stmts);
}

typedef struct impl {
	impl_func *func;
	impl_stmts *stmts;
} impl;

static inline impl *init_impl(impl_func *func, impl_stmts *stmts) {
	impl *i = malloc(sizeof(*i));
	i->func = func;
	i->stmts = stmts;
	return i;
}

static inline void free_impl(impl *impl) {
	free_impl_func(impl->func);
	free_impl_stmts(impl->stmts);
	free(impl);
}

typedef struct impls {
	size_t count;
	impl **list;
} impls;

static impls *add_impl(impls *impls, impl *impl) {
	if (!impls) {
		impls = calloc(1, sizeof(*impls));
	}
	impls->list = realloc(impls->list, ++impls->count * sizeof(*impls->list));
	impls->list[impls->count-1] = impl;
	return impls;
}

static void free_impls(impls *impls) {
	size_t i;

	for (i = 0; i < impls->count; ++i) {
		free_impl(impls->list[i]);
	}
	free(impls->list);
	free(impls);
}
