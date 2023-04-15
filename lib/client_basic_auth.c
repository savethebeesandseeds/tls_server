#include "util_piaabo.h"
#include "requests_piaabo.h"

/* main for basic auth */
int main(int argc, char *argv[]){
  log("Client basic auth\n");
  /* Request parameters */
  const char *METHOD = "GET";
  const char *URL = "https://www.oauth.com/playground/auth-dialog.html";
  /* Request memory object */
  request_memory_t *request_text_response = build_request_memory();
  /* Request configuration */
  request_config_t *request_config = initialize_request_config(METHOD, URL);
  request_config->_data_callback = data_callback_text;
  request_config->_arg_data_callback = request_text_response;
  request_config->_headers_callback = header_callback;
  request_config->_curl_verbose = true;
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
}