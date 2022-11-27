#ifndef __FILES_PIAABO
#define __FILES_PIAABO
#include <string.h>
#include <memory.h>
#include <stdio.h>
#include <stdbool.h>

#include "util_piaabo.h"

static inline size_t read_file(FILE *d_file, char** buff, bool as_string){
  /* validate input */
  if(*buff!=NULL)
    log_err("Please pass a null pointer, as reference to FILES buffer");
  /* seek file end */
  fseek(d_file, 0, SEEK_END);
  /* calculate contents size */
  size_t buff_size = ftell(d_file);
  /* allocate buffer */
  *buff =((char*) realloc(*buff, sizeof(char)*(buff_size+as_string)));
  /* validate pointer */
  if(*buff==NULL){
    log_err("Unable to allocate memory to read file");
    exit(1);
  }
  /* initialize buffer */
  memset(*buff, '\0', sizeof(char)*(buff_size+as_string));
  /* rewind binary file */
  rewind(d_file);
  /* read file */
  fread(*buff, sizeof(char), buff_size, d_file);
  /* return buffer size */
  return buff_size;
}
static inline size_t read_text_file(const char* input_file, char** buff){
  /* open file in read-binary mode */
  FILE *text_file = fopen(input_file,"r");
  /* evaluate file */
  if(!text_file){
    log_err("Unable to open text file: %s",input_file);
    return 0;
  }
  /* read file contents */
  size_t buff_size = read_file(text_file, buff, true);
  /* close file */
  fclose(text_file);
  /* return success */
  return buff_size;
}
static inline size_t read_binary_file(const char* input_file, char**buff){
  /* open file in read-binary mode */
  FILE *binary_file = fopen(input_file,"rb");
  /* evaluate file */
  if(!binary_file){
    log_err("Unable to open binary file: %s",input_file);
    return 0;
  }
  /* read file contents */
  size_t buff_size = read_file(binary_file, buff, false);
  /* close file */
  fclose(binary_file);
  /* return success */
  return buff_size;
}
static inline bool file_exist(const char* input_file){
  /* open the file */
  FILE *dfile = fopen(input_file,"r");
  if(dfile){
    fclose(dfile);
    return true;
  }
  return false;
}
// static inline long get_file_size(const char *input_file){
//   /* temporal variables */
//   FILE *fp;
//   long size;
//   /* open file */
//   fp = fopen(input_file, "rb");
//   /* open file */
//   if(fp){
//     if((0 != fseek(fp, 0, SEEK_END)) ||(-1 ==(size = ftell(fp))))
//       size = 0;
//     fclose(fp);
//     /* return success */
//     return size;
//   }
//   /* return error */
//   return 0;
// }
// static char *load_file(const char *filename){
//   /* get file fize */
//   size_t size = get_file_size(filename);
//   /* validate file */
//   if(size == 0)
//     return NULL;
//   /* open file */
//   FILE *fp = fopen(filename, "rb");
//   if(!fp)
//     return NULL;
//   /* allocate buffer */
//   char *buffer =(char*)malloc(size);
//   if(!buffer){
//     fclose(fp);
//     return NULL;
//   }
//   /* load buffer */
//   if(size != fread(buffer, 1, size, fp)){
//     free(buffer);
//     buffer = NULL;
//   }
//   /* close file */
//   fclose(fp);
//   /* return buffer */
//   return buffer;
// }
#endif