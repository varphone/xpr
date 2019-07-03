/* src/config.h.  Generated from config.h.in by configure.  */
/* src/config.h.in.  Generated from configure.ac by autoheader.  */

#define CONFIG_XML_CONTENT 1
#define CONFIG_XML_NAV 1
#define CONFIG_XML_BUFF 1

/* Can the target library commit a tree. When set to 0, this result in smaller
   binary (this saves about 5kb). [default=yes] */
#define CONFIG_XML_COMMIT 1

/* Can the target library modify XML tree. When set to 0, this result in
   smaller binary (this saves about 3kb). [default=yes] */
#define CONFIG_XML_EDIT 1

/* Can the target library parse file documents. When set to 0, this result in
   smaller binary. [default=yes] */
#define CONFIG_XML_FILE 1

/* Does the target library support float XML values. [default=yes] */
#define CONFIG_XML_FLOAT 1

/* Is the target library uses stack for internal buffers or heap. This is not
   compatible with thread safety. [default=no] */
/* #undef CONFIG_XML_HEAP_BUFFERS */

/* Define the size of internal buffer. For very small systems, large internal
   buffers can cause the systeml the behave strangely. [default=no] */
/* #undef CONFIG_XML_SMALL_BUFFER */

/* Limit the size of input XML libroxml can handle to 64kb instead of 4Gb.
   Setting this option will reduce the size of RAM loaded tree. [default=no]
   */
/* #undef CONFIG_XML_SMALL_INPUT_FILE */

/* Is the target library thread safe or not. [default=yes] */
#define CONFIG_XML_THREAD_SAFE 1

/* Can the target library perform xpath queries. When set to 0, this result in
   smaller binary (this saves about 20kb). [default=yes] */
#define CONFIG_XML_XPATH 1

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* Define to 1 if you have the <fuse.h> header file. */
/* #undef HAVE_FUSE_H */

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define to the sub-directory where libtool stores uninstalled libraries. */
#define LT_OBJDIR ".libs/"

/* Name of package */
#define PACKAGE "libroxml"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "tristan.lelong@libroxml.net"

/* Define to the full name of this package. */
#define PACKAGE_NAME "libroxml"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "libroxml 3.0.2"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "libroxml"

/* Define to the home page for this package. */
#define PACKAGE_URL ""

/* Define to the version of this package. */
#define PACKAGE_VERSION "3.0.2"

/* The size of `int', as computed by sizeof. */
#define SIZEOF_INT 4

/* The size of `long', as computed by sizeof. */
#define SIZEOF_LONG 4

/* The size of `size_t', as computed by sizeof. */
#define SIZEOF_SIZE_T 4

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Version number of package */
#define VERSION "3.0.2"
