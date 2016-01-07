pin_gen_udf
===========

A user defined function for generating PINs of between 4 and 24 ASCII characters. Each character can have
the values 0-9, A-Z and a-z with the values allowed being specified.

Compile for OSX or linux using the appropriate script. May need tweaking depending on the location of mysql
executable and include directories. `mysql_config --cflags` should give you good info about the flags
required for compilation.

Anyway, after compilation you should have a `pin_gen_udf.so` shared library. Copy this to the plugin directory
of mysql, e.g. on OSX `/usr/local/mysql/lib/plugin` (I installed using the DMG from Oracle, instead of using brew)
or, on ubuntu `/usr/lib/mysql/plugin`.

(Note there is primitive versioning: store a release by copying to the `library` directories and giving the released
library a version number. Look at the files already stored to understand the approach.)

Then, run:

```sql
CREATE FUNCTION pin_gen_udf RETURNS STRING SONAME "pin_gen_udf.so";
```

to install the UDF and:

```sql
SELECT pin_gen_udf(16, 'ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789');
```

to generate a PIN.

Note that for live use, I've given the shared library file to the DBAs who've put it into their puppet scripts for managing database servers. There's no versioning or packaging as I haven't worked out how to do it, but this approach is adequate for the moment.
