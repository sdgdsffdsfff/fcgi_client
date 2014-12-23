dnl $Id$
dnl config.m4 for extension fcgi_client

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(fcgi_client, for fcgi_client support,
dnl Make sure that the comment i aligned:
dnl [  --with-fcgi_client             Include fcgi_client support])

dnl Otherwise use enable:

PHP_ARG_ENABLE(fcgi_client, whether to enable fcgi_client support,
Make sure that the comment is aligned:
[  --enable-fcgi_client           Enable fcgi_client support])

if test "$PHP_FCGI_CLIENT" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-fcgi_client -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/fcgi_client.h"  # you most likely want to change this
  dnl if test -r $PHP_FCGI_CLIENT/$SEARCH_FOR; then # path given as parameter
  dnl   FCGI_CLIENT_DIR=$PHP_FCGI_CLIENT
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for fcgi_client files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       FCGI_CLIENT_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$FCGI_CLIENT_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the fcgi_client distribution])
  dnl fi

  dnl # --with-fcgi_client -> add include path
  dnl PHP_ADD_INCLUDE($FCGI_CLIENT_DIR/include)

  dnl # --with-fcgi_client -> check for lib and symbol presence
  dnl LIBNAME=fcgi_client # you may want to change this
  dnl LIBSYMBOL=fcgi_client # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $FCGI_CLIENT_DIR/lib, FCGI_CLIENT_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_FCGI_CLIENTLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong fcgi_client lib version or lib not found])
  dnl ],[
  dnl   -L$FCGI_CLIENT_DIR/lib -lm
  dnl ])
  dnl
  dnl PHP_SUBST(FCGI_CLIENT_SHARED_LIBADD)

  PHP_NEW_EXTENSION(fcgi_client, fcgi_client.c, $ext_shared)
fi
