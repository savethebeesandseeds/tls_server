#include <vector>
#include <map>
#include "requests_piaabo.hpp"
#include "util_piaabo.h"
#include "file_piaabo.h"
/* main for clients tls auth */
#define METHOD "GET"
#define URL "https://localhost:8888"
#define SSL_TYPE "PEM"
#define SSL_CLIENT_CERT "access/client.pem"
#define SSL_CLIENT_KEY "access/client.key.pem"

int main(int argc, char *argv[]){
  log("Client tls auth\n");
  
  log("file_exist(SSL_CLIENT_CERT): %s\n",print_bool(file_exist(SSL_CLIENT_CERT)));
  log("file_exist(SSL_CLIENT_KEY): %s\n",print_bool(file_exist(SSL_CLIENT_KEY)));

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
    SSL_TYPE, /* ssl_cert_type */
    SSL_CLIENT_CERT, /* ssl_cert */
    SSL_CLIENT_KEY, /* ssl_key */
    NULL  /* ssl_key_password */
  );   

  REQUESTS::request(get_request_config);
  log("Response: \t%s\n",request_text_response.response);
  free(request_text_response.response);
  
  return 0;
}