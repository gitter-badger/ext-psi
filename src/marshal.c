#ifdef HAVE_CONFIG_H
# include "config.h"
#else
# include "php_config.h"
#endif

#include "php.h"
#include "php_psi.h"
#include "parser.h"
#include "marshal.h"
#include "engine.h"
#include "calc.h"

void psi_to_void(zval *return_value, set_value *set, impl_val *ret_val)
{
	RETVAL_NULL();
}

impl_val *psi_let_void(impl_val *tmp, decl_type *spec, token_t impl_type, impl_val *ival, zval *zvalue, void **to_free)
{
	return tmp;
}

void psi_to_zval(zval *return_value, set_value *set, impl_val *ret_val) {
	RETVAL_ZVAL(ret_val->ptr, 1, 0);
}

impl_val *psi_let_zval(impl_val *tmp, decl_type *spec, token_t impl_type, impl_val *ival, zval *zvalue, void **to_free)
{
	*to_free = tmp->ptr = emalloc(sizeof(zval));
	ZVAL_COPY_VALUE(tmp->ptr, zvalue);
	return tmp;
}

void psi_to_bool(zval *return_value, set_value *set, impl_val *ret_val)
{
	psi_to_int(return_value, set, ret_val);
	convert_to_boolean(return_value);
}

static inline impl_val *psi_val_boolval(impl_val *tmp, token_t real_type, zend_bool boolval) {
	switch (real_type) {
	case PSI_T_INT8:		tmp->i8 = boolval;		break;
	case PSI_T_UINT8:		tmp->u8 = boolval;		break;
	case PSI_T_INT16:		tmp->i16 = boolval;		break;
	case PSI_T_UINT16:		tmp->u16 = boolval;		break;
	case PSI_T_INT32:		tmp->i32 = boolval;		break;
	case PSI_T_UINT32:		tmp->u32 = boolval;		break;
	case PSI_T_INT64:		tmp->i64 = boolval;		break;
	case PSI_T_UINT64:		tmp->u64 = boolval;		break;
	case PSI_T_FLOAT:		tmp->fval = boolval;	break;
	case PSI_T_DOUBLE:		tmp->dval = boolval;	break;
#ifdef HAVE_LONG_DOUBLE
	case PSI_T_LONG_DOUBLE:	tmp->ldval = boolval;	break;
#endif
	EMPTY_SWITCH_DEFAULT_CASE();
	}
	return tmp;
}

impl_val *psi_let_boolval(impl_val *tmp, decl_type *spec, token_t impl_type, impl_val *ival, zval *zvalue, void **to_free)
{
	zend_bool boolval;
	token_t real_type = spec ? real_decl_type(spec)->type : PSI_T_UINT8;

	if (ival && impl_type == PSI_T_BOOL) {
		boolval = ival->zend.bval;
	} else {
		boolval = zend_is_true(zvalue);
	}

	return psi_val_boolval(tmp, real_type, boolval);
}

# define RETVAL_LONG_U64(V) \
		if (V > ZEND_LONG_MAX) { \
			char d[24] = {0}; \
			RETVAL_STRING(zend_print_ulong_to_buf(&d[22], V)); \
		} else { \
			RETVAL_LONG(V); \
		}

void psi_to_int(zval *return_value, set_value *set, impl_val *ret_val)
{
	decl_var *var = set->vars->vars[0];
	token_t t = real_decl_type(var->arg->type)->type;
	impl_val *v = deref_impl_val(ret_val, var);

	switch (t) {
	case PSI_T_INT8:		RETVAL_LONG(v->i8);					break;
	case PSI_T_UINT8:		RETVAL_LONG(v->u8);					break;
	case PSI_T_INT16:		RETVAL_LONG(v->i16);				break;
	case PSI_T_UINT16:		RETVAL_LONG(v->u16);				break;
	case PSI_T_INT32:		RETVAL_LONG(v->i32);				break;
	case PSI_T_UINT32:		RETVAL_LONG(v->u32);				break;
	case PSI_T_INT64:		RETVAL_LONG(v->i64);				break;
	case PSI_T_UINT64:		RETVAL_LONG_U64(v->u64);			break;
	case PSI_T_FLOAT:		RETVAL_DOUBLE((double) v->fval);	break;
	case PSI_T_DOUBLE:		RETVAL_DOUBLE(v->dval);				break;
#ifdef HAVE_LONG_DOUBLE
	case PSI_T_LONG_DOUBLE:	RETVAL_DOUBLE((double) v->ldval);	break;
#endif
	EMPTY_SWITCH_DEFAULT_CASE();
	}

	convert_to_long(return_value);
}

static inline impl_val *psi_val_intval(impl_val *tmp, token_t real_type, zend_long intval) {
	switch (real_type) {
	case PSI_T_INT8:		tmp->i8 = intval;		break;
	case PSI_T_UINT8:		tmp->u8 = intval;		break;
	case PSI_T_INT16:		tmp->i16 = intval;		break;
	case PSI_T_UINT16:		tmp->u16 = intval;		break;
	case PSI_T_INT32:		tmp->i32 = intval;		break;
	case PSI_T_UINT32:		tmp->u32 = intval;		break;
	case PSI_T_INT64:		tmp->i64 = intval;		break;
	case PSI_T_UINT64:		tmp->u64 = intval;		break;
	case PSI_T_INT:			tmp->ival = intval;		break;
	case PSI_T_LONG:		tmp->lval = intval;		break;
	case PSI_T_FLOAT:		tmp->fval = intval;		break;
	case PSI_T_DOUBLE:		tmp->dval = intval;		break;
#ifdef HAVE_LONG_DOUBLE
	case PSI_T_LONG_DOUBLE:	tmp->ldval = intval;	break;
#endif
	EMPTY_SWITCH_DEFAULT_CASE();
	}

	return tmp;
}

impl_val *psi_let_intval(impl_val *tmp, decl_type *spec, token_t impl_type, impl_val *ival, zval *zvalue, void **to_free)
{
	zend_long intval;
	token_t real_type = spec ? real_decl_type(spec)->type : PSI_T_LONG;

	if (ival && impl_type == PSI_T_INT) {
		intval = ival->zend.lval;
	} else {
		intval = zval_get_long(zvalue);
	}

	return psi_val_intval(tmp, real_type, intval);
}

void psi_to_double(zval *return_value, set_value *set, impl_val *ret_val)
{
	decl_var *var = set->vars->vars[0];
	token_t t = real_decl_type(var->arg->type)->type;
	impl_val *v = deref_impl_val(ret_val, var);

	switch (t) {
	case PSI_T_FLOAT:		RETVAL_DOUBLE((double) v->fval);	break;
	case PSI_T_DOUBLE:		RETVAL_DOUBLE(v->dval);				break;
#ifdef HAVE_LONG_DOUBLE
	case PSI_T_LONG_DOUBLE:	RETVAL_DOUBLE((double) v->ldval);	break;
#endif
	case PSI_T_INT8:		RETVAL_DOUBLE((double) v->i8);		break;
	case PSI_T_UINT8:		RETVAL_DOUBLE((double) v->u8);		break;
	case PSI_T_INT16:		RETVAL_DOUBLE((double) v->i16);		break;
	case PSI_T_UINT16:		RETVAL_DOUBLE((double) v->u16);		break;
	case PSI_T_INT32:		RETVAL_DOUBLE((double) v->i32);		break;
	case PSI_T_UINT32:		RETVAL_DOUBLE((double) v->u32);		break;
	case PSI_T_INT64:		RETVAL_DOUBLE((double) v->i64);		break;
	case PSI_T_UINT64:		RETVAL_DOUBLE((double) v->u64);		break;
	EMPTY_SWITCH_DEFAULT_CASE();
	}
}

static inline impl_val *psi_val_floatval(impl_val *tmp, token_t real_type, double floatval) {
	switch (real_type) {
	case PSI_T_INT8:		tmp->i8 = floatval;		break;
	case PSI_T_UINT8:		tmp->u8 = floatval;		break;
	case PSI_T_INT16:		tmp->i16 = floatval;	break;
	case PSI_T_UINT16:		tmp->u16 = floatval;	break;
	case PSI_T_INT32:		tmp->i32 = floatval;	break;
	case PSI_T_UINT32:		tmp->u32 = floatval;	break;
	case PSI_T_INT64:		tmp->i64 = floatval;	break;
	case PSI_T_UINT64:		tmp->u64 = floatval;	break;
	case PSI_T_FLOAT:		tmp->fval = floatval;	break;
	case PSI_T_DOUBLE:		tmp->dval = floatval;	break;
#ifdef HAVE_LONG_DOUBLE
	case PSI_T_LONG_DOUBLE:	tmp->ldval = floatval;	break;
#endif
	EMPTY_SWITCH_DEFAULT_CASE();
	}

	return tmp;
}

impl_val *psi_let_floatval(impl_val *tmp, decl_type *spec, token_t impl_type, impl_val *ival, zval *zvalue, void **to_free)
{
	double floatval;
	token_t real_type = spec ? real_decl_type(spec)->type : PSI_T_DOUBLE;

	if (ival && (impl_type == PSI_T_FLOAT || impl_type == PSI_T_DOUBLE)) {
		floatval = ival->dval;
	} else {
		floatval = zval_get_double(zvalue);
	}

	return psi_val_floatval(tmp, real_type, floatval);
}

void psi_to_string(zval *return_value, set_value *set, impl_val *ret_val)
{
	char *str;
	decl_var *var = set->vars->vars[0];
	token_t t = real_decl_type(var->arg->type)->type;

	switch (t) {
	case PSI_T_FLOAT:		RETVAL_DOUBLE((double) deref_impl_val(ret_val, var)->fval);		break;
	case PSI_T_DOUBLE:		RETVAL_DOUBLE(deref_impl_val(ret_val, var)->dval);				break;
#ifdef HAVE_LONG_DOUBLE
	case PSI_T_LONG_DOUBLE:	RETVAL_DOUBLE((double) deref_impl_val(ret_val, var)->ldval);	break;
#endif
	default:
		if (!var->arg->var->pointer_level) {
			RETVAL_STRINGL(&ret_val->cval, 1);
		} else {
			ret_val = deref_impl_val(ret_val, var);
			if (var->arg->var->array_size) {
				str = (char *) ret_val;
			} else {
				str = ret_val->ptr;
			}
			if (str) {
				if (set->num) {
					zend_long n = psi_long_num_exp(set->num, set->outer.val);
					RETVAL_STRINGL(str, n);
				} else {
					RETVAL_STRING(str);
				}
			} else {
				RETVAL_EMPTY_STRING();
			}
		}
		return;
	}

	convert_to_string(return_value);
}

impl_val *psi_let_strval(impl_val *tmp, decl_type *spec, token_t impl_type, impl_val *ival, zval *zvalue, void **to_free)
{
	if (ival && impl_type == PSI_T_STRING) {
		if (ival->zend.str) {
			/*tmp->ptr = estrndup(ival->zend.str->val, ival->zend.str->len);
			*to_free = tmp->ptr;*/
			tmp->ptr = ival->zend.str->val;
		} else {
			tmp->ptr = "";
		}
	} else if (0 && Z_TYPE_P(zvalue) == IS_STRING) {
		tmp->ptr = Z_STRVAL_P(zvalue);
	} else {
		zend_string *zs = zval_get_string(zvalue);
		tmp->ptr = estrdup(zs->val);
		*to_free = tmp->ptr;
		zend_string_release(zs);
	}

	return tmp;
}

impl_val *psi_let_pathval(impl_val *tmp, decl_type *spec, token_t impl_type, impl_val *ival, zval *zvalue, void **to_free)
{
	tmp = psi_let_strval(tmp, spec, impl_type, ival, zvalue, to_free);
	if (SUCCESS != php_check_open_basedir(tmp->ptr)) {
		efree(tmp->ptr);
		tmp->ptr = NULL;
		return *to_free = NULL;
	}
	return tmp;
}

impl_val *psi_let_strlen(impl_val *tmp, decl_type *spec, token_t impl_type, impl_val *ival, zval *zvalue, void **to_free)
{
	if (ival && impl_type == PSI_T_STRING) {
		if (ival->zend.str) {
			tmp->lval = ival->zend.str->len;
		} else {
			tmp->lval = 0;
		}
	} else {
		zend_string *zs = zval_get_string(zvalue);
		tmp->lval = zs->len;
		zend_string_release(zs);
	}

	return tmp;
}

static impl_val *iterate(impl_val *val, size_t size, unsigned i, impl_val *tmp)
{
	memset(tmp, 0, sizeof(*tmp));
	memcpy(tmp, ((char *) val) + size * i, size);
	return tmp;
}

void psi_from_zval_ex(void *cb_ctx, impl_val **ptr, decl_arg *spec, token_t cast, zval *zv, void **tmp)
{
	decl_type *real = real_decl_type(spec->type);
	impl_val *val = *ptr;

	switch (real->type) {
	default:
		ZEND_ASSERT(0);
		val->i64 = zval_get_long(zv);
		break;
	case PSI_T_INT8:
		val->i8 = zval_get_long(zv);
		break;
	case PSI_T_UINT8:
		val->u8 = zval_get_long(zv);
		break;
	case PSI_T_INT16:
		val->i16 = zval_get_long(zv);
		break;
	case PSI_T_UINT16:
		val->u16 = zval_get_long(zv);
		break;
	case PSI_T_INT32:
		val->i32 = zval_get_long(zv);
		break;
	case PSI_T_UINT32:
		val->u32 = zval_get_long(zv);
		break;
	case PSI_T_INT64:
		val->i64 = zval_get_long(zv);
		break;
	case PSI_T_UINT64:
		val->u64 = zval_get_long(zv);
		break;
	case PSI_T_FLOAT:
		val->fval = zval_get_double(zv);
		break;
	case PSI_T_DOUBLE:
		val->dval = zval_get_double(zv);
		break;
#ifdef HAVE_LONG_DOUBLE
	case PSI_T_LONG_DOUBLE:
		val->ldval = zval_get_double(zv);
		break;
#endif
	case PSI_T_ENUM:
		val->ival = zval_get_long(zv);
		break;
	case PSI_T_STRUCT:
		*tmp = *ptr = psi_array_to_struct_ex(real->real.strct, HASH_OF(zv), psi_from_zval_ex, cb_ctx);
		break;
	case PSI_T_UNION:
		*tmp = *ptr = psi_array_to_union_ex(real->real.unn, HASH_OF(zv), psi_from_zval_ex, cb_ctx);
		break;
	case PSI_T_FUNCTION:
		/*FIXME*/
		val->ptr = NULL;
		break;
	case PSI_T_VOID:
		val->ptr = NULL;
		if (Z_TYPE_P(zv) == IS_OBJECT && instanceof_function(Z_OBJCE_P(zv), psi_object_get_class_entry())) {
			*ptr = PSI_OBJ(zv, NULL)->data;
		} else {
			zend_string *zs = zval_get_string(zv);
			*tmp = val->ptr = estrndup(zs->val, zs->len);
			zend_string_release(zs);
		}
		break;
	}
}

void *psi_array_to_struct_ex(decl_struct *s, HashTable *arr, psi_marshal_zval cb, void *cb_ctx) {
	size_t i, j = 0;
	char *mem = ecalloc(1, s->size + s->args->count * sizeof(void *));

	if (arr) for (i = 0; i < s->args->count; ++i) {
		decl_arg *darg = s->args->args[i];
		zval *entry = zend_hash_str_find_ind(arr, darg->var->name, strlen(darg->var->name));

		if (entry) {
			impl_val val, *ptr = &val;
			void *tmp = NULL;

			memset(&val, 0, sizeof(val));
			cb(cb_ctx, &ptr, darg, /*FIXME*/0, entry, &tmp);
			memcpy(mem + darg->layout->pos, ptr, darg->layout->len);
			if (tmp) {
				((void **)(mem + s->size))[j++] = tmp;
			}
		}
	}
	return mem;
}

void *psi_array_to_struct(decl_struct *s, HashTable *arr)
{
	return psi_array_to_struct_ex(s, arr, psi_from_zval_ex, NULL);
}

void *psi_array_to_union_ex(decl_union *u, HashTable *arr, psi_marshal_zval cb, void *cb_ctx) {
	size_t i;
	char *mem = ecalloc(1, u->size + sizeof(void *));

	if (arr) for (i = 0; i < u->args->count; ++i) {
		decl_arg *darg = u->args->args[i];
		zval *entry = zend_hash_str_find_ind(arr, darg->var->name, strlen(darg->var->name));

		if (entry) {
			impl_val val, *ptr = &val;
			void *tmp = NULL;

			memset(&val, 0, sizeof(val));
			cb(cb_ctx, &ptr, darg, /*FIXME*/0, entry, &tmp);
			memcpy(mem, &val, darg->layout->len);
			if (tmp) {
				((void **)(mem + u->size))[0] = tmp;
			}
			/* first found entry wins */
			break;
		}
	}

	return mem;
}

void *psi_array_to_union(decl_union *u, HashTable *arr) {
	return psi_array_to_union_ex(u, arr, psi_from_zval_ex, NULL);
}

void psi_to_recursive(zval *return_value, set_value *set, impl_val *r_val) {
	set->outer.set->func->handler(return_value, set, r_val);
}

void psi_to_array(zval *return_value, set_value *set, impl_val *r_val)
{
	size_t i;
	decl_var *var = set->vars->vars[0];
	token_t t = real_decl_type(var->arg->type)->type;
	impl_val tmp, *ret_val = deref_impl_val(r_val, var);

	if ((intptr_t) ret_val <= (intptr_t) 0) {
		RETURN_NULL();
	}

	array_init(return_value);

	if (t == PSI_T_STRUCT || t == PSI_T_UNION) {
		// decl_struct *s = real_decl_type(var->arg->type)->strct;

		if (set->inner && set->inner->count) {
			/* explicit member casts */
			for (i = 0; i < set->inner->count; ++i) {
				set_value *sub_set = set->inner->vals[i];
				decl_var *sub_var = sub_set->vars->vars[0];

				sub_set->outer.val = ret_val;

				if (sub_var->arg) {
					impl_val *tmp = NULL, *val;
					zval ztmp;

					val = struct_member_ref(sub_var->arg, ret_val, &tmp);
					sub_set->func->handler(&ztmp, sub_set, val);
					add_assoc_zval(return_value, sub_var->name, &ztmp);

					if (tmp) {
						free(tmp);
					}
				}
			}
			return;
		}
	}

	if (var->arg->var->array_size) {
		/* to_array(foo[NUMBER]) */
		for (i = 0; i < var->arg->var->array_size; ++i) {
			size_t size = psi_t_size(var->arg->var->pointer_level > 1 ? PSI_T_POINTER : t);
			impl_val *ptr = iterate(ret_val->ptr, size, i, &tmp);
			zval ele;

			switch (t) {
			case PSI_T_FLOAT:
				ZVAL_DOUBLE(&ele, (double) ptr->fval);
				break;
			case PSI_T_DOUBLE:
				ZVAL_DOUBLE(&ele, ptr->dval);
				break;
			default:
				ZVAL_LONG(&ele, ptr->lval);
				break;
			}

			add_next_index_zval(return_value, &ele);
		}
		return;
	} else if (set->num) {
		/* to_array(arr_var, num_expr, to_int(*arr_var)) */
		zval ele;
		char *ptr;
		zend_long i, n = psi_long_num_exp(set->num, set->outer.val);
		size_t size = psi_t_size(var->arg->var->pointer_level ? PSI_T_POINTER : t);
		set_value *sub_set = set->inner->vals[0];

		sub_set->outer.val = set->outer.val;
		for (i = 0; i < n; ++i) {
			ptr = (char *) ret_val->ptr + i * size;
			sub_set->func->handler(&ele, sub_set, (void *) ptr);
			add_next_index_zval(return_value, &ele);
		}
	} else {
		/* to_array(arr_var, to_int(*arr_var)) */
		zval ele;
		char *ptr = ret_val->ptr;
		size_t size = psi_t_size(var->arg->var->pointer_level ? PSI_T_POINTER : t);
		set_value *sub_set = set->inner->vals[0];

		sub_set->outer.val = set->outer.val;
		while (*(void **) ptr) {
			sub_set->func->handler(&ele, sub_set, (void *) ptr);
			add_next_index_zval(return_value, &ele);
			ptr += size;
		}
	}
}

impl_val *psi_let_arrval(impl_val *tmp, decl_type *spec, decl_var *spec_var, token_t impl_type, impl_val *ival, zval *zvalue, void **to_free)
{
	decl_type *real = real_decl_type(spec);
	HashTable *arr;
	zval *zv;
	size_t i, sz;
	decl_arg tmp_arg = {0};

	if (impl_type != PSI_T_ARRAY) {
		SEPARATE_ARG_IF_REF(zvalue);
		convert_to_array(zvalue);
	}
	arr = HASH_OF(zvalue);

	switch (real->type) {
	case PSI_T_STRUCT:
		*to_free = tmp = psi_array_to_struct(real->real.strct, arr);
		break;
	case PSI_T_UNION:
		*to_free = tmp = psi_array_to_union(real->real.unn, arr);
		break;
	default:
		sz = psi_t_size(real->type);
		tmp = *to_free = ecalloc(zend_hash_num_elements(arr), sz);
		tmp_arg.type = spec;
		tmp_arg.var = spec_var;
		ZEND_HASH_FOREACH_VAL_IND(arr, zv)
		{
			void *ptr = ((char *) tmp) + (i++ * sz);
			psi_from_zval_ex(NULL, (impl_val **) &ptr, &tmp_arg, 0, zv, NULL);
		}
		ZEND_HASH_FOREACH_END();
	}

	return tmp;
}

impl_val *psi_let_count(impl_val *tmp, decl_type *spec, token_t impl_type, impl_val *ival, zval *zvalue, void **to_free)
{
	return psi_val_intval(tmp, real_decl_type(spec)->type, psi_zval_count(zvalue));
}


void psi_to_object(zval *return_value, set_value *set, impl_val *r_val)
{
	decl_var *var = set->vars->vars[0];
	impl_val *ret_val = deref_impl_val(r_val, var);

	if ((intptr_t) ret_val->ptr > (intptr_t) 0) {
		object_init_ex(return_value, psi_object_get_class_entry());
		PSI_OBJ(return_value, NULL)->data = ret_val->ptr;
	} else {
		RETVAL_NULL();
	}
}

impl_val *psi_let_objval(impl_val *tmp, decl_type *spec, token_t impl_type, impl_val *ival, zval *zvalue, void **to_free)
{
	psi_object *obj;

	if (Z_TYPE_P(zvalue) != IS_OBJECT
	||	!instanceof_function(Z_OBJCE_P(zvalue), psi_object_get_class_entry())) {
		return NULL;
	}

	obj = PSI_OBJ(zvalue, NULL);
	tmp->ptr = obj->data;

	return tmp;
}

