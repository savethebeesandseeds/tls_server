#ifndef __SERVER_PIAABO
#define __SERVER_PIAABO
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <microhttpd.h>
#include <string.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>

#include "util_piaabo.h"

#define FAVICON "resources/waacamaya_q1.png"

#define MHD_RequestTerminationCode_to_string (const char[4][40]) {\
  "TERMINATED_COMPLETED_OK","TERMINATED_WITH_ERROR","TERMINATED_TIMEOUT_REACHED","TERMINATED_DAEMON_SHUTDOWN"}

typedef const char* output_char_function_pointer();
typedef enum MHD_Result route_pointer(
  void *dh_cls, struct MHD_Connection *connection);

/* access function to process Incomming request */
static enum MHD_Result on_client_connect(
  void *cls, const struct sockaddr *addr, socklen_t addrlen){
  /* incomming request, new request detected */
	log("--- --- --- %sRequest started%s.\n",
    ANSI_COLOR_Green,ANSI_COLOR_RESET);
  /* allocate space for the ip string */
  char ip_str[addrlen]; /* allocate space for address string */
  /* catch and standarize ip address */
  inet_ntop(
    addr->sa_family, 
    addr->sa_family == AF_INET6 
    ? (void*)(&((struct sockaddr_in6 *)addr)->sin6_addr) 
    : (void*)(&((struct sockaddr_in *)addr)->sin_addr), 
    ip_str, addrlen);
  /* log the adrees */
  log("---%s Incomming from ip(%s)%s: %s\n",
    ANSI_COLOR_Bright_Yellow,
    addr->sa_family == AF_INET6 ? "v6" : "v4", 
    ANSI_COLOR_RESET, ip_str);
  /* proceed to the server */
	return MHD_YES;
}
/* finalization function to process request */
void on_request_completed(
  void *dh_cls, 
  struct MHD_Connection *connection,
  void **con_cls,
  enum MHD_RequestTerminationCode toe){
  /* finalizing request */
  log("--- --- --- %sRequest completed%s: %s.\n",
    ANSI_COLOR_Green,ANSI_COLOR_RESET,
    MHD_RequestTerminationCode_to_string[toe]);
}

/* log request headers */
static enum MHD_Result log_conection_value(
  void *cls, enum MHD_ValueKind kind, 
  const char *key, const char *value){
  char str_kind[40];
  switch (kind){
  case MHD_HEADER_KIND:       strcpy(str_kind,"HEADER");   break;
  case MHD_COOKIE_KIND:       strcpy(str_kind,"COOKIE");   break;
  case MHD_POSTDATA_KIND:     strcpy(str_kind,"POSTDATA"); break;
  case MHD_GET_ARGUMENT_KIND: strcpy(str_kind,"GET_ARG");  break;
  case MHD_FOOTER_KIND:       strcpy(str_kind,"FOOTER");   break;
  default:                    strcpy(str_kind,"UNKNOWN");  break;
  }
  /* log key, value pair */
  log("%s:\t%s%s%s: %s\n", str_kind,(char*)cls, key, ANSI_COLOR_RESET, value);
  /* return success */
  return MHD_YES;
}
/* log request information */
static void log_request(
  void *cls, 
  struct MHD_Connection *connection,
  const char *url,
  const char *method, 
  const char *version,
  const char *upload_data,
  size_t *upload_data_size, void **con_cls){
  /* log path and version */
  log("--- %s[%s] request%s, for path: %s, using version %s\n", 
    ANSI_COLOR_Bright_Yellow, method, ANSI_COLOR_RESET, url, version);
  /* log request values */
  MHD_get_connection_values(connection, MHD_HEADER_KIND, &log_conection_value, (void *)&ANSI_COLOR_Red);
  MHD_get_connection_values(connection, MHD_COOKIE_KIND, &log_conection_value, (void *)&ANSI_COLOR_Magenta);
  MHD_get_connection_values(connection, MHD_POSTDATA_KIND, &log_conection_value, (void *)&ANSI_COLOR_Blue);
  MHD_get_connection_values(connection, MHD_GET_ARGUMENT_KIND, &log_conection_value, (void *)&ANSI_COLOR_Green);
  MHD_get_connection_values(connection, MHD_FOOTER_KIND, &log_conection_value, (void *)&ANSI_COLOR_Bright_Yellow);
}
/* session manager with cookies */
static enum MHD_Result session_cookie(
  struct MHD_Connection *connection,
  struct MHD_Response *response){
  /* reading cookies */
  const char *read_value = 
    MHD_lookup_connection_value(
      connection, MHD_COOKIE_KIND, "SESSION");
  if(read_value!=NULL){
    log("Session cookie found\n");
    return MHD_YES;
  }
  /* setting cookies */
  char set_value[128];
  char raw_value[65];
  for (unsigned int i=0;i<sizeof(raw_value);i++)
    raw_value[i]='A' + (rand () % 26); /* bad PRNG! */
  raw_value[64] = '\0';
  snprintf(set_value, sizeof(set_value), "%s=%s","SESSION",raw_value);
  /* setting cookie */
  return MHD_add_response_header(
    response, MHD_HTTP_HEADER_SET_COOKIE, set_value);
}
/* respond with HTML */
static enum MHD_Result respond_with_html(
  const char* page_contents, 
  struct MHD_Connection *connection, 
  struct MHD_Response *response,
  unsigned int status_code){
  /* temporal variables */
  enum MHD_Result ret;
  const char *content_type="text/html";
  /* fill response from buffer */
  response = MHD_create_response_from_buffer(
    strlen(page_contents), (void*)page_contents, MHD_RESPMEM_PERSISTENT);
  /* verify the response has been set */
  if(response==NULL)
    return MHD_NO;
  /* add response headers */
  ret = MHD_queue_response(connection, status_code, response);
  /* add response headers */
  MHD_add_response_header(
    response, "Content-Type", content_type);
  /* log response */
  log("--- --- %sResponding with%s: statusCode:[%d], Content-Type:[%s], body:%s\n",
    ANSI_COLOR_Bright_Yellow, ANSI_COLOR_RESET, status_code, content_type, page_contents);
  /* set session cookie */
  session_cookie(connection, response);
  /* free presponse buffer */
  MHD_destroy_response(response);
  /* return success */
  return ret;
}
/* respond with FILE */
static enum MHD_Result respond_with_file(
  int fd,
  const char *content_type,
  struct MHD_Connection *connection, 
  struct MHD_Response *response,
  unsigned int status_code){
  /* temporary variables */
  enum MHD_Result ret;
  struct stat sbuf;
  /* verify the file contetns */
  if((-1 == fd) || (0 != fstat(fd, &sbuf)) ){
    if (fd != -1) (void) close (fd);
    return respond_with_html(
      "<html><body>An internal server error has occurred!</body></html>", 
      connection, response, MHD_HTTP_INTERNAL_SERVER_ERROR);
  }
  /* fabric MHD_Response with binary buffer data */
  response = MHD_create_response_from_fd_at_offset64(
    sbuf.st_size, fd, 0);
  /* verify the response has been set */
  if(response==NULL)
    return MHD_NO;
  /* add response status code */
  ret = MHD_queue_response(connection, status_code, response);
  /* add content type header */
  MHD_add_response_header(response, "Content-Type", content_type);
  /* set session cookie */
  session_cookie(connection, response);
  /* log response */
  log("--- --- %sResponding with%s: statusCode:[%d], Content-Type:%s\n",
    ANSI_COLOR_Bright_Yellow, ANSI_COLOR_RESET, status_code, content_type);
  /* free presponse buffer */
  MHD_destroy_response(response);
  /* return sucess */
  return ret; 
}
/* favicon.ico */
static enum MHD_Result favicon_route(
  void *dh_cls,
  struct MHD_Connection *connection){
  struct MHD_Response *response=NULL;
  return respond_with_file(
    open(FAVICON, O_RDONLY), "image/png", 
    connection, response, MHD_HTTP_OK);
}
/* default route */
static enum MHD_Result default_route(
  void *dh_cls,
  struct MHD_Connection *connection){
  struct MHD_Response *response=NULL;
  /* render HTML response (default) */
  return respond_with_html(
    "<html><body>Hello, browser!</body></html>", 
    connection, response, MHD_HTTP_OK);
}
/* selects the appropriate route */
static route_pointer* route_coordinator(
  const char *url,
  const char *method, 
  const char *version){
  if(!strcmp(method,"GET") && !strcmp(url,"/favicon.ico")){
    return favicon_route;
  } else {
    return default_route;
  }
}
#endif