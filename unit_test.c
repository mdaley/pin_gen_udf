#include <stdio.h>
#include <string.h>
#include "minunit.h"
#include "pin_gen_udf.h"

static void free_udf_args(UDF_ARGS* args) {
  int i;
  for (i = 0; i < args->arg_count; i++) {
    if (args->args[i] != NULL) {
      free(args->args[i]);
    }
  }
  free(args->args);
  free(args->arg_type);
  free(args);
}

static UDF_ARGS *alloc_udf_args(int count) {
  UDF_ARGS *args = malloc(sizeof(UDF_ARGS));

  args->arg_count = count;
  args->arg_type = malloc(count * sizeof(int));
  args->args = malloc(count * sizeof(char*));
  args->lengths = malloc(count * sizeof(unsigned long));

  return args;
}

static void set_arg_as_int(UDF_ARGS* args, int index, long value) {
  args->arg_type[index] = INT_RESULT;
  args->args[index] = malloc(sizeof(long));
  *((long*)args->args[index]) = value; // mysql naughtiness, storing a long in the space for a string!
}

static void set_arg_as_string(UDF_ARGS* args, int index, char* value) {
  args->arg_type[index]= STRING_RESULT;
  args->args[index] = malloc(256);
  args->lengths[index] = strlen(value);
  snprintf(args->args[index], 255, "%s", value);
}

static int is_valid_pin(char* value, int length) {
  int i;
  for (i = 0; i < length; i++) {
    char v = value[i];
    if (!((v >= 'A' && v <= 'Z') ||
          (v >= '0' && v <= '9') ||
          (v >= 'a' && v <= 'z')))
      return -1;
  }

  return 0;
}

int tests_run = 0;

static char* init_returns_error_when_no_parameters_supplied() {
  UDF_ARGS udf_args;
  udf_args.arg_count = 0;

  char message[256];

  my_bool result = pin_gen_udf_init(NULL, &udf_args, (char*)&message);

  if (result == 1 && strstr(message, "expected two arguments"))
    return NULL;
  else
    return "expected error code and message not returned.";
}

static char* init_returns_error_when_more_than_two_parameters_supplied() {
  UDF_ARGS* udf_args = alloc_udf_args(3);

  char message[256];

  my_bool result = pin_gen_udf_init(NULL, udf_args, (char*)&message);

  free(udf_args);

  if (result == 1)
    return NULL;
  else
    return "expected error code and message not returned.";
}

static char* init_returns_error_when_two_parameters_supplied_but_first_is_not_of_integer_type() {
  UDF_ARGS* udf_args = alloc_udf_args(2);
  set_arg_as_string(udf_args, 0, "a string value");

  char message[256];

  my_bool result = pin_gen_udf_init(NULL, udf_args, (char*)&message);

  free(udf_args);

  if (result == 1)
    return NULL;
  else
    return "expected error code and message not returned.";
}

static char* init_returns_error_when_two_parameters_supplied_but_second_is_not_of_string_type() {
  UDF_ARGS* udf_args = alloc_udf_args(2);
  set_arg_as_int(udf_args, 0, 8);
  set_arg_as_int(udf_args, 1, 0);

  char message[256];

  my_bool result = pin_gen_udf_init(NULL, udf_args, (char*)&message);

  free(udf_args);

  if (result == 1)
    return NULL;
  else
    return "expected error code and message not returned.";
}

static char* init_returns_error_when_size_parameter_supplied_but_it_is_too_small() {
  UDF_ARGS* udf_args = alloc_udf_args(2);
  set_arg_as_int(udf_args, 0, 3);

  char message[256];

  my_bool result = pin_gen_udf_init(NULL, udf_args, (char*)&message);

  free(udf_args);

  if (result == 1)
    return NULL;
  else
    return "expected error code and message not returned.";
}

static char* init_returns_error_when_size_parameter_supplied_but_it_is_too_large() {
  UDF_ARGS* udf_args = alloc_udf_args(2);
  set_arg_as_int(udf_args, 0, 25);

  char message[256];

  my_bool result = pin_gen_udf_init(NULL, udf_args, (char*)&message);

  free(udf_args);

  if (result == 1)
    return NULL;
  else
    return "expected error code and message not returned";
}

static char* init_returns_error_when_allowedchars_parameter_supplied_but_it_is_too_short() {
  UDF_ARGS* udf_args = alloc_udf_args(2);
  set_arg_as_int(udf_args, 0, 8);
  set_arg_as_string(udf_args, 1, "012345678"); // 9 chars

  char message[256];

  my_bool result = pin_gen_udf_init(NULL, udf_args, (char*)&message);

  free(udf_args);

  if (result == 1)
    return NULL;
  else
    return "expected error code and message not returned";
}

static char* init_returns_error_when_allowedchars_parameter_supplied_but_it_is_too_long() {
  UDF_ARGS* udf_args = alloc_udf_args(2);
  set_arg_as_int(udf_args, 0, 8);
  set_arg_as_string(udf_args, 1, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789A"); // 63 chars

  char message[256];

  my_bool result = pin_gen_udf_init(NULL, udf_args, (char*)&message);

  free(udf_args);

  if (result == 1)
    return NULL;
  else
    return "expected error code and message not returned";
}

static char* init_returns_error_when_allowedchars_parameter_contains_an_invalid_value() {
  UDF_ARGS* udf_args = alloc_udf_args(2);
  set_arg_as_int(udf_args, 0, 8);
  set_arg_as_string(udf_args, 1, "ABCDEFGHIJKLMNOPQRSTUVWXYZab+defghijklmnopqrstuvwxyz0123456789");

  char message[256];

  my_bool result = pin_gen_udf_init(NULL, udf_args, (char*)&message);

  free(udf_args);

  if (result == 1)
    return NULL;
  else
    return "expected error code and message not returned";
}

static char* init_returns_success_when_valid_parameter_is_supplied() {
  UDF_ARGS* udf_args = alloc_udf_args(2);
  set_arg_as_int(udf_args, 0, 16);
  set_arg_as_string(udf_args, 1, "ABCDEFGHIJKLMNOPQ");

  char message[256];

  my_bool result = pin_gen_udf_init(NULL, udf_args, (char*)&message);

  free(udf_args);

  if (result == 0)
    return NULL;
  else
    return "incorrect result code received.";
}

static char* deinit_succeeds() {
  pin_gen_udf_deinit(NULL);
  return NULL;
}

static char* pin_generation_succeeds() {
  UDF_ARGS* udf_args = alloc_udf_args(1);
  set_arg_as_int(udf_args, 0, 16);
  set_arg_as_string(udf_args, 1, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890");

  char result[256];
  unsigned long result_length;
  char is_null;
  char error;

  char* pin = pin_gen_udf(NULL, udf_args, result, &result_length, &is_null, &error);

  free(udf_args);

  if (result_length == 16 && is_null == 0 && error == 0 && is_valid_pin(result, result_length) == 0)
    return NULL;
  else
    return "Incorrect response from pin generation";
}

static char* generated_pins_are_different() {
  UDF_ARGS* udf_args = alloc_udf_args(1);
  set_arg_as_int(udf_args, 0, 16);
  set_arg_as_string(udf_args, 1, "ABCDEFGHIJKLMNOPQRSTUVWXYZ123456");

  int iterations = 1000;
  char result[iterations][256];
  unsigned long result_length;
  char is_null;
  char error;

  int i, j;
  for (i = 0; i < iterations; i++) {
    char* pin = pin_gen_udf(NULL, udf_args, result[i], &result_length, &is_null, &error);
  }

  free(udf_args);

  // OK, using some sort of hashmap might be better for these comparisons but this will do!
  for (i = 0; i < iterations; i++) {
    for (j = 0; j < iterations; j++) {
      if (i != j) {
        if (strncmp(result[i], result[j], 16) == 0)
          return "Two pins were identical, which is not good at all!";
      }
    }
  }

  return NULL;
}

static char* generated_pins_contain_full_range_of_characters_and_nothing_else() {

  char* allowed_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
  UDF_ARGS* udf_args = alloc_udf_args(1);
  set_arg_as_int(udf_args, 0, 16);
  set_arg_as_string(udf_args, 1, allowed_chars);

  int iterations = 1000;
  char result[iterations][256];
  unsigned long result_length;
  char is_null;
  char error;

  int i, j;
  for (i = 0; i < iterations; i++) {
    char* pin = pin_gen_udf(NULL, udf_args, result[i], &result_length, &is_null, &error);
  }

  free(udf_args);

  int counts[62] = { 0 };

  for (i = 0; i < iterations; i++) {
    for (j = 0; j < 16; j++) {
      char v = result[i][j];
      char* vlocation = strchr(allowed_chars, v);
      if (vlocation == NULL) {
        return "Character in PIN is not in allowed chars";
      }
      int index = (int) (vlocation - allowed_chars);
      counts[index]++;
    }
  }

  for (i = 0; i < 62; i++) {
    if (counts[i] == 0) {
      return "An expected character hasn't been used at all.";
    }
  }

  return NULL;
}

static char* all_tests() {
  mu_run_test(init_returns_error_when_no_parameters_supplied);
  mu_run_test(init_returns_error_when_more_than_two_parameters_supplied);
  mu_run_test(init_returns_error_when_two_parameters_supplied_but_first_is_not_of_integer_type);
  mu_run_test(init_returns_error_when_two_parameters_supplied_but_second_is_not_of_string_type);
  mu_run_test(init_returns_error_when_size_parameter_supplied_but_it_is_too_small);
  mu_run_test(init_returns_error_when_size_parameter_supplied_but_it_is_too_large);
  mu_run_test(init_returns_error_when_allowedchars_parameter_supplied_but_it_is_too_short);
  mu_run_test(init_returns_error_when_allowedchars_parameter_supplied_but_it_is_too_long);
  mu_run_test(init_returns_error_when_allowedchars_parameter_contains_an_invalid_value);
  mu_run_test(init_returns_success_when_valid_parameter_is_supplied);

  mu_run_test(deinit_succeeds);

  mu_run_test(pin_generation_succeeds);
  mu_run_test(generated_pins_are_different);
  mu_run_test(generated_pins_contain_full_range_of_characters_and_nothing_else);
  return 0;
}

int main(int argc, char **argv) {
  char *result = all_tests();
  if (result != 0) {
    printf("%s\n", result);
  }
  else {
    printf("ALL TESTS PASSED\n");
  }
  printf("Tests run: %d\n", tests_run);

  return result != 0;
 }
