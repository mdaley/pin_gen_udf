#include "pin_gen_udf.h"

/* The init function; returns non-zero if parameters are invalid.
 *
 * @initid - unused.
 * @args - arguments passed in (two: the length of the PIN requested and the chanracters allowed).
 * @message - an already allocated buffer that can be filling with an error
 *            message if necessary.
 *
 * returns - 0 for success and 1 for failure.
 */
my_bool pin_gen_udf_init(UDF_INIT *initid __attribute__((unused)),
                         UDF_ARGS *args, char *message)
{
  int ok = TRUE;

  if (!(args->arg_count == 2)) {
    ok = FALSE;
  } else if (args->arg_type[0] != INT_RESULT || *args->args[0] < 4 || *args->args[0] > 24
             || args->arg_type[1] != STRING_RESULT || args->lengths[1] < 10 || args->lengths[1] > 62) {
    ok = FALSE;
  } else if (validate_allowed_chars(args->args[1], args->lengths[1]) != 0) {
    ok = FALSE;
  }

  if (ok == FALSE) {
    strcpy(message, "args: length (4 to 24) followed by string of allowed chars (A-Za-z0-9).");
    return 1;
  }

  return 0;
}

int validate_allowed_chars(char* chars, unsigned long long length) {

  unsigned long long i;
  for (i = 0; i < length; i++) {
    char v = chars[i];
    if (!((v >= 'A' && v <= 'Z') ||
          (v >= '0' && v <= '9') ||
          (v >= 'a' && v <= 'z')))
      return -1;
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
 * @args - the arguments passed in (the length of PIN requested and the chars allowed).
 * @result - pointer to the buffer (already allocated by mysql, size 256)
 *           where the resulting PIN needs to be stored.
 * @result_length - the size of of the generated PIN (and the size of the
 *                  result buffer actually used).
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
  unsigned int uints[length];
  char* allowed = args->args[1];
  unsigned long long allowed_size = args->lengths[1];

  if (random_uints(uints, length) == -1) {
    *is_null = 1;
    *error = 1;
    return NULL;
  }

  int i;
  for (i = 0; i < length; i++) {
    uints[i] = uints[i] % allowed_size;
  }

  for (i = 0; i < length; i++) {
    result[i] = allowed[uints[i]];
  }

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

/* Fill a buffer with random unsigned ints
 *
 * @ints - pointer to buffer of unsigned ints to be filled.
 * @length - the size of the buffer.
 *
 * return: 0 for success and -1 for failure.
 */
int random_uints(unsigned int* ints, int length) {
  return random_bytes((char*) ints, length * (sizeof(unsigned int)));
}
