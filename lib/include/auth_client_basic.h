#ifndef __AUTH_CLIENT_BASIC
#define __AUTH_CLIENT_BASIC
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <microhttpd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "util_piaabo.h"

static enum MHD_Result ask_for_basic_authentication(
  struct MHD_Connection *connection, 
  const char *realm){
  /* temporal variables */
  enum MHD_Result ret;
  struct MHD_Response *response;
  char *auth_header;
  const char *strbase = "Basic realm=";
  /* create NULL response */
  response = MHD_create_response_from_buffer(
    0, NULL, MHD_RESPMEM_PERSISTENT);
  /* validate remponse */
  if(!response)
    return MHD_NO;
  /* allocate header space */
  auth_header =(char*)malloc(strlen(strbase) + strlen(realm) + 1);
  if(!auth_header)
    return MHD_NO;
  /* fill auth header */
  strcpy(auth_header, strbase);
  strcat(auth_header, realm);
  /* set response header */
  ret = MHD_add_response_header(response, "WWW-Authenticate", auth_header);
  /* free header */
  free(auth_header);
  /* validate ret */
  if(!ret){
    MHD_destroy_response(response);
    return MHD_NO;
  }
  /* queue response */
  ret = MHD_queue_response(connection, MHD_HTTP_UNAUTHORIZED, response);
  /* free response */
  MHD_destroy_response(response);
  /* return */
  return ret;
}

bool is_basic_authenticated(
  struct MHD_Connection *connection,
  const char *username, 
  const char *password){
  /* temporary variables */
  const char *auth_header;
  char *expected_b64, *expected;
  const char *strbase = "Basic ";
  bool authenticated;
  /* check headers */
  auth_header =
    MHD_lookup_connection_value(
      connection, MHD_HEADER_KIND,
      "Authorization");
  /* validate authentication */
  if(NULL == auth_header)
    return 0;
  /* validate authentication is of type Basic */
  if(0 != strncmp(auth_header, strbase, strlen(strbase)))
    return 0;
  /* allocate space for password */
  expected =(char*)malloc(strlen(username) + 1 + strlen(password) + 1);
  if(NULL == expected)
    return 0;
  /* fill up the expected username and password */
  strcpy(expected, username);
  strcat(expected, ":");
  strcat(expected, password);
  /* encode base64 */
  expected_b64 = encode_base64(expected);
  /* free expected */
  free(expected);
  /* validate encoding is ok */
  if(NULL == expected_b64)
    return 0;
  /* authentication comparison */
  authenticated =
   (strcmp(auth_header + strlen(strbase), expected_b64) == 0);
  /* free base64 */
  free(expected_b64);
  /* return boolean */
  return authenticated;
}
#endif