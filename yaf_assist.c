/*
  +----------------------------------------------------------------------+
  | PHP Version 7                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2015 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_yaf_assist.h"
#include "database/mysql.h"

/* If you declare any globals in php_yaf_assist.h uncomment this: */
ZEND_DECLARE_MODULE_GLOBALS(yaf_assist)

/* True global resources - no need for thread safety here */
static int le_yaf_assist;

/* {{{ PHP_INI
 */
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("yaf_assist.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_yaf_assist_globals, yaf_assist_globals)
    STD_PHP_INI_ENTRY("yaf_assist.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_yaf_assist_globals, yaf_assist_globals)
PHP_INI_END()
/* }}} */

/* Remove the following function when you have successfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */

/* Every user-visible function in PHP should document itself in the source */
/* {{{ proto string confirm_yaf_assist_compiled(string arg)
   Return a string to confirm that the module is compiled in */
PHP_FUNCTION(confirm_yaf_assist_compiled)
{
	char *arg = NULL;
	size_t arg_len, len;
	zend_string *strg;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &arg, &arg_len) == FAILURE) {
		return;
	}

	strg = strpprintf(0, "Congratulations! You have successfully modified ext/%.78s/config.m4. Module %.78s is now compiled into PHP.", "yaf_assist", arg);

	RETURN_STR(strg);
}
/* }}} */
/* The previous line is meant for vim and emacs, so it can correctly fold and
   unfold functions in source code. See the corresponding marks just before
   function definition, where the functions purpose is also documented. Please
   follow this convention for the convenience of others editing your code.
*/


/* {{{ php_yaf_assist_init_globals */

static void php_yaf_assist_init_globals(zend_yaf_assist_globals *yaf_assist_globals)
{
	yaf_assist_globals->global_value = 0;
	yaf_assist_globals->global_string = NULL;
}

/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(yaf_assist)
{
	/* If you have INI entries, uncomment these lines */
	REGISTER_INI_ENTRIES();

	yaf_database_mysql_init();
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(yaf_assist)
{
	/* uncomment this line if you have INI entries */
	UNREGISTER_INI_ENTRIES();
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(yaf_assist)
{
#if defined(COMPILE_DL_YAF_ASSIST) && defined(ZTS)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(yaf_assist)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(yaf_assist)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "yaf_assist support", "enabled");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini */
	DISPLAY_INI_ENTRIES();
}
/* }}} */

/* {{{ yaf_assist_functions[]
 *
 * Every user visible function must have an entry in yaf_assist_functions[].
 */
const zend_function_entry yaf_assist_functions[] = {
	PHP_FE(confirm_yaf_assist_compiled,	NULL)		/* For testing, remove later. */
	PHP_FE_END	/* Must be the last line in yaf_assist_functions[] */
};
/* }}} */

/* {{{ yaf_assist_module_entry */
zend_module_entry yaf_assist_module_entry = {
	STANDARD_MODULE_HEADER,
	"yaf_assist",
	yaf_assist_functions,
	PHP_MINIT(yaf_assist),
	PHP_MSHUTDOWN(yaf_assist),
	PHP_RINIT(yaf_assist),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(yaf_assist),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(yaf_assist),
	PHP_YAF_ASSIST_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

void call_method_hash(zval* obj, zend_class_entry* ce, const char *method_name, size_t method_name_len, zval *retval, zval *hash) {
	zend_fcall_info fci;
	zval noreturn;

	fci.size = sizeof(fci);
	fci.no_separation = 1;
	fci.symbol_table = NULL;

	ZVAL_STRINGL(&fci.function_name, method_name, method_name_len);

	fci.object = Z_OBJ_P(obj);
	fci.function_table = NULL;
	fci.params = NULL;
	zend_fcall_info_args(&fci, hash);

	fci.retval = retval ? retval : &noreturn;

	zend_call_function(&fci, NULL);

	zend_fcall_info_args_clear(&fci, 1);
	zval_ptr_dtor(&fci.function_name);
	if (!retval) {
		zval_ptr_dtor(&noreturn);
	}
}

void call_method_array(zval* obj, zend_class_entry* ce, const char *method_name, size_t method_name_len, zval *retval, zval *array, int array_size) {
	zend_fcall_info fci;
	zval noreturn;
	//zend_fcall_info_cache fcic;
	// 
	fci.size = sizeof(fci);
	fci.no_separation = 1;
	fci.symbol_table = NULL;

	ZVAL_STRINGL(&fci.function_name, method_name, method_name_len);

	fci.object = Z_OBJ_P(obj);
	fci.function_table = NULL;
	fci.params = array;
	fci.param_count = array_size;
	fci.retval = retval ? retval : &noreturn;
	zend_call_function(&fci, NULL);

	zval_ptr_dtor(&fci.function_name);
	if (!retval) {
		zval_ptr_dtor(&noreturn);
	}
}

#ifdef COMPILE_DL_YAF_ASSIST
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE();
#endif
ZEND_GET_MODULE(yaf_assist)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
