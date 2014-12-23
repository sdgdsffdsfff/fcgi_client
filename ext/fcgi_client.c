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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_fcgi_client.h"
#include "php_network.h"

static int le_fcgi_client;

const zend_function_entry fcgi_client_functions[] = {
	PHP_FE(confirm_fcgi_client_compiled,	NULL)
	PHP_FE(fcgi_connect,	NULL)
	PHP_FE(fcgi_request,	NULL)
	PHP_FE_END
};

zend_module_entry fcgi_client_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"fcgi_client",
	fcgi_client_functions,
	PHP_MINIT(fcgi_client),
	PHP_MSHUTDOWN(fcgi_client),
	PHP_RINIT(fcgi_client),	
	PHP_RSHUTDOWN(fcgi_client),
	PHP_MINFO(fcgi_client),
#if ZEND_MODULE_API_NO >= 20010901
	PHP_FCGI_CLIENT_VERSION,
#endif
	STANDARD_MODULE_PROPERTIES
};


#ifdef COMPILE_DL_FCGI_CLIENT
ZEND_GET_MODULE(fcgi_client)
#endif


PHP_MINIT_FUNCTION(fcgi_client)
{
	return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(fcgi_client)
{
	return SUCCESS;
}

PHP_RINIT_FUNCTION(fcgi_client)
{
	return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(fcgi_client)
{
	return SUCCESS;
}

PHP_MINFO_FUNCTION(fcgi_client)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "fcgi_client support", "enabled");
	php_info_print_table_end();
}

PHP_FUNCTION(confirm_fcgi_client_compiled)
{
	char *arg = NULL;
	int arg_len, len;
	char *strg;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &arg, &arg_len) == FAILURE) {
		return;
	}

	len = spprintf(&strg, 0, "Congratulations! You have successfully modified ext/%.78s/config.m4. Module %.78s is now compiled into PHP.", "fcgi_client", arg);
	RETURN_STRINGL(strg, len, 0);
}

void fcgi_header_set_request_id(fcgi_header *h, uint16_t request_id)
{
    h->request_id_lo = BYTE_0(request_id);
    h->request_id_hi = BYTE_1(request_id);
}

fcgi_header* create_header(uchar type, uint16_t request_id)
{
    fcgi_header* tmp = (fcgi_header *) ecalloc(1,sizeof(fcgi_header));
    tmp->version = FCGI_VERSION_1;
    tmp->type    = type;
    fcgi_header_set_request_id(tmp, request_id);
    return tmp;
}

void serialize(char* buffer, void *st, size_t size)
{
   memcpy(buffer, st, size);
}

fcgi_begin_request* create_begin_request(uint16_t request_id)
{
    fcgi_begin_request *h = (fcgi_begin_request *) emalloc(sizeof(fcgi_begin_request));
    h->header = create_header(FCGI_BEGIN_REQUEST, request_id);
    h->body   = ecalloc(1, sizeof(fcgi_begin_request_body));
    // h->body->flags |= FCGI_KEEP_CONN;
    h->body->role_lo = FCGI_RESPONDER;
    h->header->content_len_lo = sizeof(fcgi_begin_request_body);
    return h;
}

uint32_t serialize_name_value(char* buffer, char* name, char *value)
{
    char *p = buffer;
    uint32_t nl, vl;
    nl = strlen(name);
    vl = strlen(value);

    if( nl < 128 )
        *p++ = BYTE_0(nl);
    else
    {
        *p++ = BYTE_0(nl);
        *p++ = BYTE_1(nl);
        *p++ = BYTE_2(nl);
        *p++ = BYTE_3(nl);
    }

    if ( vl < 128 )
        *p++ = BYTE_0(vl);
    else
    {
        *p++ = BYTE_0(vl);
        *p++ = BYTE_1(vl);
        *p++ = BYTE_2(vl);
        *p++ = BYTE_3(vl);
    }

    memcpy(p, name, nl);
    p+=nl;
    memcpy(p, value, vl);
    p+= vl;

    return p - buffer;
}

fcgi_record* fcgi_record_create()
{
    fcgi_record* tmp = (fcgi_record *) emalloc(sizeof(fcgi_record));
    tmp->header = (fcgi_header *) ecalloc(sizeof(fcgi_header), 1);
    tmp->next = NULL;
    tmp->state = 0;
    tmp->offset = 0;
    return tmp;
}

int fcgi_process_header(char ch,
        fcgi_record* rec)
{
    fcgi_header *h;
    fcgi_state *state = &(rec->state);
    h = rec->header;

    switch(*state){
        case fcgi_state_version:
            h->version = ch;
            *state = fcgi_state_type;
            break;
        case fcgi_state_type:
            h->type = ch;
            *state = fcgi_state_request_id_hi;
            break;
        case fcgi_state_request_id_hi:
            h->request_id_hi = ch;
            *state = fcgi_state_request_id_lo;
            break;
        case fcgi_state_request_id_lo:
            h->request_id_lo = ch;
            *state = fcgi_state_content_len_hi;
            break;
        case fcgi_state_content_len_hi:
            h->content_len_hi = ch;
            *state = fcgi_state_content_len_lo;
            break;
        case fcgi_state_content_len_lo:
            h->content_len_lo = ch;
            *state = fcgi_state_padding_len;
            break;
        case fcgi_state_padding_len:
            h->padding_len = ch;
            *state = fcgi_state_reserved;
            break;
        case fcgi_state_reserved:
            h->reserved = ch;
            *state = fcgi_state_content_begin;
            break;

        case fcgi_state_content_begin:
        case fcgi_state_content_proc:
        case fcgi_state_padding:
        case fcgi_state_done:
            return FCGI_PROCESS_DONE;
    }

    return FCGI_PROCESS_AGAIN;
}

uint32_t fcgi_header_get_content_len(fcgi_header *h)
{
    // php_printf("length %d %d\n", h->content_len_hi, h->content_len_lo);
    return   (h->content_len_hi << 8) + h->content_len_lo;
}

int fcgi_process_content(char **beg_buf, char *end_buf,
       fcgi_record *rec)
{

    size_t tot_len, con_len, cpy_len, offset, nb = end_buf - *beg_buf;
    fcgi_state *state = &(rec->state);
    fcgi_header *h = rec->header;
    offset = rec->offset;
    // php_printf("fcgi_process_content state %d\n", *state);

    if ( *state == fcgi_state_padding ){
        *state = fcgi_state_done;
        *beg_buf += (size_t) ((int)rec->length - (int)offset + (int)h->padding_len);
        return FCGI_PROCESS_DONE;
    }

    con_len = rec->length - offset;
    tot_len = con_len + h->padding_len;

    if(con_len <= nb)
        cpy_len = con_len;
    else
        cpy_len = nb;

    // php_printf("fcgi_process_content copy len:%d offse:%d\n", cpy_len, offset);
    memcpy(rec->content + offset, *beg_buf, cpy_len);

    if(tot_len <= nb)
    {
        rec->offset += tot_len;
        *state = fcgi_state_done;
        *beg_buf += tot_len;
        return FCGI_PROCESS_DONE;
    }
    else if( con_len <= nb )
    {
        /* Have to still skip all or some of padding */
        *state = fcgi_state_padding;
        rec->offset += nb;
        *beg_buf += nb;
        return FCGI_PROCESS_AGAIN;
    }
    else
    {  
        rec->offset += nb;
        *beg_buf += nb;
        return FCGI_PROCESS_AGAIN;
    }
    return 0;
}

int fcgi_process_record(char **beg_buf, char *end_buf, fcgi_record *rec)
{
    int rv;
    while(rec->state < fcgi_state_content_begin)
    {
        if((rv = fcgi_process_header(**beg_buf, rec)) == FCGI_PROCESS_ERR)
                return FCGI_PROCESS_ERR;
        (*beg_buf)++;
        if( *beg_buf == end_buf )
            return FCGI_PROCESS_AGAIN;
    }
    // php_printf("state %d\n", rec->state);
    if(rec->state == fcgi_state_content_begin)
    {
       rec->length = fcgi_header_get_content_len(rec->header);
       rec->content = emalloc(rec->length);
       rec->state++;
    }  
    // php_printf("rec %d %d %d\n", rec->length, rec->state, rec->offset);
    return fcgi_process_content(beg_buf, end_buf, rec);
}

void fcgi_process_buffer(char* beg_buf, char* end_buf,
       fcgi_record_list** head)
{

    fcgi_record* tmp, *h;
    size_t i;
    if(*head == NULL)
        *head = fcgi_record_create();
    h = *head;
    while(1)
    {
        if( h->state == fcgi_state_done )
        {
            tmp = h;
            *head = fcgi_record_create();
            h = *head;
            h->next = tmp;
        }
        int re = fcgi_process_record(&beg_buf, end_buf, h);
        // php_printf("result %d\n", re);
        // if( re == FCGI_PROCESS_DONE ){
        //     if(h->header->type == FCGI_STDOUT)
        //         for(i=0;i < h->length; i++)
        //             php_printf("%c", ((uchar *)h->content)[i]);
        // }


        if ( beg_buf == end_buf )
            return;
    }
}


void print_bytes(char *buf, int n)
{
    int i;
    php_printf("{");
    for(i=0;i<n;i++)
        php_printf("%02X.", buf[i]);
    php_printf("}\n");
}

PHP_FUNCTION(fcgi_request)
{
	zval *array;
	zval *conn;
	zval **fcgi_value;
	int ret;
	char *buffer = NULL;
	php_stream *stream;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zr", &array, &conn) == FAILURE) {
		RETURN_FALSE;
	}

	if (Z_TYPE_P(array) != IS_ARRAY) {
		RETURN_FALSE;
	}

	php_stream_from_zval_no_verify(stream, &conn);

	uint16_t req_id = 1;
    uint16_t len=0;
    int nb, i;
    char *p, *buf, *rbuf;
    fcgi_header* head;
    fcgi_begin_request* begin_req = create_begin_request(req_id);

    rbuf = emalloc(5000);
    buf  = emalloc(5000);
    p = buf;
    serialize(p, begin_req->header, sizeof(fcgi_header));
    p += sizeof(fcgi_header);
    // php_printf("write len %d\n", p-buf);
    serialize(p, begin_req->body, sizeof(fcgi_begin_request_body));
    p += sizeof(fcgi_begin_request_body);
    // php_printf("write len %d\n", p-buf);

    /* Sending fcgi_params */
    head = create_header(FCGI_PARAMS, req_id);

    len = 0;
    for(i = 0; i< zend_hash_num_elements(Z_ARRVAL_P(array)); i++) {
    	char* key;
    	ulong idx;
    	zend_hash_get_current_data(Z_ARRVAL_P(array), (void**) &fcgi_value);
    	convert_to_string_ex(fcgi_value);
    	if (zend_hash_get_current_key(Z_ARRVAL_P(array), &key, &idx, 0) == HASH_KEY_IS_STRING) {
            // php_printf("%s : %s\n", key, Z_STRVAL_PP(fcgi_value));
    		nb = serialize_name_value(p, key, Z_STRVAL_PP(fcgi_value));
        	len += nb;	
    	} else {
    		RETURN_FALSE; 
    	}
    	zend_hash_move_forward(Z_ARRVAL_P(array));
    }

    head->content_len_lo = BYTE_0(len);
    head->content_len_hi = BYTE_1(len);

    serialize(p, head, sizeof(fcgi_header));
    p += sizeof(fcgi_header);
    // php_printf("write len %d\n", p-buf);

    zend_hash_internal_pointer_reset(Z_ARRVAL_P(array));
    for(i = 0; i< zend_hash_num_elements(Z_ARRVAL_P(array)); i++) {
        char* key;
        ulong idx;
        zend_hash_get_current_data(Z_ARRVAL_P(array), (void**) &fcgi_value);
        convert_to_string_ex(fcgi_value);
        if (zend_hash_get_current_key(Z_ARRVAL_P(array), &key, &idx, 0) == HASH_KEY_IS_STRING) {
            // php_printf("%s : %s\n", key, Z_STRVAL_PP(fcgi_value));
            nb = serialize_name_value(p, key, Z_STRVAL_PP(fcgi_value));
            p += nb;  
        } else {
            RETURN_FALSE; 
        }
        zend_hash_move_forward(Z_ARRVAL_P(array));
    }

    head->content_len_lo = 0;
    head->content_len_hi = 0;

    serialize(p, head, sizeof(fcgi_header));
    p += sizeof(fcgi_header);
    
    
    // print_bytes(buf, p-buf);
    // php_printf("write len %d\n", p - buf);
    ret = php_stream_write(stream, buf, p - buf);
	if (buf) {
	   efree(buf);
	}
    // php_printf("write end\n");
	fcgi_record_list *rlst = NULL, *rec;

    while(1){
        // php_printf("read begin\n");
        if ((nb = php_stream_read(stream, rbuf, 5000-1)) == -1) {
            RETURN_FALSE;
        }
        // php_printf("read end\n");
        if(nb == 0)
            break;
        // print_bytes(rbuf, nb);
        fcgi_process_buffer(rbuf, rbuf+(size_t)nb, &rlst);
    }
    // php_printf("read parse end\n");

    for(rec=rlst; rec!=NULL; rec=rec->next)
    {
        if(rec->header->type == FCGI_STDOUT) {
            // php_printf("PADD<%d>", rec->header->padding_len);
            // php_printf("length %d\n", rec->length);
            // php_printf("content %s\n", (char *)rec->content);
            RETURN_STRINGL((char *)rec->content, rec->length, 0);
        }
    }

    RETURN_FALSE;
}

PHP_FUNCTION(fcgi_connect)
{
	char *host;
	int host_len;
	long port = -1;
	zval *zerrno = NULL, *zerrstr = NULL;
	double timeout = 10;
	unsigned long conv;
	struct timeval tv;
	char *hashkey = NULL;
	php_stream *stream = NULL;
	int err;
	char *hostname = NULL;
	long hostname_len;
	char *errstr = NULL;

	RETVAL_FALSE;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|lzzd", &host, &host_len, &port, &zerrno, &zerrstr, &timeout) == FAILURE) {
		RETURN_FALSE;
	}

	if (port > 0) {
		hostname_len = spprintf(&hostname, 0, "%s:%ld", host, port);
	} else {
		hostname_len = host_len;
		hostname = host;
	}
	
	/* prepare the timeout value for use */
	conv = (unsigned long) (timeout * 1000000.0);
	tv.tv_sec = conv / 1000000;
	tv.tv_usec = conv % 1000000;

	if (zerrno)	{
		zval_dtor(zerrno);
		ZVAL_LONG(zerrno, 0);
	}
	if (zerrstr) {
		zval_dtor(zerrstr);
		ZVAL_STRING(zerrstr, "", 1);
	}

	stream = php_stream_xport_create(hostname, hostname_len, REPORT_ERRORS,
			STREAM_XPORT_CLIENT | STREAM_XPORT_CONNECT, hashkey, &tv, NULL, &errstr, &err);

	if (port > 0) {
		efree(hostname);
	}
	if (stream == NULL) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "unable to connect to %s:%ld (%s)", host, port, errstr == NULL ? "Unknown error" : errstr);
	}

	if (hashkey) {
		efree(hashkey);
	}
	
	if (stream == NULL)	{
		if (zerrno) {
			zval_dtor(zerrno);
			ZVAL_LONG(zerrno, err);
		}
		if (zerrstr && errstr) {
			/* no need to dup; we need to efree buf anyway */
			zval_dtor(zerrstr);
			ZVAL_STRING(zerrstr, errstr, 0);
		}
		else if (!zerrstr && errstr) {
			efree(errstr);
		} 

		RETURN_FALSE;
	}

	if (errstr) {
		efree(errstr);
	}
		
	php_stream_to_zval(stream, return_value);
}



/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
