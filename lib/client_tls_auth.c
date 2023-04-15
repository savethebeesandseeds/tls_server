#include "util_piaabo.h"
#include "file_piaabo.h"
#include "requests_piaabo.h"

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

  /* Request memory object */
  request_memory_t *request_text_response = build_request_memory();
  /* Request configuration */
  request_config_t *request_config = initialize_request_config(METHOD, URL);
  request_config->_data_callback = data_callback_text;
  request_config->_arg_data_callback = request_text_response;
  request_config->_headers_callback = header_callback;
  request_config->_curl_verbose = true;
  request_config->_trust_self_signed_server_ssl_tls = true;
  request_config->_ssl_cert_type = SSL_TYPE;
  request_config->_ssl_cert = SSL_CLIENT_CERT;
  request_config->_ssl_key = SSL_CLIENT_KEY;
  /* set the params */
  add_param(request_config, "data", "contents");
  /* set the headers */
  add_header(request_config, "Content-Type application/json");
  /* Make the request */
  http_request(request_config);
  fprintf(stdout,"Response: \n%s\n",request_text_response->response);
  /* clear the memory */
  free_request_memory(request_text_response);
  free_request_config(request_config);

  return 0;
}