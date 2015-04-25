/*
 *  HTTPCWS - HTTPCWS is an Chinese Word Segmentation System Based on the HTTP protocol.
 *
 *       http://code.google.com/p/httpcws/
 *
 *  Copyright 2009-2010 Zhang Yan.  All rights reserved.
 *
 *  Use and distribution licensed under the BSD license. 
 *
 *  Authors:
 *      Zhang Yan <net@s135.com> http://blog.s135.com
 */

extern "C" {
#include <sys/types.h>
#include <sys/time.h>
#include <sys/queue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <time.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <assert.h>
#include <signal.h>
#include <stdbool.h>
#include <wait.h>

#include <err.h>
#include <event.h>
#include <evhttp.h>

#define VERSION "1.1.0"
}
#include "ICTCLAS/ICTCLAS30.h"

/* ȥ���ַ������˵Ŀո� */
void trim( char *str )
{
        char *copied, *tail = NULL;

        if ( str == NULL )
                return;

        for( copied = str; *str; str++ )
        {
                if ( *str != ' ' && *str != '\t' )
                {
                        *copied++ = *str;
                         tail = copied;
                }
                else
                {
                         if ( tail )
                                 *copied++ = *str;
                }
        }

        if ( tail )
             *tail = 0;
        else
             *copied = 0;

        return;
}

char *urldecode(char *in)
{
    char  *out;
    char   temp[32];
    char  *p, *q;
    int             value;

    out = (char *) malloc(strlen(in) + 1);

    p = in, q = out;
    while (*p != 0) {
        if (*p == '+') {
            *q++ = ' ';
        } else if (*p == '%') {
            temp[0] = *(p + 1);
            temp[1] = *(p + 2);
            temp[2] = 0;
            sscanf(temp, "%x", &value);

            *q++ = 0xff & value;

            p += 2;
        } else {
            *q++ = *p;
        }

        p++;
    }
    *q = '\0';

    return (char *) out;
}

static void show_help(void)
{ 
	char *b = "HTTPCWS v" VERSION " written by Zhang Yan (http://blog.s135.com)\n"
		  "\n"
		   "-l <ip_addr>  interface to listen on, default is 0.0.0.0\n"
		   "-p <num>      TCP port number to listen on (default: 1985)\n"
		   "-x <path>     Chinese dictionary path (example: /usr/local/httpcws/dict)\n"
		   "-t <second>   timeout for an http request (default: 120)\n"		   
		   "-d            run as a daemon\n"
		   "-h            print this help and exit\n"	   
		   "\n";
	fprintf(stderr, b, strlen(b));
}

/* ����ģ�� */
void httpcws_handler(struct evhttp_request *req, void *arg)
{	
        struct evbuffer *buf;
        buf = evbuffer_new();
		
		/* ����URL���� */
		struct evkeyvalq httpcws_http_query;
		evhttp_parse_query(evhttp_request_uri(req), &httpcws_http_query);
		
		/* ����POST����Ϣ */
		const char *tcsql_input_postbuffer = (const char*) EVBUFFER_DATA(req->input_buffer);		
		
		/* ����GET������ */
		const char *httpcws_input_words = evhttp_find_header (&httpcws_http_query, "w");
		const char *httpcws_input_tag = evhttp_find_header (&httpcws_http_query, "t");

		int httpcws_input_tagint = 0; /* Ĭ�Ϲرմ��Ա�ע */
		if (httpcws_input_tag != NULL) {
			if (strcmp(httpcws_input_tag, "1") == 0) {
				httpcws_input_tagint = 1; /* �������Ա�ע */
			}
		}
		
		const char *httpcws_output_tmp = NULL;
		char *httpcws_output_words = "\0";
		if (tcsql_input_postbuffer != NULL) {
			char *tcsql_input_postbuffer_tmp = (char *) malloc(EVBUFFER_LENGTH(req->input_buffer)+1);
			memset (tcsql_input_postbuffer_tmp, '\0', EVBUFFER_LENGTH(req->input_buffer)+1);
			strncpy(tcsql_input_postbuffer_tmp, tcsql_input_postbuffer, EVBUFFER_LENGTH(req->input_buffer));
			char *decode_uri = urldecode(tcsql_input_postbuffer_tmp);
			free(tcsql_input_postbuffer_tmp);
			httpcws_output_tmp = ICTCLAS_ParagraphProcess(decode_uri, httpcws_input_tagint);
			free(decode_uri);
			httpcws_output_words = strdup(httpcws_output_tmp);
			trim (httpcws_output_words);
		} else if (httpcws_input_words != NULL) {
			char *httpcws_input_words_tmp = strdup(httpcws_input_words);
			char *decode_uri = urldecode(httpcws_input_words_tmp);
			free(httpcws_input_words_tmp);
			httpcws_output_tmp = ICTCLAS_ParagraphProcess(decode_uri, httpcws_input_tagint);
			free(decode_uri);
			httpcws_output_words = strdup(httpcws_output_tmp);
			trim (httpcws_output_words);
		} else {
			httpcws_output_words = strdup("");
		}
		
		/* ������ݸ��ͻ��� */
		evhttp_add_header(req->output_headers, "Server", "HTTPCWS/1.1.0");
		evhttp_add_header(req->output_headers, "Content-Type", "text/plain; charset=GB2312");
		evhttp_add_header(req->output_headers, "Keep-Alive", "120");
		evbuffer_add_printf(buf, "%s", httpcws_output_words);
        evhttp_send_reply(req, HTTP_OK, "OK", buf);
		
		free(httpcws_output_words);
		evhttp_clear_headers(&httpcws_http_query);
		evbuffer_free(buf);	
}

int main(int argc, char **argv)
{
	int c;
	/* Ĭ�ϲ������� */
	char *httpcws_settings_listen = "0.0.0.0";
	int httpcws_settings_port = 1985;
	char *httpcws_settings_datapath = NULL; /*���Ĵʵ����ݿ�·�� */
	bool httpcws_settings_daemon = false;
	int httpcws_settings_timeout = 120; /* ��λ���� */

    /* process arguments */
    while ((c = getopt(argc, argv, "l:p:x:t:dh")) != -1) {
        switch (c) {
        case 'l':
            httpcws_settings_listen = strdup(optarg);
            break;
        case 'p':
            httpcws_settings_port = atoi(optarg);
            break;
        case 'x':
            httpcws_settings_datapath = strdup(optarg); /* �ʿ��ļ��洢·�� */
            break;
        case 't':
            httpcws_settings_timeout = atoi(optarg);
            break;			
        case 'd':
            httpcws_settings_daemon = true;
            break;
		case 'h':			
        default:
            show_help();
            return 1;
        }
    }
	
	/* �ж��Ƿ���˱������ -x */
	if (httpcws_settings_datapath == NULL) {
		show_help();
		fprintf(stderr, "Attention: Please use the indispensable argument: -x <path>\n\n");		
		exit(1);
	}

	/* ��ʼ���ִ���� */
	if(!ICTCLAS_Init(httpcws_settings_datapath))
	{
		printf("%s\n", httpcws_settings_datapath);
		fprintf(stderr, "ERROR: Count not open the Chinese dictionary!\n\n");		
		exit(1);
	}
	ICTCLAS_SetPOSmap(ICT_POS_MAP_SECOND);

	fprintf(stderr, "Loading Chinese dictionary 'httpcws_dict.txt' into memory, please waitting ......\n");
	char *httpcws_settings_dataname = (char *)malloc(1024);
	memset (httpcws_settings_dataname, '\0', 1024);
	sprintf(httpcws_settings_dataname, "%s/httpcws_dict.txt", httpcws_settings_datapath);
	int nCount = ICTCLAS_ImportUserDict(httpcws_settings_dataname);
	if (nCount > 0) {
		ICTCLAS_SaveTheUsrDic();
	}
	free(httpcws_settings_dataname);
	printf("OK! %d words has loaded into memory.\n\n", nCount);
	printf("HTTPCWS Server running on %s:%d\n", httpcws_settings_listen, httpcws_settings_port);

	/* �������-d���������ػ��������� */
	if (httpcws_settings_daemon == true){
        pid_t pid;

        /* Fork off the parent process */       
        pid = fork();
        if (pid < 0) {
                exit(EXIT_FAILURE);
        }
        /* If we got a good PID, then
           we can exit the parent process. */
        if (pid > 0) {
                exit(EXIT_SUCCESS);
        }
	}
	
	/* �������� */
	struct evhttp *httpd;

	event_init();
	httpd = evhttp_start(httpcws_settings_listen, httpcws_settings_port);
	evhttp_set_timeout(httpd, httpcws_settings_timeout);

	/* Set a callback for all other requests. */
	evhttp_set_gencb(httpd, httpcws_handler, NULL);

	event_dispatch();

	/* Not reached in this code as it is now. */
	evhttp_free(httpd);

    return 0;
}


