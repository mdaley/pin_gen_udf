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

/* The init function; returns error if param (length of PIN) is invalid.
 *
 * @initid - unused
 * @args - arguments passed in (only one, the length of the PIN requested)
 * @message - an already allocated buffer that can be filling with an error
 *            message if necessary.
 *
 * returns - 0 for success and 1 for failure.
 */
my_bool pin_gen_udf_init(UDF_INIT *initid __attribute__((unused)),
                         UDF_ARGS *args, char *message)
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

/* The deinit function. nothing needs to happen here, e.g. no
 * memory to release.
 *
 * @initid - unused.
 */
void pin_gen_udf_deinit(UDF_INIT *initid __attribute__((unused)))
{
}

/* Generate and return a PIN.
 *
 * @initid - unused.
 * @args - the arguments passed in (only one, the length of PIN requested).
 * @result - pointer to the buffer (already allocated by mysql, size 256)
 *           where the resulting PIN needs to be stored.
 * @result_length - the size of of the generated PIN (and the size of the
 *                  result buffer actually used.
 * @is_null - pointer to a value that should be set to 1 if the result is NULL.
 * @error - pointer to a value that should be set to 1 if an error has occurred.
 *
 * returns - pointer to the buffer containing the generated PIN (same as @result).
 */
char* pin_gen_udf(UDF_INIT* initid __attribute__((unused)), UDF_ARGS* args,
                  char* result, unsigned long* result_length,
                  char* is_null, char* error)
{
  int length = *args->args[0];
  char bytes[length];

  if (random_bytes(bytes, length) == -1) {
    *is_null = 1;
    *error = 1;
    return NULL;
  }

  to_pincode_bytes(bytes, length);
  strncpy(result, bytes, length);

  *result_length = length;
  return result;
}

/* Fill a buffer with values from /dev/urandom. Of course, this only
 * works on linux but this is OK for us.
 *
 * @bytes - pointer to buffer of bytes to be filled.
 * @length - the size of the buffer.
 *
 * returns: 0 for success and -1 for failure.
 */
int random_bytes(char* bytes, int length) {
  int f = open("/dev/urandom", O_RDONLY);

  if (f < 0)
    return -1;

  int bytes_read = read(f, bytes, length);

  close(f);

  return (bytes_read == length) ? 0 : -1;
}

/* The allowed values of digits in a PIN */
char* pincode_points = "0123456789BCDEFGHJKLMNPQRSTVWXYZ";

/* Change the byte values in the supplied buffer from 0x00 to 0xFF
 * to the ASCII values 0-9 A-Z excluding A, I, O and U which can
 * be easily confused with numbers or can help to avoid the problem
 * of rude words in PINs, e.g. F?CK, SH?T, W?NK; you get the idea.
 *
 * @bytes - pointer to the buffer of bytes to be modified.
 * @length - length of the buffer.
 */
void to_pincode_bytes(char* bytes, int length) {
  int i = 0;
  for (i = 0; i < length; i++) {
    *(bytes + i) = *(pincode_points + (*(bytes + i) & 31));
  }
}

#endif /* HAVE_DLOPEN */
