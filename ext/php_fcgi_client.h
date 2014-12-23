/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2013 The PHP Group                                |
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

#ifndef PHP_FCGI_CLIENT_H
#define PHP_FCGI_CLIENT_H

extern zend_module_entry fcgi_client_module_entry;
#define phpext_fcgi_client_ptr &fcgi_client_module_entry

#define PHP_FCGI_CLIENT_VERSION "0.1.0"

#ifdef PHP_WIN32
#	define PHP_FCGI_CLIENT_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_FCGI_CLIENT_API __attribute__ ((visibility("default")))
#else
#	define PHP_FCGI_CLIENT_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

/**
 * fcgi define begin
 */
#define FCGI_LISTENSOCK_FILENO 0
#define FCGI_VERSION_1         1

#define FCGI_BEGIN_REQUEST       1
#define FCGI_ABORT_REQUEST       2
#define FCGI_END_REQUEST         3
#define FCGI_PARAMS              4
#define FCGI_STDIN               5
#define FCGI_STDOUT              6
#define FCGI_STDERR              7
#define FCGI_DATA                8
#define FCGI_GET_VALUES          9
#define FCGI_GET_VALUES_RESULT  10
#define FCGI_UNKNOWN_TYPE       11
#define FCGI_MAXTYPE (FCGI_UNKNOWN_TYPE)

#define FCGI_NULL_REQUEST_ID     0

typedef struct {
    unsigned char version;
    unsigned char type;
    unsigned char request_id_hi;
    unsigned char request_id_lo;
    unsigned char content_len_hi;
    unsigned char content_len_lo;
    unsigned char padding_len;
    unsigned char reserved;
} fcgi_header;

/* Generic structure for both name and value
 * Does not contain the length values since
 */
typedef struct {
    char *name, *value;
} fcgi_name_value;

/* Management records */
typedef struct {
    unsigned char type;    
    unsigned char reserved[7];
} fcgi_unknown_type_body;

typedef struct {
    fcgi_header header;
    fcgi_unknown_type_body body;
} fcgi_unknown_type;

/* Variable names for FCGI_GET_VALUES / FCGI_GET_VALUES_RESULT */
#define FCGI_MAX_CONNS  "FCGI_MAX_CONNS"
#define FCGI_MAX_REQS   "FCGI_MAX_REQS"
#define FCGI_MPXS_CONNS "FCGI_MPXS_CONNS"

/* Application records */

/* Mask for flags in fcgi_begin_request_body */
#define FCGI_KEEP_CONN  1
/*
 * Values for role component of fcgi_begin_request
 */
#define FCGI_RESPONDER  1
#define FCGI_AUTHORIZER 2
#define FCGI_FILTER     3

typedef struct {
    unsigned char  role_hi;
    unsigned char  role_lo;
    unsigned char  flags;
    unsigned char  reserved[5];
} fcgi_begin_request_body;

typedef struct {
    fcgi_header *header;
    fcgi_begin_request_body *body;
} fcgi_begin_request;


typedef struct {
    unsigned char app_status;
    unsigned char appStatusB2;
    unsigned char appStatusB1;
    unsigned char appStatusB0;
    unsigned char protocol_status;
    unsigned char reserved[3];
} fcgi_end_request_body;

typedef struct {
    fcgi_header header;
    fcgi_end_request_body body;
} fcgi_end_request;

/*
 * Values for protocolStatus component of FCGI_EndRequestBody
 */
#define FCGI_REQUEST_COMPLETE 0
#define FCGI_CANT_MPX_CONN    1
#define FCGI_OVERLOADED       2
#define FCGI_UNKNOWN_ROLE     3

/* Bytes from LSB to MSB 0..3 */
#define BYTE_0(x) ((x) & 0xff)
#define BYTE_1(x) ((x)>>8 & 0xff)
#define BYTE_2(x) ((x)>>16 & 0xff)
#define BYTE_3(x) ((x)>>24 & 0xff)

typedef unsigned char uchar;
typedef enum{
    fcgi_state_version = 0,
    fcgi_state_type,
    fcgi_state_request_id_hi,
    fcgi_state_request_id_lo,
    fcgi_state_content_len_hi,
    fcgi_state_content_len_lo,
    fcgi_state_padding_len,
    fcgi_state_reserved,
    fcgi_state_content_begin,
    fcgi_state_content_proc,
    fcgi_state_padding,
    fcgi_state_done
} fcgi_state;

typedef struct fcgi_record_{
    fcgi_header* header;
    void *content;
    size_t offset, length;
    fcgi_state state;
    struct fcgi_record_* next;
} fcgi_record;

typedef fcgi_record fcgi_record_list;

#define FCGI_PROCESS_AGAIN 1
#define FCGI_PROCESS_DONE 2
#define FCGI_PROCESS_ERR 3
/**
 * fcgi define end
 */

PHP_MINIT_FUNCTION(fcgi_client);
PHP_MSHUTDOWN_FUNCTION(fcgi_client);
PHP_RINIT_FUNCTION(fcgi_client);
PHP_RSHUTDOWN_FUNCTION(fcgi_client);
PHP_MINFO_FUNCTION(fcgi_client);

PHP_FUNCTION(confirm_fcgi_client_compiled);
PHP_FUNCTION(fcgi_connect);
PHP_FUNCTION(fcgi_request);


#ifdef ZTS
#define FCGI_CLIENT_G(v) TSRMG(fcgi_client_globals_id, zend_fcgi_client_globals *, v)
#else
#define FCGI_CLIENT_G(v) (fcgi_client_globals.v)
#endif

#endif	/* PHP_FCGI_CLIENT_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
