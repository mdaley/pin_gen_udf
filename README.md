pin_gen_udf
===========

A user defined function for generating PINs of between 4 and 24 ASCII characters. Each character can have
the values 0-9, A-Z excluding A, I, O and U (32 possible values). The exclusions help to avoid confusion
with numbers and reduce the possibility of certain rude words cropping up.

Compile for OSX or linux using the appropriate script. May need tweaking depending on the location of mysql
executable and include directories. `mysql_config --cflags` should give you good info about the flags
required for compilation.

Anyway, after compilation you should have a `pin_gen_udf.so` shared library. Copy this to the plugin directory
of mysql, e.g. on OSX `/usr/local/mysql/lib/plugin` (I installed using the DMG from Oracle, instead of using brew)
or, on ubuntu `/usr/lib/mysql/plugin`.

Then, run:

```
CREATE FUNCTION pin_gen_udf RETURNS STRING SONAME "pin_gen_udf.so"
```

to install the UDF and:

```sql
SELECT pi_gen_udf(16)
```

to generate a PIN.
