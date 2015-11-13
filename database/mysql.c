#include "php.h"
#include "zend_interfaces.h"
#include "zend_exceptions.h"

#include "../php_yaf_assist.h"
#include "mysql.h"

zend_class_entry* class_yaf_database_mysql;

#define CLASS_PROPERTY_CONFIG_MASTER	"config_master"
#define CLASS_PROPERTY_CONFIG_SLAVE		"config_slave"
#define CLASS_PROPERTY_CACHE_MASTER		"cache_master"
#define CLASS_PROPERTY_CACHE_SLAVE		"cache_slave"
#define CLASS_NAME_MYSQLI				"mysqli"
#define CONFIG_KEY_HOST					"host"
#define CONFIG_KEY_PORT					"port"
#define CONFIG_KEY_USER					"user"
#define CONFIG_KEY_PASS					"pass"
#define CONFIG_KEY_NAME					"name"

// ----------------------------------------------------------------------------
// 静态
ZEND_BEGIN_ARG_INFO_EX(argv_bootstrap, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, master, IS_ARRAY, 0)
	ZEND_ARG_TYPE_INFO(0, slave , IS_ARRAY, 1)
ZEND_END_ARG_INFO()
ZEND_METHOD(yaf_database_mysql, bootstrap) {
	// 接收数据库配置
	zval* master;
	zval* slave = NULL;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "z|z", &master, &slave) == FAILURE || Z_TYPE_P(master) != IS_ARRAY || slave != NULL && Z_TYPE_P(slave) != IS_ARRAY) {
		zend_throw_exception(NULL, "'master'|'slave' must be of type Array", YAF_EXCEPTION_DATABASE_MYSQL);
		return;
	}

	if (!zend_hash_index_exists(Z_ARR_P(master), 0l) || slave != NULL && !zend_hash_index_exists(Z_ARR_P(slave), 0l)) {
		zend_throw_exception(NULL, "'master'/'slave' must be of type Array", YAF_EXCEPTION_DATABASE_MYSQL);
		return;
	}
	// TODO 是否要检查 master[0][host] 等结构？
	zend_update_static_property(class_yaf_database_mysql, ZEND_STRL(CLASS_PROPERTY_CONFIG_MASTER), master);
	if (slave != NULL) {
		zend_update_static_property(class_yaf_database_mysql, ZEND_STRL(CLASS_PROPERTY_CONFIG_SLAVE), slave);
	}
}

static int create_yaf_database_mysql(zval* conf, zval* retval) {
	// 先构造一个 mysqli 实例
	zend_string* class_name_mysqli = zend_string_init(ZEND_STRL(CLASS_NAME_MYSQLI), 0);
	zend_class_entry *class_mysqli = zend_lookup_class(class_name_mysqli);
	zend_string_release(class_name_mysqli);
	if (class_mysqli == NULL) {
		zend_throw_exception(NULL, "depending ext 'mysqli' not found", YAF_EXCEPTION_DATABASE_MYSQL);
		return FAILURE;
	}
	zval mysqli_ins;
	object_init_ex(&mysqli_ins, class_mysqli);
	zval params[5];
	ZVAL_COPY_VALUE(&params[0], zend_hash_str_find(Z_ARR_P(conf), ZEND_STRL(CONFIG_KEY_HOST)));
	ZVAL_COPY_VALUE(&params[1], zend_hash_str_find(Z_ARR_P(conf), ZEND_STRL(CONFIG_KEY_USER)));
	ZVAL_COPY_VALUE(&params[2], zend_hash_str_find(Z_ARR_P(conf), ZEND_STRL(CONFIG_KEY_PASS)));
	zval *config_name = zend_hash_str_find(Z_ARR_P(conf), ZEND_STRL(CONFIG_KEY_NAME));
	if (config_name != NULL) {
		ZVAL_COPY_VALUE(&params[3], config_name);
	} else {
		ZVAL_EMPTY_STRING(&params[3]);
	}
	ZVAL_COPY_VALUE(&params[4], zend_hash_str_find(Z_ARR_P(conf), ZEND_STRL(CONFIG_KEY_PORT)));
	call_method_array(&mysqli_ins, Z_OBJCE(mysqli_ins), ZEND_STRL("__construct"), NULL, params, 5);
	
	// 构造 yaf_database_mysql 实例
	ZVAL_MAKE_REF(&mysqli_ins); // 构造函数需要引用型
	object_init_ex(retval, class_yaf_database_mysql);
	zend_call_method_with_1_params(retval, class_yaf_database_mysql, NULL, "__construct", NULL, &mysqli_ins);

	zval_ptr_dtor(&mysqli_ins);
	zval_ptr_dtor(params);
	zval_ptr_dtor(params + 1);
	zval_ptr_dtor(params + 2);
	zval_ptr_dtor(params + 3);
	zval_ptr_dtor(params + 4);

	ZVAL_MAKE_REF(retval);
	return SUCCESS;
}
static void create_from_cache(zend_ulong index, const char* cache_name, size_t cache_name_len, const char* config_name, size_t config_name_len, zval* return_value) {
	// 确认是否已经缓存了对象
	zval* cache = zend_read_static_property(class_yaf_database_mysql, cache_name, cache_name_len, 1);
	if (Z_TYPE_P(cache) == IS_ARRAY) {
		zval* cache_db = zend_hash_index_find(Z_ARR_P(cache), index);
		if (cache_db != NULL) { // Z_TYPE(cache_db) == IS_REFERENCE
			RETURN_ZVAL(cache_db, 1, 0);
		}
	} else {
		ZVAL_NEW_ARR(cache);
	}
	zval* master = zend_read_static_property(class_yaf_database_mysql, config_name, config_name_len, 0);
	zval* conf = zend_hash_index_find(Z_ARR_P(master), index);

	if (conf == NULL) {
		zend_throw_exception(NULL, "index does not exist", YAF_EXCEPTION_DATABASE_MYSQL);
		return;
	}
	if (create_yaf_database_mysql(conf, return_value) == SUCCESS) {
		// 保留缓存
		zend_hash_index_add(Z_ARR_P(cache), index, return_value);
		Z_ADDREF_P(return_value); // 返回一份，己方这份保留
	}
}


ZEND_BEGIN_ARG_INFO_EX(argv_get_master, 0, ZEND_RETURN_REFERENCE, 0)
	ZEND_ARG_INFO(0, index)
ZEND_END_ARG_INFO()
ZEND_METHOD(yaf_database_mysql, getMaster) {
	zend_ulong index  = 0l;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|l", &index) == FAILURE) {
		index = 0l;
	}
	create_from_cache(index, ZEND_STRL(CLASS_PROPERTY_CACHE_MASTER), ZEND_STRL(CLASS_PROPERTY_CONFIG_MASTER), return_value);
}

ZEND_BEGIN_ARG_INFO_EX(argv_get_slave, 0, ZEND_RETURN_REFERENCE, 0)
	ZEND_ARG_INFO(0, index)
ZEND_END_ARG_INFO()
ZEND_METHOD(yaf_database_mysql, getSlave) {
	zend_ulong index = 0l;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|l", &index) == FAILURE) {
		index = 0l;
	}
	create_from_cache(index, ZEND_STRL(CLASS_PROPERTY_CACHE_SLAVE), ZEND_STRL(CLASS_PROPERTY_CONFIG_SLAVE), return_value);
}
// -----------------------------------------------------------------------------
// 实例
ZEND_BEGIN_ARG_INFO_EX(argv_construct, 0, 0, 1)
	ZEND_ARG_INFO(1, mysqli)
ZEND_END_ARG_INFO()
ZEND_METHOD(yaf_database_mysql, __construct) {
	zval* mysqli;
	// 检查实例类型
	zend_string* class_name_mysqli = zend_string_init(ZEND_STRL(CLASS_NAME_MYSQLI), 0);
	zend_class_entry *class_mysqli = zend_lookup_class(class_name_mysqli);
	zend_string_release(class_name_mysqli);
	if (class_mysqli == NULL) {
		zend_throw_exception(NULL, "depending ext 'mysqli' not found", YAF_EXCEPTION_DATABASE_MYSQL);
		return;
	}
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &mysqli) == FAILURE || Z_TYPE_P(mysqli) != IS_REFERENCE || !instanceof_function(Z_OBJCE_P(Z_REFVAL_P(mysqli)), class_mysqli)) {
		zend_throw_exception(NULL, "'mysqli' instance reference is required", YAF_EXCEPTION_DATABASE_MYSQL);
		return;
	}
	zend_update_property(class_yaf_database_mysql, getThis(), ZEND_STRL(CLASS_NAME_MYSQLI), mysqli);
}
ZEND_BEGIN_ARG_INFO_EX(argv_format_query, 0, 0, 1)
	ZEND_ARG_INFO(0, format)
	ZEND_ARG_VARIADIC_INFO(0, vars)
ZEND_END_ARG_INFO()
ZEND_METHOD(yaf_database_mysql, formatQuery) {
	zval   *format_str,
		   *vars = NULL,
		   *mysqli;
	size_t size;
	int    vars_len;
	// 解析参数
	if(zend_parse_parameters(ZEND_NUM_ARGS(), "z*", &format_str, &vars, &vars_len) == FAILURE) {
		zend_throw_exception(NULL, "'format' of type 'string' is required", YAF_EXCEPTION_DATABASE_MYSQL);
		return;
	}
	mysqli = zend_read_property(class_yaf_database_mysql, getThis(), ZEND_STRL(CLASS_NAME_MYSQLI), 1, NULL);
	if(vars_len == 0) { // 没有需要 format 的参数时，直接代理到原 query 函数
		zend_call_method_with_1_params(mysqli, Z_OBJCE_P(mysqli), NULL, "query", return_value, format_str);
		return;
	}
	size = ZSTR_LEN(Z_STR_P(format_str));
	// TODO 
	size_t i = 0, j = 0, m = 0;
	for(;m<vars_len;++m) { // 将所有参数转换为 string 类型，并进行 escape
		convert_to_string(vars + m);
		zend_call_method_with_1_params(mysqli, Z_OBJCE_P(mysqli), NULL, "escape_string", &vars[m], &vars[m]);
		// 计算目标 string 长度
		size += ZSTR_LEN(Z_STR(vars[m])) + 1; // 减去 ? 占位符 -1 加上 转义后 需要包裹的两个单引号 + 2
	}

	if(m == 0) {
		zend_throw_exception(NULL, "placeholders and variables don't match", YAF_EXCEPTION_DATABASE_MYSQL);
		return;
	}
	zval query_argv;
	zend_string* sql = zend_string_alloc(size, 0);
	m = 0;
	while(i< ZSTR_LEN(Z_STR_P(format_str))) {
		while(ZSTR_VAL(Z_STR_P(format_str))[i] != '?') {
			ZSTR_VAL(sql)[j++] = ZSTR_VAL(Z_STR_P(format_str))[i++];
		}
		if(m >= vars_len) {
			break; // 碰到了 "?" 占位符，但已经没有对应的参数了
		}
		ZSTR_VAL(sql)[j++] = '\'';
		memcpy(ZSTR_VAL(sql) + j, ZSTR_VAL(Z_STR(vars[m])), ZSTR_LEN(Z_STR(vars[m])));
		j += ZSTR_LEN(Z_STR(vars[m]));
		ZSTR_VAL(sql)[j++] = '\'';
		++i;
		++m;
	}
	// 提供的变量没有用完 或者 不够（提前结束了遍历）
	if(m != ZEND_NUM_ARGS() - 1 || i < ZSTR_LEN(Z_STR_P(format_str))) {
		zend_throw_exception(NULL, "placeholders and variables don't match", YAF_EXCEPTION_DATABASE_MYSQL);
		return;
	}
	ZVAL_STR(&query_argv, sql); // 转接调用 mysqli->query
	zend_call_method_with_1_params(mysqli, Z_OBJCE_P(mysqli), NULL, "query", return_value, &query_argv);
	// zend_string_free(sql); // zval 会将其内部的 value 释放
	zval_ptr_dtor(&query_argv);
}

ZEND_BEGIN_ARG_INFO_EX(argv_call, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, method, IS_STRING, 0)
	ZEND_ARG_VARIADIC_INFO(0, vars)
ZEND_END_ARG_INFO()
ZEND_METHOD(yaf_database_mysql, __call) {
	zend_string* method_name;
	zval*        method_argv;
	// 解析参数
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "Sa/", &method_name, &method_argv) == FAILURE) {
		zend_throw_exception(NULL, "method_name/method_argv", YAF_EXCEPTION_DATABASE_MYSQL);
		return;
	}
	
	zval *mysqli = zend_read_property(class_yaf_database_mysql, getThis(), ZEND_STRL(CLASS_NAME_MYSQLI), 1, NULL);
	if (zend_hash_exists(&Z_OBJCE_P(mysqli)->function_table, method_name)) { // 对应方法存在直接转接
		call_method_hash(mysqli, Z_OBJCE_P(mysqli), ZSTR_VAL(method_name), ZSTR_LEN(method_name), return_value, method_argv);
	} else { // 不存在的方法
		zend_throw_exception(NULL, "method does not exist", YAF_EXCEPTION_DATABASE_MYSQL);
	}
}

ZEND_BEGIN_ARG_INFO_EX(argv_isset, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, method, IS_STRING, 0)
ZEND_END_ARG_INFO()
ZEND_METHOD(yaf_database_mysql, __isset) {
	zend_string* argv_name;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &argv_name) == FAILURE) {
		zend_throw_exception(NULL, "argv_name is required", YAF_EXCEPTION_DATABASE_MYSQL);
		return;
	}
	zval *mysqli = zend_read_property(class_yaf_database_mysql, getThis(), ZEND_STRL(CLASS_NAME_MYSQLI), 1, NULL);
	ZVAL_BOOL(return_value, zend_hash_exists(&Z_OBJCE_P(mysqli)->properties_info, argv_name));
}
ZEND_METHOD(yaf_database_mysql, __get) {
	zend_string* argv_name;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &argv_name) == FAILURE) {
		zend_throw_exception(NULL, "argv_name is required", YAF_EXCEPTION_DATABASE_MYSQL);
		return;
	}
	zval *mysqli = zend_read_property(class_yaf_database_mysql, getThis(), ZEND_STRL(CLASS_NAME_MYSQLI), 1, NULL);
	zend_read_property(Z_OBJCE_P(mysqli), mysqli, ZSTR_VAL(argv_name), ZSTR_LEN(argv_name), 1, return_value);
}

const zend_function_entry methods_yaf_database_mysql[] = {
	ZEND_ME(yaf_database_mysql, bootstrap, argv_bootstrap , ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	ZEND_ME(yaf_database_mysql, getMaster, argv_get_master, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	ZEND_ME(yaf_database_mysql, getSlave , argv_get_slave , ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)

	ZEND_ME(yaf_database_mysql, __construct, argv_construct   , ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	ZEND_ME(yaf_database_mysql, formatQuery, argv_format_query, ZEND_ACC_PUBLIC)
	ZEND_ME(yaf_database_mysql, __call     , argv_call        , ZEND_ACC_PUBLIC)
	ZEND_ME(yaf_database_mysql, __isset    , argv_isset       , ZEND_ACC_PUBLIC)
	ZEND_ME(yaf_database_mysql, __get      , argv_isset       , ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

int yaf_database_mysql_init() {
	zend_class_entry ce;
	// 汉字
	INIT_CLASS_ENTRY(ce, "yaf_database_mysql", methods_yaf_database_mysql);
	class_yaf_database_mysql = zend_register_internal_class_ex(&ce, NULL);

	zend_declare_property_null(class_yaf_database_mysql, ZEND_STRL(CLASS_NAME_MYSQLI), ZEND_ACC_PUBLIC);
	zend_declare_property_null(class_yaf_database_mysql, ZEND_STRL(CLASS_PROPERTY_CACHE_MASTER), ZEND_ACC_PRIVATE | ZEND_ACC_STATIC);
	zend_declare_property_null(class_yaf_database_mysql, ZEND_STRL(CLASS_PROPERTY_CONFIG_MASTER), ZEND_ACC_PRIVATE | ZEND_ACC_STATIC);
	zend_declare_property_null(class_yaf_database_mysql, ZEND_STRL(CLASS_PROPERTY_CACHE_SLAVE), ZEND_ACC_PRIVATE | ZEND_ACC_STATIC);
	zend_declare_property_null(class_yaf_database_mysql, ZEND_STRL(CLASS_PROPERTY_CONFIG_SLAVE) , ZEND_ACC_PRIVATE | ZEND_ACC_STATIC);
	return SUCCESS;
}


