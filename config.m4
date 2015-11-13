dnl $Id$
dnl config.m4 for extension yaf_assist

PHP_ARG_ENABLE(yaf_assist, whether to enable yaf_assist support,
  [  --enable-yaf_assist           Enable yaf_assist support])

if test "$PHP_YAF_ASSIST" != "no"; then

  PHP_ADD_EXTENSION_DEP(yaf_assist, mysqli)
  PHP_NEW_EXTENSION(yaf_assist, yaf_assist.c, $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)

fi
