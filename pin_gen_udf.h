#ifndef PIN_GEN_UDF_H
#define PIN_GEN_UDF_H

#ifdef STANDARD
  /* STANDARD is defined, don't use any mysql functions */
  #include <stdlib.h>
  #include <stdio.h>
  #include <string.h>

  #ifdef __WIN__
    typedef unsigned __int64 ulonglong;/* Microsofts 64 bit types */
    typedef __int64 longlong;
  #else
    typedef unsigned long long ulonglong;
    typedef long long longlong;
  #endif /*__WIN__*/
#else
  #include <my_global.h>
  #include <my_sys.h>

  #if defined(MYSQL_SERVER)
    #include <m_string.h>/* To get strmov() */
  #else
    /* when compiled as standalone */
    #include <string.h>
    #define strmov(a,b) stpcpy(a,b)
    #define bzero(a,b) memset(a,0,b)
    #define memcpy_fixed(a,b,c) memcpy(a,b,c)
  #endif
#endif

#include <mysql.h>
#include <ctype.h>

#ifdef HAVE_DLOPEN
  #if !defined(HAVE_GETHOSTBYADDR_R) || !defined(HAVE_SOLARIS_STYLE_GETHOST)
  static pthread_mutex_t LOCK_hostname;
  #endif

my_bool pin_gen_udf_init(UDF_INIT *initid __attribute__((unused)),
                         UDF_ARGS *args, char *message);

void pin_gen_udf_deinit(UDF_INIT *initid __attribute__((unused)));

char* pin_gen_udf(UDF_INIT* initid __attribute__((unused)), UDF_ARGS* args,
                  char* result, unsigned long* length,
                  char* is_null, char* error);


int random_bytes(char* bytes, int length);

void to_pincode_bytes(char* bytes, int length);

#endif /* HAVE_DLOPEN */

#endif /* PIN_GEN_UDF_ */
