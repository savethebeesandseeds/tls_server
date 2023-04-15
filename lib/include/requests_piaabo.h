#ifndef __REQUEST_PIAABO
#define __REQUEST_PIAABO

#include <curl/curl.h>
#include <stdlib.h>
#include <pthread.h>
#include "util_piaabo.h"
#include "queue_piaabo.h"

#define CURL_VERBOSE true /* default value for CURL verbose */
pthread_mutex_t curl_lock=PTHREAD_MUTEX_INITIALIZER;

/* function pointers */
typedef size_t data_callback_pointer(void *data, size_t size, size_t nmemb, void *userp);
typedef size_t header_callback_pointer(char *buffer, size_t size, size_t nitems, void *userdata);

/* types of queues */
typedef __queue_t headers_queue_t;
typedef struct {char value[1<<12];} header_type_t;
typedef __queue_t params_queue_t;
typedef struct {char key[1<<12]; char value[1<<12];} param_type_t;

/* structures */
typedef struct {
  char *response;
  size_t size;
} request_memory_t;

typedef struct {
  const char* _method;                         /* required method, "GET", "POST", "DELETE",... */
  const char* _url;                            /* required url of the request (dont include the get parameters) */
  
  data_callback_pointer *_data_callback;       /* optional method to capture the request result data */
  void*_arg_data_callback;                     /* optional argument to pass the data_callback method */
  header_callback_pointer *_headers_callback;  /* optional method to process the request result headers */
  void* _arg_headers_callback;                 /* optional argument to pass the headers_callback method */

  headers_queue_t *_headers;                   /* optional vector of headers */
  params_queue_t *_params;                     /* optional map of GET params */
  void *_post_fileds;                          /* optional pointer to POST data or body */
  size_t _post_fileds_size;                    /* optional size of post_fileds data */

  bool _curl_verbose;                          /* optioanl */
  bool _trust_self_signed_server_ssl_tls;      /* optional */
  
  const char * _ssl_cert_type;                 /* optional */
  const char * _ssl_cert;                      /* optional */
  const char * _ssl_key;                       /* optional */
  const char * _ssl_key_password;              /* optional */

  struct curl_slist *_headers_list;            /* hidden */
  pthread_mutex_t _request_mutex;              /* hidden curl is thread safe this mutex is to safely change the request arguemts */
} request_config_t;
/* build request memory */
request_memory_t *build_request_memory(){
  request_memory_t* nreq_mem = (request_memory_t*)malloc(sizeof(request_memory_t));
  nreq_mem->response=NULL;
  nreq_mem->size=0;
  return nreq_mem;
}
/* free request memory */
void free_request_memory(request_memory_t *req_mem){
  free(req_mem->response);
  free(req_mem);
}
/* request config functions */
request_config_t * build_request_config(
  /* required parameters */
  const char *method,
  const char *url, 
  /* paremeters with default values */
  data_callback_pointer *data_callback,
  void* arg_data_callback,
  header_callback_pointer *headers_callback,
  void* arg_headers_callback,
  headers_queue_t *headers,
  params_queue_t *params,
  void *post_fileds,
  size_t post_fileds_size,
  bool curl_verbose,
  bool trust_self_signed_server_ssl_tls,
  const char * ssl_cert_type,
  const char * ssl_cert,
  const char * ssl_key,
  const char * ssl_key_password){
  /* request config struct allocation */
  request_config_t *nrc = (request_config_t *)malloc(sizeof(request_config_t));
  /* struct initialization */
  nrc->_method = method;                             /* _method */
  nrc->_url = url;                                   /* _url */
  nrc->_data_callback = data_callback;               /* _data_callback */
  nrc->_arg_data_callback = arg_data_callback;       /* _arg_data_callback */
  nrc->_headers_callback = headers_callback;         /* _headers_callback */
  nrc->_arg_headers_callback = arg_headers_callback; /* _arg_headers_callback */
  nrc->_headers = headers;                           /* _headers */
  nrc->_params = params;                             /* _params */
  nrc->_post_fileds = post_fileds;                   /* _post_fileds */
  nrc->_post_fileds_size = post_fileds_size;         /* _post_fileds_size */
  nrc->_curl_verbose = curl_verbose;                 /* _curl_verbose */
  nrc->_trust_self_signed_server_ssl_tls = 
    trust_self_signed_server_ssl_tls;                /* _trust_self_signed_server_ssl_tls */
  nrc->_ssl_cert_type = ssl_cert_type;               /* _ssl_cert_type */
  nrc->_ssl_cert = ssl_cert;                         /* _ssl_cert */
  nrc->_ssl_key = ssl_key;                           /* _ssl_key */
  nrc->_ssl_key_password = ssl_key_password;         /* _ssl_key_password */
  nrc->_headers_list = NULL;                         /* _headers_list */
  pthread_mutex_init(&nrc->_request_mutex, NULL);    /* _request_mutex */
  
  /* return the new request object instance */
  return nrc;
}
/* initialize_request_config */
request_config_t *initialize_request_config(
  /* required parameters */
  const char *method,
  const char *url){
  /* paremeters with default values */
  data_callback_pointer *data_callback=NULL;
  void* arg_data_callback=NULL;
  header_callback_pointer *headers_callback=NULL;
  void* arg_headers_callback=NULL;
  headers_queue_t *headers = queue_fabric();
  params_queue_t *params = queue_fabric();
  void *post_fileds=NULL;
  size_t post_fileds_size=0;
  bool curl_verbose=CURL_VERBOSE;
  bool trust_self_signed_server_ssl_tls=false;
  const char * ssl_cert_type=NULL;
  const char * ssl_cert=NULL;
  const char * ssl_key=NULL;
  const char * ssl_key_password=NULL;

  return build_request_config(
    /* required parameters */ method,url, /* paremeters with default values */
    data_callback,arg_data_callback,headers_callback,
    arg_headers_callback,headers,params,post_fileds,post_fileds_size,curl_verbose,
    trust_self_signed_server_ssl_tls,ssl_cert_type,ssl_cert,ssl_key,ssl_key_password);
}
/* free the memory of REQUEST_CONFIG */
void free_request_config(request_config_t *rc){
  /* finalize queues */
  queue_destructor(rc->_headers);
  queue_destructor(rc->_params);
  /* free the headers list */
  if(rc->_headers_list!=NULL){
    curl_slist_free_all(rc->_headers_list);
  }
  /* finalize the mutex */
  pthread_mutex_destroy(&rc->_request_mutex);
  /* free the request config */
  free(rc);
}
/* add header functionality */
void add_header(request_config_t *rc, const char *dheader){
  if(rc==NULL){
    log_warn("Null reqest_config_t, hint: use initialize_request_config()\n");
    return;
  }
  /* create new header_type_t */
  header_type_t *header = (header_type_t*) malloc(sizeof(header_type_t));
  memset(header, 0, sizeof(header_type_t));
  header->value[0] = '\0';
  memcpy(header->value, dheader, sizeof(char)*strlen(dheader));
  /* add header to the queue */
  queue_insert_item_on_top(rc->_headers, header, sizeof(header_type_t), free); // the queue will free the memory on queue_destructor
}
/* add param functionality */
void add_param(request_config_t *rc, const char *dkey, const char *dvalue){
  if(rc==NULL){
    log_warn("Null reqest_config_t, hint: use initialize_request_config()\n");
    return;
  }
  /* create new param_type_t */
  param_type_t *param = (param_type_t*) malloc(sizeof(param_type_t));
  memset(param, 0, sizeof(param_type_t));
  param->key[0] = '\0';
  param->value[0] = '\0';
  memcpy(param->key, dkey, sizeof(char)*strlen(dkey));
  memcpy(param->value, dvalue, sizeof(char)*strlen(dvalue));
  /* add param to the queue */
  queue_insert_item_on_top(rc->_params, param, sizeof(param_type_t), free); // the queue will free the memory on queue_destructor
}
/* url_encode_string */
static void url_encode_string(CURL *curl_handle, char *string, char **encoded_string){
  /* fabricate encoded string */
  char *temp = curl_easy_escape(curl_handle, string, 0);
  /* fabricate encoded string */
  strcpy(*encoded_string,temp);
  /* free the encoded char* */
  curl_free(temp);
}
/* encode_get_params */
static void encode_get_params(CURL *curl_handle, params_queue_t *params, char **encoded_params){
  /* validate if there are any params */
  if(queue_is_empty(params))
    return;
  /* temporal variables */
  char *temp_in=(char*)malloc(65536);
  char *temp_out=(char*)malloc(65536);
  char *temp_key;
  char *temp_value;
  bool is_first=true;
  memset(temp_in, 0, 65536);
  memset(temp_out, 0, 65536);
  /* initialize encoded_params variable */
  memset(*encoded_params, 0, strlen(*encoded_params));
  __queue_item_t *item=NULL;
  queue_to_base(params);
  while((item=queue_to_next(params)) != NULL){
    /* clear the temporary variables */
    memset(temp_in, 0, sizeof(char)*strlen(temp_in));
    memset(temp_out, 0, sizeof(char)*strlen(temp_out));
    /* fabricate the query parameter */
    temp_key=((param_type_t*)item->data)->key;
    temp_value=((param_type_t*)item->data)->value;
    /* fabricate the string */
    if(temp_key != NULL && temp_value != NULL){
      strcat(temp_in, temp_key);
      strcat(temp_in,"=");
      strcat(temp_in,temp_value);
    } else if(temp_key != NULL){
      strcat(temp_in,temp_key);
    } else if(temp_value != NULL){
      strcat(temp_in,temp_value);
    }
    /* encode the string */
    url_encode_string(curl_handle, temp_in, &temp_out);
    /* append the symbol & */
    if(!is_first)
      strcat(*encoded_params, "&");
    /* append the encoded string */
    if(strlen(temp_out)>0){  
      strcat(*encoded_params, temp_out);
      is_first=false;
    }
  }
  /* free the temporal pointers */
  free(temp_in);
  free(temp_out);
}
/* configure the curl_handle acording to options */
void configure_curl(CURL *curl_handle, unsigned int option, request_config_t *rc){
  /* temporal variables */
  char *url=(char*)malloc(65536);
  char *encoded_url=(char*)malloc(65536);
  header_type_t *temp_header=(header_type_t*)malloc(sizeof(header_type_t));
  memset(url, 0, 65536);
  memset(encoded_url, 0, 65536);
  /* case options */
  switch (option){
  /*  
    * Set the http method:
    *                              https://curl.se/libcurl/c/CURLOPT_CUSTOMREQUEST.html
    */
  case CURLOPT_CUSTOMREQUEST:
    curl_easy_setopt(curl_handle, CURLOPT_CUSTOMREQUEST, rc->_method);
  /*  
    * Enable the Verbose for CURL
    *                              https://curl.se/libcurl/c/CURLOPT_VERBOSE.html
    */
  case CURLOPT_VERBOSE:
    curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, (int)rc->_curl_verbose);
    break;
  /*  
    * Encode GET parameters 
    *                              https://curl.se/libcurl/c/CURLOPT_URL.html
    */
  case CURLOPT_URL:
    strcpy(url, rc->_url);
    encode_get_params(curl_handle, rc->_params, &encoded_url);
    if(strlen(encoded_url)>0){
      strcat(url, "?");
      strcat(url, encoded_url);
    }
    curl_easy_setopt(curl_handle, CURLOPT_URL, url);
    break;
  /*  
    * Callback for the response data 
    *                              https://curl.se/libcurl/c/CURLOPT_WRITEFUNCTION.html
    */
  case CURLOPT_WRITEFUNCTION:
    if(rc->_data_callback!=NULL)
      curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, rc->_data_callback);
    else
      log_warn("CURLOPT_WRITEFUNCTION required with NULL pointer.\n");
    break;
  /*  
    * Argument for the response data callback 
    *                              https://curl.se/libcurl/c/CURLOPT_WRITEDATA.html
    */
  case CURLOPT_WRITEDATA:
    if(rc->_arg_data_callback!=NULL)
      curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void*)rc->_arg_data_callback);
    else
      log_warn("CURLOPT_WRITEDATA required with NULL pointer.\n");
    break;
  /*  
    * Callback for the response headers 
    *                              https://curl.se/libcurl/c/CURLOPT_HEADERFUNCTION.html
    */
  case CURLOPT_HEADERFUNCTION:
    if(rc->_headers_callback!=NULL)
      curl_easy_setopt(curl_handle, CURLOPT_HEADERFUNCTION, rc->_headers_callback);
    else
      log_warn("CURLOPT_HEADERFUNCTION required with NULL pointer.\n");
    break;
  /*  
    * Argument for the response headers callback 
    *                              https://curl.se/libcurl/c/CURLOPT_HEADERDATA.html
    */
  case CURLOPT_HEADERDATA:
    if(rc->_arg_headers_callback!=NULL)
      curl_easy_setopt(curl_handle, CURLOPT_HEADERDATA, (void*)rc->_arg_headers_callback);
    else
      log_warn("CURLOPT_HEADERDATA required with NULL pointer.\n");
    break;
  /*  
    * Set up the request headers 
    *                              https://curl.se/libcurl/c/CURLOPT_HTTPHEADER.html
    */
  case CURLOPT_HTTPHEADER:
    if(rc->_headers!=NULL){
      __queue_item_t *item=NULL;
      queue_to_base(rc->_headers);
      while((item=queue_to_next(rc->_headers)) != NULL){
        memcpy(temp_header, (header_type_t*)item->data, sizeof(header_type_t));
        if(temp_header!=NULL && temp_header->value[0]!='\0' && strlen(temp_header->value)>0)
          rc->_headers_list = curl_slist_append(rc->_headers_list, temp_header->value);
      }
      curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, rc->_headers_list);
    } else {
      log_warn("CURLOPT_HTTPHEADER required with NULL pointer.\n");
    }
    break;
  /*  
    * Set up for a POST request the data
    *                              https://curl.se/libcurl/c/CURLOPT_POSTFIELDS.html
    */
  case CURLOPT_POSTFIELDS:
    if(rc->_post_fileds!=NULL)
      curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDSIZE, rc->_post_fileds);
    else
      log_warn("CURLOPT_POSTFIELDS required with NULL pointer.\n");
    break;
  /*  
    * Set up for a POST request the size of the data 
    *                              https://curl.se/libcurl/c/CURLOPT_POSTFIELDSIZE.html
    */
  case CURLOPT_POSTFIELDSIZE:
    curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, rc->_post_fileds_size);
    break;
  /*  
    * Trust servers with self signed ssl certificates
    *                              https://curl.se/libcurl/c/CURLOPT_SSL_VERIFYPEER.html
    */
  case CURLOPT_SSL_VERIFYPEER:
    if(rc->_trust_self_signed_server_ssl_tls){
      curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0L);
      log_warn("Request trusting servers with self signed SSL/TLS certificates.\n");
    }
    break;
  /*  
    * Parse the SSL/TLS client certificate Type
    * Supported formats are "PEM" and "DER", except with Secure Transport or Schannel.
    *                              https://curl.se/libcurl/c/CURLOPT_SSLCERTTYPE.html
    */
  case CURLOPT_SSLCERTTYPE:
    if(rc->_ssl_cert_type!=NULL)
      curl_easy_setopt(curl_handle, CURLOPT_SSLCERTTYPE, rc->_ssl_cert_type);
    else
      log_warn("CURLOPT_SSLCERTTYPE required with NULL pointer\n");
    break;
  /*  
    *  Parse the SSL/TLS client certificate File Path
    *                              https://curl.se/libcurl/c/CURLOPT_SSLCERT.html
    */
  case CURLOPT_SSLCERT:
    if(rc->_ssl_cert!=NULL)
      curl_easy_setopt(curl_handle, CURLOPT_SSLCERT, rc->_ssl_cert);
    else
      log_warn("CURLOPT_SSLCERT required with NULL pointer\n");
    break;
  /*  
    *  Specify the SSL/TLS client certificate key file 
    *                              https://curl.se/libcurl/c/CURLOPT_SSLKEY.html
    */
  case CURLOPT_SSLKEY:
    if(rc->_ssl_key!=NULL)
      curl_easy_setopt(curl_handle, CURLOPT_SSLKEY, rc->_ssl_key);
    else
      log_warn("CURLOPT_SSLKEY required with NULL pointer\n");
    break;
  /*  
    *  Specify the SSL/TLS client certficiate file password
    *                              https://curl.se/libcurl/c/CURLOPT_KEYPASSWD.html
    */
  case CURLOPT_KEYPASSWD:
    if(rc->_ssl_key_password!=NULL)
      curl_easy_setopt(curl_handle, CURLOPT_KEYPASSWD, rc->_ssl_key_password);
    else
      log_warn("CURLOPT_KEYPASSWD required with NULL pointer\n");
    break;
  /*  */
  default:
    break;
  }
  /* free the temporal variables */
  free(url);
  free(encoded_url);
  free(temp_header);
}
/* finalize_configure_curl */
void finalize_configure_curl(request_config_t *rc){
  if(rc->_headers_list!=NULL)
    curl_slist_free_all(rc->_headers_list);
}
/* request module finalization */
static void request_finit(){
  curl_global_cleanup();
}
/* request module initialization,
 * automatically called when the library request_piaabo.h is imported
 */
void __attribute__((constructor)) request_init(void) {
  /* initialize curl */
  curl_global_init(CURL_GLOBAL_DEFAULT);
  /* registers a function to be called when the program ends */
  atexit(request_finit);
}
/* configure a basic post request */
static void basic_post_query(CURL *curl_handle, request_config_t *config){
  /* configure the POST request */
  configure_curl(curl_handle, CURLOPT_VERBOSE, config);
  configure_curl(curl_handle, CURLOPT_CUSTOMREQUEST, config);
  configure_curl(curl_handle, CURLOPT_URL, config);
  configure_curl(curl_handle, CURLOPT_WRITEFUNCTION, config);
  configure_curl(curl_handle, CURLOPT_WRITEDATA, config);
  configure_curl(curl_handle, CURLOPT_HEADERFUNCTION, config);
  configure_curl(curl_handle, CURLOPT_HEADERDATA, config);
  configure_curl(curl_handle, CURLOPT_HTTPHEADER, config);
  configure_curl(curl_handle, CURLOPT_POSTFIELDS, config);
  configure_curl(curl_handle, CURLOPT_POSTFIELDSIZE, config);
  configure_curl(curl_handle, CURLOPT_SSL_VERIFYPEER, config);
  configure_curl(curl_handle, CURLOPT_SSLCERT, config);
  configure_curl(curl_handle, CURLOPT_SSLCERTTYPE, config);
  configure_curl(curl_handle, CURLOPT_SSLKEY, config);
  configure_curl(curl_handle, CURLOPT_KEYPASSWD, config);
}
/* configure a basict get request */
static void basic_get_query(CURL *curl_handle, request_config_t *config){
  /* configure the GET request */
  configure_curl(curl_handle, CURLOPT_VERBOSE, config);
  configure_curl(curl_handle, CURLOPT_CUSTOMREQUEST, config);
  configure_curl(curl_handle, CURLOPT_URL, config);
  configure_curl(curl_handle, CURLOPT_WRITEFUNCTION, config);
  configure_curl(curl_handle, CURLOPT_WRITEDATA, config);
  configure_curl(curl_handle, CURLOPT_HEADERFUNCTION, config);
  configure_curl(curl_handle, CURLOPT_HEADERDATA, config);
  configure_curl(curl_handle, CURLOPT_HTTPHEADER, config);
  configure_curl(curl_handle, CURLOPT_SSL_VERIFYPEER, config);
  configure_curl(curl_handle, CURLOPT_SSLCERTTYPE, config);
  configure_curl(curl_handle, CURLOPT_SSLCERT, config);
  configure_curl(curl_handle, CURLOPT_SSLKEY, config);
  configure_curl(curl_handle, CURLOPT_KEYPASSWD, config);
}
/* make a HTTP request */
static void http_request(request_config_t *config){
  /* lock the mutex */
  pthread_mutex_lock(&config->_request_mutex);
  /* create a handle */
  CURL *curl_handle=curl_easy_init();
  /* check the handle */
  if(!curl_handle){
    log_err("Unable to perform http_request, unable to initialize curl");
    return;
  }
  /* configure thre request */
  if(strcmp(config->_method,"GET")==0)
    basic_get_query(curl_handle, config);
  else
    basic_post_query(curl_handle, config);
  /* lock thread */
  pthread_mutex_lock(&curl_lock);
  /* perform the operation */
  CURLcode curl_code = curl_easy_perform(curl_handle);
  /* unlock thread */
  pthread_mutex_unlock(&curl_lock);
  /* check for errors */
  if(curl_code!=CURLE_OK)
    log_err("curl failed to perform [basic_get_query]: %s\n",curl_easy_strerror(curl_code));
  /* finalize the request list and queues */
  finalize_configure_curl(config);
  /* clean the handle */
  curl_easy_cleanup(curl_handle);
  /* free the mutex lock */
  pthread_mutex_unlock(&config->_request_mutex);
}
/* default method for data_callback_text, it concatenates the data as it arrives */
size_t data_callback_text(void *data, size_t size, size_t nmemb, void *userp){
  /* calculate incoming memory size */
  size_t realsize = size * nmemb;
  /* cast struct */
  request_memory_t *mem = (request_memory_t *)userp;
  /* reallocate memory */
  char *ptr = (char*)realloc(mem->response, mem->size + realsize + 1);
  /* validate memory allocation */
  if(ptr == NULL)
    return 0;  /* out of memory! */
  /* assign data */
  mem->response = ptr;
  /* transfer the old data */
  memcpy(&(mem->response[mem->size]), data, realsize);
  /* assign size */
  mem->size += realsize;
  /* mark termination byte */
  mem->response[mem->size] = 0;
  /* return success */
  return realsize;
}
/* default method for header_callback, it prints the response headers */
size_t header_callback(char *buffer, size_t size, size_t nitems, void *userdata){
  if(strcmp(buffer,"")!=0 && strcmp(buffer,"\n")!=0 && strcmp(buffer,"\0")!=0 && strlen(buffer)>2){
    log("HEADER: \t%s",buffer);
  }
  return nitems * size;
}
#endif