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

my_bool pin_gen_udf_init(UDF_INIT *initid, UDF_ARGS *args, char *message);

void pin_gen_udf_deinit(UDF_INIT *initid __attribute__((unused)));

char* pin_gen_udf(UDF_INIT* initid, UDF_ARGS* args __attribute__((unused)),
                  char* result, unsigned long* length,
                  char* is_null __attribute__((unused)), char* error __attribute__((unused)));


int random_bytes(char* bytes, int length);

void to_pincode_bytes(char* bytes, int length);

my_bool pin_gen_udf_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{
  int ok = TRUE;

  if(!(args->arg_count == 1)) {
    ok = FALSE;
  } else if (args->arg_type[0] != INT_RESULT || *args->args[0] < 4 || *args->args[0] > 24) {
    ok = FALSE;
  }

  if (ok == FALSE) {
    strcpy(message, "expected one argument 'length', integer, from 4 to 24 inclusive.");
    return 1;
  }

  return 0;
}

void pin_gen_udf_deinit(UDF_INIT *initid __attribute__((unused)))
{
}

char* pin_gen_udf(UDF_INIT* initid, UDF_ARGS* args __attribute__((unused)),
                  char* result, unsigned long* result_length,
                  char* is_null __attribute__((unused)),
                  char* error __attribute__((unused)))
{
  int length = *args->args[0];
  char bytes[length];

  if (random_bytes(bytes, length) == -1) {
    result = "wrong bytes read";
    *result_length = 16;
    return;
  }

  to_pincode_bytes(bytes, length);
  strncpy(result, bytes, length);

  *result_length = length;
  return result;
}

int random_bytes(char* bytes, int length) {
  int f = open("/dev/urandom", O_RDONLY);

  if (f < 0)
    return -1;

  int bytes_read = read(f, bytes, length);

  close(f);

  return (bytes_read == length) ? 0 : -1;
}

char* pincode_points = "01234567890ACDEFGHJKLMNPQRSTUVWXY";

void to_pincode_bytes(char* bytes, int length) {
  int i = 0;
  for (i = 0; i < length; i++) {
    *(bytes + i) = *(pincode_points + (*(bytes + i) & 31));
  }
}

#endif /* HAVE_DLOPEN */
