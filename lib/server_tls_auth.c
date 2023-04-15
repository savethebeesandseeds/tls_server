#include "stdio.h"
#include "auth_client_tls.h"
#include "server_piaabo.h"
#include "util_piaabo.h"
#include "file_piaabo.h"

#define SERVERPORT 8888
#define SERVERCERTKEYFILE "access/server.key.pem"
#define SERVERCERTPEMFILE "access/server.pem"
#define CLIENTCERTPEMFILE "access/client.pem"

/* method to process request with server having tls auth and client having tls auth */
static enum MHD_Result answer_to_connection_tls_auth(
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
  /* check if the client send a ssl/tls cert */
  gnutls_session_t tls_session = get_tls_session(connection);
  if(tls_session==NULL){
    log_warn("No tls session found for client\n");
    return ask_for_tls_authentication(connection);
  }
  /* retrive the ssl/tls cert */
  gnutls_x509_crt_t client_cert = get_client_certificate(tls_session);
  if(client_cert==NULL){
    log_warn("No certficate session found for client\n");
    return ask_for_tls_authentication(connection);
  }
  /* retrive the client distinguished name */
  char *distinguished_name = cert_auth_get_dn(client_cert);
  /* retrive the client alternative name */
  char *alt_name = MHD_cert_auth_get_alt_name(client_cert,0,0);
  /* log requests */
  log("TLS CERT:\tdistinguished_name:%s\n",distinguished_name);
  log("TLS CERT:\talt_name:%s\n",alt_name);
  log_request(dh_cls, connection, url, method, version, upload_data, upload_data_size, con_cls);
  /* respond to request */
  return route_coordinator(url, method, version)(dh_cls, connection);;
}
/* main */
int main(int argc, char *argv[]){
  /* pointers for keys */
  char *server_cert_key=NULL;
  char *server_cert_pem=NULL;
  char *client_cert_pem=NULL;
  /* read key files */
  read_text_file(SERVERCERTKEYFILE,&server_cert_key);
  read_text_file(SERVERCERTPEMFILE,&server_cert_pem);
  read_text_file(CLIENTCERTPEMFILE,&client_cert_pem);
  /* validate keys */
  if((server_cert_key == NULL) || (server_cert_pem == NULL)){
    log("The key/certificate files could not be read.\n");
    return 1;
  }
  /* create server daemon */
  struct MHD_Daemon *daemon;
  daemon = MHD_start_daemon(
    MHD_USE_DEBUG | MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_THREAD_PER_CONNECTION | MHD_USE_SSL, SERVERPORT, 
    on_client_connect, NULL,
    &answer_to_connection_tls_auth, NULL, 
    MHD_OPTION_HTTPS_MEM_KEY, server_cert_key,
    MHD_OPTION_HTTPS_MEM_CERT, server_cert_pem,
    MHD_OPTION_HTTPS_MEM_TRUST, client_cert_pem,
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
    free(client_cert_pem);
    /* return error */
    return 1;
  }
  /* loop */
  log("[Running TLS server on port %s%d%s]...\n",ANSI_COLOR_Red,SERVERPORT,ANSI_COLOR_RESET);
  getchar();
  log("[Closing TLS server...]\n");
  /* free keys chars */
  free(server_cert_key);
  free(server_cert_pem);
  free(client_cert_pem);
  /* stop server daemon */
  MHD_stop_daemon(daemon);
  /* return success */
  return 0;
}