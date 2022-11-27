#ifndef __URL_PIAABO
#define __URL_PIAABO

#include <curl/curl.h>
#include <stdexcept>
#include <memory>
#include <vector>
#include <cstring>
#include <map>

#include "util_piaabo.h"

pthread_mutex_t curl_lock=PTHREAD_MUTEX_INITIALIZER;

/* function pointers */
typedef size_t data_callback_pointer(void *data, size_t size, size_t nmemb, void *userp);
typedef size_t header_callback_pointer(char *buffer, size_t size, size_t nitems, void *userdata);

class REQUESTS {
public:
/* substructures */
struct memory {
  char *response;
  size_t size;
};
/* subclasses */
class REQUEST_CONFIG {
public:
  const char* _method;                         /* required method, "GET", "POST", "DELETE",... */
  const char* _url;                            /* required url of the request (dont include the get parameters) */
  
  data_callback_pointer *_data_callback;       /* optional method to capture the request result data */
  void*_arg_data_callback;                     /* optional argument to pass the data_callback method */
  header_callback_pointer *_headers_callback;  /* optional method to porcess the request result headers */
  void* _arg_headers_callback;                 /* optional argument to pass the headers_callback method */

  std::vector<std::string> *_headers;          /* optional vector of headers */
  std::map<std::string, std::string> *_params; /* optional map of GET params */
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
public:
  REQUEST_CONFIG(
    /* required parameters */
    const char *method,
    const char *url, 
    /* paremeters with default values */
    data_callback_pointer *data_callback=NULL,
    void* arg_data_callback=NULL,
    header_callback_pointer *headers_callback=NULL,
    void* arg_headers_callback=NULL,
    std::vector<std::string> *headers=NULL,
    std::map<std::string, std::string> *params=NULL,
    void *post_fileds=NULL,
    size_t post_fileds_size=0,
    bool curl_verbose=false,
    bool trust_self_signed_server_ssl_tls=false,
    const char * ssl_cert_type=NULL,
    const char * ssl_cert=NULL,
    const char * ssl_key=NULL,
    const char * ssl_key_password=NULL):
    /* class initialization */
    _method(method),
    _url(url),
    
    _data_callback(data_callback),
    _arg_data_callback(arg_data_callback),
    _headers_callback(headers_callback),
    _arg_headers_callback(arg_headers_callback),


    _headers(headers),
    _params(params),
    _post_fileds(post_fileds),
    _post_fileds_size(post_fileds_size),

    _curl_verbose(curl_verbose),
    _trust_self_signed_server_ssl_tls(trust_self_signed_server_ssl_tls),

    _ssl_cert_type(ssl_cert_type),
    _ssl_cert(ssl_cert),
    _ssl_key(ssl_key),
    _ssl_key_password(ssl_key_password),

    _headers_list(NULL),
    _request_mutex(PTHREAD_MUTEX_INITIALIZER){}
  virtual ~REQUEST_CONFIG(){}
public:
  void configure_curl(CURL *curl_handle, unsigned int option){
    /* temporal variables */
    std::string url;
    std::string encoded_url;
    /* case options */
    switch (option){
    /*  
     * Set the http method:
     *                              https://curl.se/libcurl/c/CURLOPT_CUSTOMREQUEST.html
     */
    case CURLOPT_CUSTOMREQUEST:
      curl_easy_setopt(curl_handle, CURLOPT_CUSTOMREQUEST, this->_method);
    /*  
     * Enable the Verbose for CURL
     *                              https://curl.se/libcurl/c/CURLOPT_VERBOSE.html
     */
    case CURLOPT_VERBOSE:
      curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, (int)this->_curl_verbose);
      break;
    /*  
     * Encode GET parameters 
     *                              https://curl.se/libcurl/c/CURLOPT_URL.html
     */
    case CURLOPT_URL:
      url = std::string(this->_url);
      encoded_url = std::string(REQUESTS::encode_get_params(curl_handle,*this->_params).c_str());
      if(encoded_url.length()>0){url.append("?");url.append(encoded_url);}
      curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
      break;
    /*  
     * Callback for the response data 
     *                              https://curl.se/libcurl/c/CURLOPT_WRITEFUNCTION.html
     */
    case CURLOPT_WRITEFUNCTION:
      if(this->_data_callback!=NULL)
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, this->_data_callback);
      else
        log_warn("CURLOPT_WRITEFUNCTION required with NULL pointer.\n");
      break;
    /*  
     * Argument for the response data callback 
     *                              https://curl.se/libcurl/c/CURLOPT_WRITEDATA.html
     */
    case CURLOPT_WRITEDATA:
      if(this->_arg_data_callback!=NULL)
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void*)this->_arg_data_callback);
      else
        log_warn("CURLOPT_WRITEDATA required with NULL pointer.\n");
      break;
    /*  
     * Callback for the response headers 
     *                              https://curl.se/libcurl/c/CURLOPT_HEADERFUNCTION.html
     */
    case CURLOPT_HEADERFUNCTION:
      if(this->_headers_callback!=NULL)
        curl_easy_setopt(curl_handle, CURLOPT_HEADERFUNCTION, this->_headers_callback);
      else
        log_warn("CURLOPT_HEADERFUNCTION required with NULL pointer.\n");
      break;
    /*  
     * Argument for the response headers callback 
     *                              https://curl.se/libcurl/c/CURLOPT_HEADERDATA.html
     */
    case CURLOPT_HEADERDATA:
      if(this->_arg_headers_callback!=NULL)
        curl_easy_setopt(curl_handle, CURLOPT_HEADERDATA, (void*)this->_arg_headers_callback);
      else
        log_warn("CURLOPT_HEADERDATA required with NULL pointer.\n");
      break;
    /*  
     * Set up the request headers 
     *                              https://curl.se/libcurl/c/CURLOPT_HTTPHEADER.html
     */
    case CURLOPT_HTTPHEADER:
      if(this->_headers!=NULL){
        for(std::string ch: *this->_headers)
          this->_headers_list = curl_slist_append(this->_headers_list, ch.c_str());
        curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, this->_headers_list);
      } else {
        log_warn("CURLOPT_HTTPHEADER required with NULL pointer.\n");
      }
      break;
    /*  
     * Set up for a POST request the data
     *                              https://curl.se/libcurl/c/CURLOPT_POSTFIELDS.html
     */
    case CURLOPT_POSTFIELDS:
      if(this->_post_fileds!=NULL)
        curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDSIZE, this->_post_fileds);
      else
        log_warn("CURLOPT_POSTFIELDS required with NULL pointer.\n");
      break;
    /*  
     * Set up for a POST request the size of the data 
     *                              https://curl.se/libcurl/c/CURLOPT_POSTFIELDSIZE.html
     */
    case CURLOPT_POSTFIELDSIZE:
      curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, this->_post_fileds_size);
      break;
    /*  
     * Trust servers with self signed ssl certificates
     *                              https://curl.se/libcurl/c/CURLOPT_SSL_VERIFYPEER.html
     */
    case CURLOPT_SSL_VERIFYPEER:
      if(this->_trust_self_signed_server_ssl_tls){
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
      if(this->_ssl_cert_type!=NULL)
        curl_easy_setopt(curl_handle, CURLOPT_SSLCERTTYPE, this->_ssl_cert_type);
      else
        log_warn("CURLOPT_SSLCERTTYPE required with NULL pointer\n");
      break;
    /*  
     *  Parse the SSL/TLS client certificate File Path
     *                              https://curl.se/libcurl/c/CURLOPT_SSLCERT.html
     */
    case CURLOPT_SSLCERT:
      if(this->_ssl_cert!=NULL)
        curl_easy_setopt(curl_handle, CURLOPT_SSLCERT, this->_ssl_cert);
      else
        log_warn("CURLOPT_SSLCERT required with NULL pointer\n");
      break;
    /*  
     *  Specify the SSL/TLS client certificate key file 
     *                              https://curl.se/libcurl/c/CURLOPT_SSLKEY.html
     */
    case CURLOPT_SSLKEY:
      if(this->_ssl_key!=NULL)
        curl_easy_setopt(curl_handle, CURLOPT_SSLKEY, this->_ssl_key);
      else
        log_warn("CURLOPT_SSLKEY required with NULL pointer\n");
      break;
    /*  
     *  Specify the SSL/TLS client certficiate file password
     *                              https://curl.se/libcurl/c/CURLOPT_KEYPASSWD.html
     */
    case CURLOPT_KEYPASSWD:
      if(this->_ssl_key_password!=NULL)
        curl_easy_setopt(curl_handle, CURLOPT_KEYPASSWD, this->_ssl_key_password);
      else
        log_warn("CURLOPT_KEYPASSWD required with NULL pointer\n");
      break;
    /*  */
    default:
      break;
    }
  }
  void finalize_configure_curl(){
    if(this->_headers_list!=NULL)
      curl_slist_free_all(this->_headers_list);
  }
};

private:
  static void finit(){
    curl_global_cleanup();
  }
  static void init(){
    curl_global_init(CURL_GLOBAL_DEFAULT);
    std::atexit(REQUESTS::finit);
  }
public:
  static class _init {public:_init(){REQUESTS::init();}}_initializer;
private:
  static std::string url_encode(CURL *curl_handle, std::string value){
    /* fabricate encoded char* */
    char* encoded_url=curl_easy_escape(curl_handle, value.c_str(), 0);
    if(encoded_url==NULL){
      log_err("Unable to url encode get request parameters\n");
      return std::string("");
    }
    /* fabricate encoded string */
    std::string encoded_str(encoded_url);
    /* free the encoded char* */
    curl_free(encoded_url);
    /* return encoded string */
    return encoded_str;
  }
  static std::string encode_get_params(CURL *curl_handle, std::map<std::string, std::string> &params){
    if(params.empty())
      return "";
    std::map<std::string, std::string>::const_iterator pb= params.cbegin(), pe= params.cend();
    std::string data=url_encode(curl_handle, pb->first)+"="+ url_encode(curl_handle, pb->second);
    ++ pb;if(pb==pe)return data;
    for(; pb!= pe; ++ pb)
      data+= "&"+ url_encode(curl_handle, pb->first)+ "="+ url_encode(curl_handle, pb->second);
    return data;
  }
private:
  static void basic_post_query(CURL *curl_handle, REQUESTS::REQUEST_CONFIG config){
    /* configure the POST request */
    config.configure_curl(curl_handle, CURLOPT_VERBOSE);
    config.configure_curl(curl_handle, CURLOPT_CUSTOMREQUEST);
    config.configure_curl(curl_handle, CURLOPT_URL);
    config.configure_curl(curl_handle, CURLOPT_WRITEFUNCTION);
    config.configure_curl(curl_handle, CURLOPT_WRITEDATA);
    config.configure_curl(curl_handle, CURLOPT_HEADERFUNCTION);
    config.configure_curl(curl_handle, CURLOPT_HEADERDATA);
    config.configure_curl(curl_handle, CURLOPT_HTTPHEADER);
    config.configure_curl(curl_handle, CURLOPT_POSTFIELDS);
    config.configure_curl(curl_handle, CURLOPT_POSTFIELDSIZE);
    config.configure_curl(curl_handle, CURLOPT_SSL_VERIFYPEER);
    config.configure_curl(curl_handle, CURLOPT_SSLCERT);
    config.configure_curl(curl_handle, CURLOPT_SSLCERTTYPE);
    config.configure_curl(curl_handle, CURLOPT_SSLKEY);
    config.configure_curl(curl_handle, CURLOPT_KEYPASSWD);
  }
  static void basic_get_query(CURL *curl_handle, REQUESTS::REQUEST_CONFIG config){
    /* configure the GET request */
    config.configure_curl(curl_handle, CURLOPT_VERBOSE);
    config.configure_curl(curl_handle, CURLOPT_CUSTOMREQUEST);
    config.configure_curl(curl_handle, CURLOPT_URL);
    config.configure_curl(curl_handle, CURLOPT_WRITEFUNCTION);
    config.configure_curl(curl_handle, CURLOPT_WRITEDATA);
    config.configure_curl(curl_handle, CURLOPT_HEADERFUNCTION);
    config.configure_curl(curl_handle, CURLOPT_HEADERDATA);
    config.configure_curl(curl_handle, CURLOPT_HTTPHEADER);
    config.configure_curl(curl_handle, CURLOPT_SSL_VERIFYPEER);
    config.configure_curl(curl_handle, CURLOPT_SSLCERTTYPE);
    config.configure_curl(curl_handle, CURLOPT_SSLCERT);
    config.configure_curl(curl_handle, CURLOPT_SSLKEY);
    config.configure_curl(curl_handle, CURLOPT_KEYPASSWD);
    
  }
public:
  static void request(REQUESTS::REQUEST_CONFIG config){
    /* lock the mutex */
    pthread_mutex_lock(&config._request_mutex);
    /* create a handle */
    CURL *curl_handle=curl_easy_init();
    /* check the handle */
    if(!curl_handle){
      log_err("unable to initialize curl");
      return;
    }
    /* configure thre request */
    if(strcmp(config._method,"GET")==0)
      basic_get_query(curl_handle, config);
    else
      basic_post_query(curl_handle, config);
    /* lock thread */
    pthread_mutex_lock(&curl_lock);
    /* perform the operation */
    CURLcode curl_code = curl_easy_perform(curl_handle);
    /* unlock thread */
		pthread_mutex_unlock(&curl_lock);
    /* finalize the request list */
    config.finalize_configure_curl();
    /* check for errors */
    if(curl_code!=CURLE_OK)
      log_err("curl failed to perform [basic_get_query]: %s\n",curl_easy_strerror(curl_code));
    /* clean the handle */
    curl_easy_cleanup(curl_handle);
    /* free the mutex lock */
    pthread_mutex_unlock(&config._request_mutex);
  }
};
REQUESTS::_init REQUESTS::_initializer;

/* this function helps to replace a string, cleaning a string */
static void clean_memory(REQUESTS::memory *mem, std::vector<const char*> marks){
  /* temporal transform to string */
  std::string temp(mem->response);
  /* clean the response object */
  for(const char* ch: marks){
    while(temp.find(ch)!=std::string::npos)
      temp.replace(temp.find(ch), std::strlen(ch), "");
  }
  /* reallocate the mem */
  mem->response = (char*)realloc(mem->response, sizeof(char)*(temp.size()+1));
  /* initialize */
  memset(mem->response, '\0', sizeof(char)*(temp.size()+1));
  /* transfer the data */
  memcpy(mem->response, temp.c_str(), temp.size());
  /* update the size */
  mem->size=temp.size();
}
size_t data_callback_text(void *data, size_t size, size_t nmemb, void *userp){
  /* calculate incoming memory size */
  size_t realsize = size * nmemb;
  /* cast struct */
  struct REQUESTS::memory *mem = (struct REQUESTS::memory *)userp;
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
size_t header_callback(char *buffer, size_t size, size_t nitems, void *userdata){
  if(strcmp(buffer,"")!=0 && strcmp(buffer,"\n")!=0 && strcmp(buffer,"\0")!=0 && strlen(buffer)>2)
    log("HEADER: \t%s",buffer);
  return nitems * size;
}
#endif