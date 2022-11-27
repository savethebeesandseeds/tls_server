#include "stdio.h"
#include "server_piaabo.h"
#include "util_piaabo.h"
#include "file_piaabo.h"
#include "auth_client_basic.h"

#define PORT 8888
#define SERVERCERTKEYFILE "access/server.key.pem"
#define SERVERCERTPEMFILE "access/server.pem"

#define REALM     "\"Maintenance\""
#define USER      "user"
#define PASSWORD  "pass"

/* method to process request with server having tls auth and client having basic auth */
static enum MHD_Result answer_to_connection_basic_auth(
  void *dh_cls, 
  struct MHD_Connection *connection,
  const char *url,
  const char *method, 
  const char *version,
  const char *upload_data,
  size_t *upload_data_size, 
  void **con_cls){
  /* initial only method GET */
  if(0 != strcmp(method, "GET"))
    return MHD_NO;
  /* initial communication */
  if(NULL == *con_cls){
    *con_cls = connection;
    return MHD_YES;// session_cookie(dh_cls, connection);
  }
  /* log requests */
  log_request(dh_cls, connection, url, method, version, upload_data, upload_data_size, con_cls);
  /* check if is authenticated */
  if(!is_basic_authenticated(connection, USER, PASSWORD))
    return ask_for_basic_authentication(connection, REALM);
  /* respond to request */
  return route_coordinator(url, method, version)(dh_cls, connection);;
}
/* main */
int main(int argc, char *argv[]){
  /* pointers for keys */
  char *server_cert_key=NULL;
  char *server_cert_pem=NULL;
  /* read key files */
  read_text_file(SERVERCERTKEYFILE,&server_cert_key);
  read_text_file(SERVERCERTPEMFILE,&server_cert_pem);
  /* validate keys */
  if((server_cert_key == NULL) || (server_cert_pem == NULL)){
    log("The key/certificate files could not be read.\n");
    return 1;
  }
  /* create server daemon */
  struct MHD_Daemon *daemon;
  daemon = MHD_start_daemon(
    MHD_USE_THREAD_PER_CONNECTION | MHD_USE_SSL, PORT, 
    on_client_connect, NULL,
    &answer_to_connection_basic_auth, NULL, 
    MHD_OPTION_HTTPS_MEM_KEY, server_cert_key,
    MHD_OPTION_HTTPS_MEM_CERT, server_cert_pem,
    MHD_OPTION_CONNECTION_TIMEOUT,(unsigned int) 15,
    MHD_OPTION_NOTIFY_COMPLETED, &on_request_completed, NULL,
    MHD_OPTION_END);
  /* validate server daemon */
  if(NULL == daemon){
    /* log error */
    log_err("Unable to start MHD.\n");
    /* free keys chars */
    free(server_cert_key);
    free(server_cert_pem);
    /* return error */
    return 1;
  }
  /* loop */
  log("[Running server...]\n");
  getchar();
  log("[Closing server...]\n");
  /* stop server daemon */
  MHD_stop_daemon(daemon);
  /* free keys chars */
  free(server_cert_key);
  free(server_cert_pem);
  /* return success */
  return 0;
}