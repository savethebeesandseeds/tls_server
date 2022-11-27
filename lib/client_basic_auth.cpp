#include "requests_piaabo.hpp"
#include <vector>
#include <map>
#include "util_piaabo.h"

#define METHOD "GET"
#define URL "https://localhost:8888"

/* main for basic auth */
int main(int argc, char *argv[]){
  log("Client basic auth\n");

  std::vector<std::string> headers={};
  std::map<std::string, std::string> get_params={{"waka","dwak el chuki"}};

  REQUESTS::memory request_text_response = {
    .response=NULL,
    .size=0
  };
  REQUESTS::REQUEST_CONFIG get_request_config = REQUESTS::REQUEST_CONFIG(
    METHOD, /* method */
    URL,  /* url */
    data_callback_text, /* data_callback */
    (void*)&request_text_response, /* arg_data_callback */
    header_callback, /* headers_callback */
    NULL, /* arg_headers_callback */
    &headers, /* headers */
    &get_params, /* get params */
    NULL, /* post_fileds */
    (size_t)0, /* post_fileds_size */
    true, /* curl_verbose */
    true, /* trust_self_signed_server_ssl_tls */
    NULL, /* ssl_cert_type */
    NULL, /* ssl_cert */
    NULL, /* ssl_key */
    NULL  /* ssl_key_password */
  );   

  REQUESTS::request(get_request_config);
  log("Response: \t%s\n",request_text_response.response);
  free(request_text_response.response);
  
  
}