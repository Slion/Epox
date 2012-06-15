/************************************************************************
 *
 * driver.cpp - definitions of the test driver
 *
 * $Id: driver.cpp 290009 2005-09-18 23:28:26Z sebor $
 *
 ************************************************************************
 *
 * Copyright (c) 1994-2005 Quovadx,  Inc., acting through its  Rogue Wave
 * Software division. Licensed under the Apache License, Version 2.0 (the
 * "License");  you may  not use this file except  in compliance with the
 * License.    You    may   obtain   a   copy   of    the   License    at
 * http://www.apache.org/licenses/LICENSE-2.0.    Unless   required    by
 * applicable law  or agreed to  in writing,  software  distributed under
 * the License is distributed on an "AS IS" BASIS,  WITHOUT WARRANTIES OR
 * CONDITIONS OF  ANY KIND, either  express or implied.  See  the License
 * for the specific language governing permissions  and limitations under
 * the License.
 *
 **************************************************************************/

// expand _TEST_EXPORT macros
#define _RWSTD_TEST_SRC

#include "opt_diags.h"
#include "opt_lines.h"
#include "opt_trace.h"
#include "opt_types.h"

#include <cmdopt.h>    // for rw_setopts()
#include <printf.h>    // for rw_snprintfa()

#include <assert.h>    // for assert
#include <setjmp.h>    // for longjmp, setjmp, ...
#include <stdarg.h>    // for va_list
#include <stdio.h>     // for fileno, fprintf
#include <stdlib.h>    // for free
#include <string.h>    // for strchr, strcpy
#include"std_log_result.h"
#define LOG_FILENAME_LINE __FILE__, __LINE__
#if !defined (_WIN32) && !defined (_WIN64)
#  include <unistd.h>    // for isatty

// declare fileno in case it's not declared (for strict ANSI conformance)
extern "C" {

IMPORT_C int (fileno)(FILE*) _LIBC_THROWS ();

}   // extern "C"

#else
   // no isatty on Windoze
#  define _RWSTD_NO_ISATTY
#endif   // _WIN{32,64}

// expand _TEST_EXPORT macros
#define _RWSTD_TEST_SRC
#include <driver.h>

/************************************************************************/

#define RW_TEST_STRSTR(x)   #x
#define RW_TEST_STR(x)      RW_TEST_STRSTR(x)

#ifndef RW_TEST_COMPILER
#  if defined (__DECCXX__)
#    define RW_TEST_COMPILER "Compaq C++, __DECCXX__ = " \
            RW_TEST_STR (__DECCXX__)
#  elif defined (__INTEL_COMPILER)
#    if defined (__EDG_VERSION__)
#      define RW_TEST_ICC_EDG_VER \
              ", __EDG_VERSION__ = "  RW_TEST_STR (__EDG_VERSION__)
#    else
#      define RW_TEST_ICC_EDG_VER ""
#    endif
#    if defined (_MSC_VER)
#      define RW_TEST_COMPILER "Intel C++, __INTEL_COMPILER = " \
              RW_TEST_STR (__INTEL_COMPILER) ", _MSC_VER = " \
              RW_TEST_STR (_MSC_VER) \
              RW_TEST_ICC_EDG_VER
#    elif defined (__INTEL_COMPILER_BUILD_DATE)
#      define RW_TEST_COMPILER "Intel C++, __INTEL_COMPILER = " \
              RW_TEST_STR (__INTEL_COMPILER) \
              ", __INTEL_COMPILER_BUILD_DATE = " \
              RW_TEST_STR (__INTEL_COMPILER_BUILD_DATE) \
              RW_TEST_ICC_EDG_VER
#    else
#      define RW_TEST_COMPILER "Intel C++, __INTEL_COMPILER = " \
              RW_TEST_STR (__INTEL_COMPILER) \
              RW_TEST_ICC_EDG_VER
#    endif
#  elif defined (__GNUC__)
#    if defined (__VERSION__)
#      define RW_TEST_GCC_VER ", __VERSION__ = \"" __VERSION__ "\""
#    else
#      define RW_TEST_GCC_VER ""
#    endif
#    if defined (__GNUC_PATCHLEVEL__)
#      define RW_TEST_COMPILER "gcc "            \
              RW_TEST_STR (__GNUC__) "."         \
              RW_TEST_STR (__GNUC_MINOR__) "."   \
              RW_TEST_STR (__GNUC_PATCHLEVEL__)  \
              RW_TEST_GCC_VER
#    else
#      define RW_TEST_COMPILER "gcc " \
              RW_TEST_STR (__GNUC__) "." RW_TEST_STR (__GNUC_MINOR__) 
              RW_TEST_GCC_VER
#    endif
#  elif defined (_COMPILER_VERSION) && defined (__sgi)
#    define RW_TEST_COMPILER "SGI MIPSpro, _COMPILER_VERSION = " \
            RW_TEST_STR (_COMPILER_VERSION)
#  elif defined (__INTEL_COMPILER)
#    if defined (_MSC_VER)
#      define RW_TEST_COMPILER "Intel C++, __INTEL_COMPILER = " \
              RW_TEST_STR (__INTEL_COMPILER) ", _MSC_VER = " \
              RW_TEST_STR (_MSC_VER)
#    else
#      define RW_TEST_COMPILER "Intel C++, __INTEL_COMPILER = " \
              RW_TEST_STR (__INTEL_COMPILER)
#    endif
#  elif defined (__EDG__)
#    define RW_TEST_COMPILER "EDG eccp, __EDG_VERSION__ = " \
            RW_TEST_STR (__EDG_VERSION__)
#  elif defined (__HP_aCC)
#    define RW_TEST_COMPILER "HP aCC, __HP_aCC = " \
            RW_TEST_STR (__HP_aCC)
#  elif defined (__IBMCPP__)
#    define RW_TEST_COMPILER "IBM VisualAge C++, __IBMCPP__ = " \
            RW_TEST_STR (__IBMCPP__)
#  elif defined (_MSC_VER)
#    define RW_TEST_COMPILER "MSVC, _MSC_VER = " \
            RW_TEST_STR (_MSC_VER)
#  elif defined (__SUNPRO_CC)
#    define RW_TEST_COMPILER "SunPro, __SUNPRO_CC = " \
            RW_TEST_STR (__SUNPRO_CC)
#  else
#    define RW_TEST_COMPILER "unknown"
#  endif
#endif

#ifndef RW_TEST_LIBSTD
#  ifdef _RWSTD_VER
#    define RW_TEST_LIBSTD "Rogue Wave C++ Standard Library, " \
            "_RWSTD_VER = " RW_TEST_STR (_RWSTD_VER)
#  elif defined (__GLIBCXX__)
#    define RW_TEST_LIBSTD "GNU C++ Standard Library, " \
            "__GLIBCXX__ = " \
            RW_TEST_STR (__GLIBCXX__)
#  elif defined (_STLPORT_VERSION)
     // check for STLport before SGI STL since STLport,
     // being derived from SGI STL, #defines both macros
#    define RW_TEST_LIBSTD "STLport, " \
            "_STLPORT_VERSION = " \
            RW_TEST_STR (_STLPORT_VERSION)
#  elif defined (__SGI_STL)
#    define RW_TEST_LIBSTD "SGI STL, " \
            "__SGI_STL = " \
            RW_TEST_STR (__SGI_STL)
#  elif defined (_YVALS)
     // is there a better way to identify the Dinkumware
     // implementation? does it have a version macro?
#    define RW_TEST_LIBSTD "Dinkum C++ Standard Library"
#  endif
#endif   // RW_TEST_LIBSTD

#ifndef RW_TEST_HARDWARE
#  if defined (__alpha__) || defined (__alpha)
#    define RW_TEST_ARCH "alpha"
#  elif defined (__amd64__) || defined (__amd64)
#    if defined (__LP64__) || defined (_LP64)
#      define RW_TEST_ARCH "amd64/LP64"
#    else
#      define RW_TEST_ARCH "amd64/ILP32"
#    endif
#  elif defined (_PA_RISC2_0)
#    define RW_TEST_ARCH "pa-risc 2.0"
#  elif defined (_PA_RISC1_0)
#    define RW_TEST_ARCH "pa-risc 1.0"
#  elif defined (__hppa)
#    define RW_TEST_ARCH "pa-risc"
#  elif defined (__pentiumpro__) || defined (__pentiumpro)
#    define RW_TEST_ARCH "pentiumpro"
#  elif defined (__pentium__) || defined (__pentium)
#    define RW_TEST_ARCH "pentium"
#  elif defined (__i486__) || defined (__i486)
#    define RW_TEST_ARCH "i486"
#  elif defined (__i386__) || defined (__i386)
#    define RW_TEST_ARCH "i386"
#  elif defined (__i586__) || defined (__i586)
#    define RW_TEST_ARCH "i586"
#  elif defined (__ia64)
#    define RW_TEST_ARCH "ia64"
#  elif defined (__mips)
#    define RW_TEST_ARCH "mips"
#  elif defined (__sparcv9)
#    define RW_TEST_ARCH "sparc-v9"
#  elif defined (__sparcv8)
#    define RW_TEST_ARCH "sparc-v8"
#  elif defined (__sparc)
#    define RW_TEST_ARCH "sparc"
#  elif defined (_POWER)
#    if defined (_ARCH_PWR5)
#      define RW_TEST_ARCH "power-5"
#    elif defined (_ARCH_PWR4)
#      define RW_TEST_ARCH "power-4"
#    elif defined (_ARCH_PWR3)
#      define RW_TEST_ARCH "power-3"
#    elif defined (_ARCH_604)
#      define RW_TEST_ARCH "powerpc-604"
#    elif defined (_ARCH_603)
#      define RW_TEST_ARCH "powerpc-603"
#    elif defined (_ARCH_602)
#      define RW_TEST_ARCH "powerpc-602"
#    elif defined (_ARCH_601)
#      define RW_TEST_ARCH "powerpc-601"
#    elif defined (_ARCH_403)
#      define RW_TEST_ARCH "powerpc-403"
#    elif defined (_ARCH_PPC64)
#      define RW_TEST_ARCH "powerpc/LP64"
#    else
#      define RW_TEST_ARCH "powerpc"
#    endif
#  elif defined (_WIN64)
#    define RW_TEST_ARCH "ia64"
#  elif defined (_WIN32)
#    define RW_TEST_ARCH "i86"
#  elif defined (__x86_64__) || defined (__x86_64)
#    if defined (__LP64__) || defined (_LP64)
#      define RW_TEST_ARCH "x86_64/LP64"
#    else
#      define RW_TEST_ARCH "x86_64/ILP32"
#    endif
#  else
#    define RW_TEST_ARCH "unknown"
#  endif


#  if defined (_AIX54)
#    define RW_TEST_OS "aix-5.4 (or better)"
#  elif defined (_AIX53)
#    define RW_TEST_OS "aix-5.3"
#  elif defined (_AIX52)
#    define RW_TEST_OS "aix-5.2"
#  elif defined (_AIX51)
#    define RW_TEST_OS "aix-5.1"
#  elif defined (_AIX50)
#    define RW_TEST_OS "aix-5.0"
#  elif defined (_AIX43)
#    define RW_TEST_OS "aix-4.3"
#  elif defined (_AIX41)
#    define RW_TEST_OS "aix-4.1"
#  elif defined (_AIX32)
#    define RW_TEST_OS "aix-3.2"
#  elif defined (_AIX)
#    define RW_TEST_OS "aix"
#  elif defined (__hpux)
#    define RW_TEST_OS "hp-ux"
#  elif defined (__osf__)
#    define RW_TEST_OS "tru64-unix"
#  elif defined (__sgi) && defined (__mips)
#    define RW_TEST_OS "irix"
#  elif defined (__linux__) || defined (__linux)

     // get Linux release string (UTS_RELEASE)
#    include <linux/version.h>

#    ifndef UTS_RELEASE
#      define UTS_RELEASE "(unknown release)"
#    endif   // UTS_RELEASE

#    if defined (__ELF__)
#      define LINUX_TYPE "linux-elf"
#    else
#      define LINUX_TYPE "linux"
#    endif

#    define RW_TEST_OS LINUX_TYPE " "     \
            UTS_RELEASE " with glibc "    \
            RW_TEST_STR (__GLIBC__) "."   \
            RW_TEST_STR (__GLIBC_MINOR__)

#  elif defined (__SunOS_5_10)
#    define RW_TEST_OS "sunos-5.10"
#  elif defined (__SunOS_5_9)
#    define RW_TEST_OS "sunos-5.9"
#  elif defined (__SunOS_5_8)
#    define RW_TEST_OS "sunos-5.8"
#  elif defined (__SunOS_5_7)
#    define RW_TEST_OS "sunos-5.7"
#  elif defined (__SunOS_5_6)
#    define RW_TEST_OS "sunos-5.6"
#  elif defined (__sun__)
#    define RW_TEST_OS "sunos"
#  elif defined (_WIN64)
#    define RW_TEST_OS "win64"
#  elif defined (_WIN32)
#    define RW_TEST_OS "win32"
#  else
#    define RW_TEST_OS "unknown"
#  endif

#  define RW_TEST_HARDWARE RW_TEST_ARCH " running " RW_TEST_OS
#else
#  define RW_TEST_HARDWARE "unknown"
#endif

/************************************************************************/

// defined in printf.cpp but not declared in printf.h
_TEST_EXPORT int
rw_vasnprintf (char**, size_t*, const char*, va_list);

/************************************************************************/

// array to store the number of each type of diagnostic
static int
ndiags [N_DIAG_TYPES][2] /* = { { total, active }, ... }*/;

static FILE *ftestout;

static jmp_buf test_env;

// set to 1 after the driver has been initialized
static int driver_initialized = 0;

// set to 1 after the driver has finished running
static int driver_finished = 0;

#if 0   // disabled
// %S: severity
// %M: diagnostic
// %m: diagnostic if not empty
// %F: file name
// %f: file name if not empty
// %C: clause
// %c: clause if not empty
// %L: line number
// %l: line number if valid
// %T: text
// %t: text if not empty
static char diag_pattern [80];
#endif

// option: use CSV format (comma separated values)
static int _rw_opt_csv = 0;

static char clause_id [80];

/************************************************************************/

#define CHECK_INIT(init, func)   _rw_check_init (init, __LINE__, func)

static inline void
_rw_check_init (bool init, int line, const char *func)
{
    if (init && !driver_initialized) {
        fprintf (stderr, "%s:%d: %s: test driver already initialized\n",
                 __FILE__, line, func);
     std_log(LOG_FILENAME_LINE,"%s:%d: %s: test driver already initialized\n",
                 __FILE__, line, func);                 
        abort ();
    }

    if (!init && driver_initialized) {
        fprintf (stderr, "%s:%d: %s: test driver not initialized yet\n",
                 __FILE__, line, func);
      std_log(LOG_FILENAME_LINE,"%s:%d: %s: test driver not initialized yet\n",
                 __FILE__, line, func);                  
        abort ();
    }

    if (driver_finished) {
        fprintf (stderr, "%s:%d: %s: test finished, cannot call\n",
                 __FILE__, line, func);
     std_log(LOG_FILENAME_LINE,"%s:%d: %s: test finished, cannot call\n",
                 __FILE__, line, func);                 
    }
}

/************************************************************************/

static int
_rw_opt_brief (int argc, char *argv[])
{
    static int opt_brief;

    if (0 == argc) {
        // query mode: return the value of the option
        return opt_brief;
    }

    if (1 == argc && argv && 0 == argv [0]) {
        // help mode: set argv[0] to the text of the help message

        static const char helpstr[] = {
            "Enables brief mode.\n"
        };

        argv [0] = _RWSTD_CONST_CAST (char*, helpstr);

        return 0;
    }

    // set mode: enable the option
    opt_brief = 1;

    return 0;
}

/************************************************************************/

static int
_rw_opt_quiet (int argc, char *argv[])
{
    static int opt_quiet;

    if (0 == argc) {
        // query mode: return the value of the option
        return opt_quiet;
    }

    if (1 == argc && argv && 0 == argv [0]) {
        // help mode: set argv[0] to the text of the help message

        static const char helpstr[] = {
            "Enables quiet mode.\n"
            "In quiet mode only diagnostics with severity 7 and above are "
            "issued."
        };

        argv [0] = _RWSTD_CONST_CAST (char*, helpstr);

        return 0;
    }

    // set mode: enable the option
    _rw_diag_mask = ~((1 << 7) | (1 << 8) | (1 << 9));
    opt_quiet     = 1;

    return 0;
}

/************************************************************************/

static int
_rw_opt_verbose (int argc, char *argv[])
{
    static int opt_verbose;

    if (0 == argc) {
        // query mode: return the value of the option
        return opt_verbose;
    }

    if (1 == argc && argv && 0 == argv [0]) {
        // help mode: set argv[0] to the text of the help message

        static const char helpstr[] = {
            "Enables verbose mode.\n"
        };

        argv [0] = _RWSTD_CONST_CAST (char*, helpstr);

        return 0;
    }

    // set mode: enable the option
    opt_verbose = 1;

    return 0;
}

/************************************************************************/

static int
_rw_setopt_csv (int argc, char *argv[])
{
    if (1 == argc && argv && 0 == argv [0]) {
        static const char helpstr[] = {
            "Enables CSV (comma separated values) mode.\n"
        };

        argv [0] = _RWSTD_CONST_CAST (char*, helpstr);

        return 0;
    }

    _rw_opt_csv = 1;
    return 0;
}

/************************************************************************/

static int
_rw_opt_compat (int argc, char *argv[])
{
    static int opt_compat;

    if (0 == argc) {
        // query mode: return the value of the option
        return opt_compat;
    }

    if (1 == argc && argv && 0 == argv [0]) {
        // help mode: set argv[0] to the text of the help message

        static const char helpstr[] = {
            "Enables RWTest-format compatibility mode.\n"
        };

        argv [0] = _RWSTD_CONST_CAST (char*, helpstr);

        return 0;
    }

    // set mode: enable the option
    opt_compat = 1;

    return 0;
}

/************************************************************************/

static int
_rw_opt_no_stdout (int argc, char *argv[])
{
    static int opt_no_stdout;

    if (0 == argc) {
        // query mode: return the value of the option
        return opt_no_stdout;
    }

    if (1 == argc && argv && 0 == argv [0]) {
        // help mode: set argv[0] to the text of the help message

        static const char helpstr[] = {
            "Prevents the program from using stdandard output for diagnostic\n"
            "messages. Instead, the driver will create a log file with a name\n"
            "obtained from the from the basename of the program source file,\n"
            "usually obtained by passing the value of the __FILE__ macro to\n"
            "the driver, with the .out extension. If successful, the driver\n"
            "will write all diagnostic messages issued by the program to this\n"
            "file. Otherwise, the driver exits with an error.\n"
        };

        argv [0] = _RWSTD_CONST_CAST (char*, helpstr);

        return 0;
    }
    
    // set mode: enable the option
    opt_no_stdout = 1;

    return 0;
}

/************************************************************************/

static int
_rw_setopt_output_file (int argc, char *argv[])
{
    if (1 == argc && argv && 0 == argv [0]) {
        static const char helpstr[] = {
            "Specifies the name of the output file to be used by the program\n"
            "for diagnostic messages. Unless this option is specified, the\n"
            "program will issue all diagnostic messages to the standard output."
            "\nDriver diagnostics are always directed to stderr.\n"
        };

        argv [0] = _RWSTD_CONST_CAST (char*, helpstr);

        return 0;
    }

    const char *file_name = 0;

    if ('-' == argv [0][0] && 'O' == argv [0][1] || 'o' == argv [0][1]) {
        file_name = argv [0] + 2;
    }
    else if (1 < argc && '-' != argv [1][0]) {
        file_name = argv [1];
    }

    if (file_name) {

        FILE* const f = fopen (file_name, "w");

        if (f) {
            if (ftestout && ftestout != stderr)
                fclose (ftestout);

            ftestout = f;
        }
    }

    // return 0 on success, any non-zero value on failure
    return !(ftestout != 0);
}

/************************************************************************/

_TEST_EXPORT int
rw_vsetopts (const char *opts, va_list va);

/************************************************************************/

static int
_rw_use_color ()
{
#ifndef _RWSTD_NO_ISATTY

    // is output sent to a terminal?
    // if so, assume a vt100 compatible terminal for now
    static const int tty = isatty (fileno (ftestout));

#else   // if defined (_RWSTD_NO_ISATTY)

    // FIXME: deal with a missing isatty() and Windows
    static const int tty = 0;

#endif   // _RWSTD_NO_ISATTY

    return 0 != tty;
}

/************************************************************************/

_TEST_EXPORT int
rw_vtest (int argc, char **argv,
          const char *file_name,
          const char *clause,
          const char *comment,
          int (*fun)(int, char**),
          const char *optstr,
          va_list     va)
{
    CHECK_INIT (false, "rw_vtest()");

    driver_initialized = 1;

    if (optstr && 0 > rw_vsetopts (optstr, va)) {
        fprintf (stderr, "%s:%d: rw_setopts() failed\n", __FILE__, __LINE__);
        std_log(LOG_FILENAME_LINE,"%s:%d: rw_setopts() failed\n", __FILE__, __LINE__);
        return 1;
    }

    const int nopts =
        rw_setopts ("|-no-stdout "
                    "|-diags= "      // argument required
                    "|-trace "
                    "|-severity= "   // argument required
                    "|-csv "
                    "|-compat "
                    "o|-output:"     // argument optional
                    "b|-brief "
                    "q|-quiet "
                    "v|-verbose",
                    _rw_opt_no_stdout,
                    _rw_setopt_diags,
                    _rw_setopt_trace,
                    _rw_setopt_trace_mask,
                    _rw_setopt_csv,
                    _rw_opt_compat,
                    _rw_setopt_output_file,
                    _rw_opt_brief,
                    _rw_opt_quiet,
                    _rw_opt_verbose,
                    0);

    if (3 > nopts) {
        fprintf (stderr, "%s:%d: rw_setopts() failed\n", __FILE__, __LINE__);
        std_log(LOG_FILENAME_LINE,"%s:%d: rw_setopts() failed\n", __FILE__, __LINE__);
        abort ();
        return 1;
    }

#ifndef _RWSTD_USE_CONFIG

    // enable RWTest-format compatibility mode
    _rw_opt_compat (1, 0);

    // disable output to stdout
    _rw_opt_no_stdout (1, 0);

#endif   // _RWSTD_USE_CONFIG

    _rw_setopts_types ();

    _rw_setopts_lines ();

    int status = rw_runopts (argc, argv);

    if (status)
        return status;

    if (0 == ftestout) {

        if (_rw_opt_no_stdout (0, 0) && file_name) {
            char fname [256] = "C:\\";
            
            char* temp_ret = strchr (file_name, '/');
            
            if(temp_ret != NULL)
            {    
             const char* const slash = strrchr (file_name, '/');

             strcat (fname, slash ? slash + 1 : file_name);
            }
            else
            {    
             const char* const slash = strrchr (file_name, _RWSTD_PATH_SEP);

             strcat (fname, slash ? slash + 1 : file_name);
            }
            
            char* const dot = strchr (fname, '.');
            if (dot)
                strcpy (dot, ".out");
            else
                strcat (fname, ".out");

            ftestout = fopen (fname, "w");
        }
        else
            ftestout = stdout;
    }

    if (clause)
        strcpy (clause_id, clause);

    const char begin_fmt[] = {
        "\n"
        "# COMPILER: %s\n"
        "# ENVIRONMENT: %s\n"
        "# FILE: %s\n"
        "# COMPILED: %s, %s\n"
        "# COMMENT: %s\n"
        "######################################################"
    };

    const char* const fname = strrchr (file_name, _RWSTD_PATH_SEP);

    rw_info (0, 0, 0,
             begin_fmt,
             RW_TEST_COMPILER, RW_TEST_HARDWARE,
             fname ? fname + 1 : file_name,
             __DATE__, __TIME__,
             comment ? comment : "");

    status = setjmp (test_env);

    if (0 == status) {
        // environment set, execute the callback function
        status = fun (argc, argv);
    }
    else {
        // fatal test error (via a call to rw_fatal())
    }

    driver_finished = 1;

    static const char tblrow[] =
        "+-----------------------+--------+--------+--------+";

    fprintf (ftestout,
             "# %s\n"
             "# | DIAGNOSTIC            | ACTIVE |  TOTAL |INACTIVE|\n"
             "# %s\n",
             tblrow, tblrow);

    int nlines = 0;

    for (int i = 0; i != N_DIAG_TYPES; ++i) {
        if (ndiags [i][0] || !(_rw_diag_mask & (1 << diag_trace))) {

            // print out details for any non-zero totals
            // or for all totals when debugging or tracing
            // is enabled

            ++nlines;

            const long num = (ndiags [i][0] - ndiags [i][1]) * 100L;
            const long den = ndiags [i][0];

            const long pct = den ? num / den : 0;

            const char* pfx = "";
            const char* sfx = "";

            static int use_color = _rw_use_color ();

            if (use_color) {
                pfx = ndiags [i][1] ? diag_msgs [i].esc_pfx : "";
                sfx = ndiags [i][1] ? diag_msgs [i].esc_sfx : "";
            }

            fprintf (ftestout,
                     "# | (S%d) %-*s |%s %6d %s| %6d | %5ld%% |\n",
                     i, int (sizeof diag_msgs [i].code), diag_msgs [i].code,
                     pfx, ndiags [i][1], sfx, ndiags [i][0], pct);
        }
    }

    if (0 == nlines)
        fprintf (ftestout, "# no diagnostics\n");

    fprintf (ftestout, "# %s\n", tblrow);

    if (_rw_opt_compat (0, 0)) {

        // TO DO: get rid of this

        // RWTest compatibility format

        fprintf (ftestout,
                 "######################################################\n"
                 "## Warnings = %d\n"
                 "## Assertions = %d\n"
                 "## FailedAssertions = %d\n",
                 ndiags [diag_warn][1] + ndiags [diag_xwarn][1],
                 ndiags [diag_assert][0],
                 ndiags [diag_assert][1] + ndiags [diag_xassert][1]);
    }

    fclose (ftestout);
    ftestout = 0;

    return status;
}

/************************************************************************/

_TEST_EXPORT int
rw_test (int argc, char **argv,
         const char *fname,
         const char *clause,
         const char *comment,
         int (*testfun)(int, char**),
         const char *optstr,
         ...)
{
    CHECK_INIT (false, "rw_test()");

    va_list va;
    va_start (va, optstr);

    const int status =
        rw_vtest (argc, argv, fname, clause, comment, testfun, optstr, va);

    va_end (va);

    return status;
}

/************************************************************************/

// escape every occurrence of the double quote character in the string
// pointed to by buf by prepending to it the escape character specified
// by the last acrgument
// returns the new buffer if the size of the existing buffer isn't
// sufficient and sets *pbufsize to the size of the newly allocated
// buffer, otherwise the original value of buf and leaves *pbufsize
// unchanged
static char*
_rw_escape (char *buf, size_t bufsize, char esc)
{
    // handle null buffer
    if (0 == buf)
        return buf;

    // count the number of embedded quotes
    char *quote = buf;
    size_t nquotes = 0;
    
    #ifdef __ARMCC__
    #pragma diag_suppress 1293
    #endif
    while ((quote = strchr (quote, '"'))) {
        ++nquotes;
        ++quote;
    }
    

    // no quotes found, return the original buffer
    if (0 == nquotes)
        return buf;

    // conpute the size of the buffer that will be needed to escape
    // all the double quotes
    size_t newbufsize = strlen (buf) + nquotes + 1;

    char *newbuf = 0;

    if (0 /* newbufsize <= bufsize */) {
        // FIXME: escape embedded quotes in place w/o reallocation
        _RWSTD_UNUSED (bufsize);
    }
    else {
        newbuf = (char*)malloc (newbufsize);
        if (0 == newbuf) {
            return 0;
        }

        // set the next pointer to the beginning of the new buffer
        // as the destination where to copy the string argument
        char *next = newbuf;

        // set quote to initially point to the beginning of
        // the source buffer and then just past the last quote
        quote = buf;

        for (char *q = buf; ; ++q) {

            // look for the next (or first) quote
            q = strchr (q, '"');

            // compute the number of characters, excluding the quote
            // to copy to the destination buffer
            const size_t nchars = q ? size_t (q - quote) : strlen (quote);

            memcpy (next, quote, nchars);

            if (q) {
                // append the escape character to the destination buffer
                next [nchars] = esc;

                // append the quote from the source string
                next [nchars + 1] = '"';

                // advance the destination pointer past the quote
                next += nchars + 2;

                // advance the source pointer past the embedded quote
                quote = q + 1;
            }
            else {
                // NUL-terminate the destination buffer
                *next = '\0';
                break;
            }
        }
    }

    return newbuf;
}

/************************************************************************/

static void
_rw_vissue_diag (diag_t diag, int severity, const char *file, int line,
                 const char *fmt, va_list va)
{
    CHECK_INIT (true, "_rw_vissue_diag()");

    if (0 == fmt)
        fmt = "";

    static char fmterr[] = "*** formatting error ***";

    char *usrbuf = 0;
    const int nchars = rw_vasnprintf (&usrbuf, 0, fmt, va);

    if (nchars < 0 || 0 == usrbuf)
        usrbuf = fmterr;

    // compute the number of newline characters in the text
    int nlines = 0;
    
    #ifdef __ARMCC__
    #pragma diag_suppress 1293
    #endif
    for (const char *nl = usrbuf; (nl = strchr (nl, '\n')); ++nl)
        ++nlines;
    

    static const int use_color = _rw_use_color ();

    const char* const diagstr[] = {
        use_color ? diag_msgs [severity].esc_pfx : "",
        *diag_msgs [severity].code ? diag_msgs [severity].code : "UNKNOWN",
        use_color  ? diag_msgs [severity].esc_sfx : "",
        _rw_opt_verbose (0, 0) && *diag_msgs [severity].desc ?
        diag_msgs [severity].desc : 0
    };

    const char* const traced_diag =
        0 == severity && diag_msgs [diag].code ? diag_msgs [diag].code : 0;

    const char* const slash = file ? strrchr (file, _RWSTD_PATH_SEP) : 0;
    if (slash)
        file = slash + 1;

    char *mybuf = 0;

    if (_rw_opt_csv) {

        // format all fields as comma separated values (CSV):
        // -- a field containing the quote character, the comma,
        //    or the newline or linefeed character must be enclosed
        //    in a pair of double quotes
        // -- every occurrence of the double quote character in a field
        //    must be escaped by prepening another double quote character
        //    to it

        // escape all double quotes by prepending the double
        // quote character to each according to the CSV format
        char* const newbuf = _rw_escape (usrbuf, 0, '"');
        if (newbuf != usrbuf) {
            free (usrbuf);
            usrbuf = newbuf;
        }

        mybuf =
            rw_sprintfa ("%d, "                      // severity
                         "\"%s%s"                    // diagnostic
                         "%{?}_%s%{;}%s\", "         // traced diagnostic
                         "\"%s\", "                  // clause
                         "\"%s\", "                  // file
                         "%d, "                      // line
                         "\"%s\"",                   // user text
                         severity,
                         diagstr [0], diagstr [1],
                         0 != traced_diag, traced_diag, diagstr [2],
                         clause_id,
                         0 != file ? file : "",
                         line,
                         usrbuf);
    }
    else {

        nlines += 2 + ('\0' != *clause_id) + (0 != file) + (0 < line);

        mybuf =
            rw_sprintfa ("# %s"                      // escape prefix
                         "%s"                        // diagnostic
                         "%{?}_%s%{;}"               // traced diagnostic
                         "%s "                       // escape suffix
                         "(S%d)"                     // severity
                         "%{?}, %s%{;} "             // description
                         "(%d lines):\n"             // number of lines
                         "# TEXT: %s\n"              // user text
                         "%{?}# CLAUSE: %s\n%{;}"    // clause if not empty
                         "%{?}# FILE: %s\n%{;}"      // file if not null
                         "%{?}# LINE: %d\n%{;}",     // line if positive
                         diagstr [0],
                         diagstr [1],
                         0 != traced_diag, traced_diag,
                         diagstr [2],
                         severity,
                         0 != diagstr [3], diagstr [3],
                         nlines,
                         usrbuf,
                         '\0' != *clause_id, clause_id,
                         0 != file, file,
                         0 < line, line);
    }
#if 0   // disabled
    else {

        mybuf =
            rw_sprintfa ("# %s%s"                 // diagnostic
                         "%{?}_%s%{;}%s "         // traced diagnostic
                         "(S%d): "                // severity
                         "%{?}[%s] %{;}"          // clause if not empty
                         "%{?}(%d lines): %{;}"   // number of lines if > 1
                         "%{?}%s:"                // if (file) then file
                         "%{?}%d:%{;} "           //   if (0 < line) line
                         "%{:}"                   // else
                         "%{?}line %d: %{;}"      //   if (0 < line) line
                         "%{;}"                   // endif
                         "%s",                    // user text
                         diagstr [0], diagstr [1],
                         0 != traced_diag, traced_diag, diagstr [2],
                         severity,
                         '\0' != *clause_id, clause_id,
                         1 < nlines, nlines,
                         0 != file, file,
                         0 < line, line,
                         0 < line, line,
                         usrbuf);
    }
#endif   // 0/1

    fprintf (ftestout, "%s\n", mybuf);

    if (mybuf != fmterr)
        free (mybuf);

    if (usrbuf != fmterr)
        free (usrbuf);
}

/************************************************************************/

static void
_rw_vdiag (diag_t diag, int severity, const char *file, int line,
           const char *fmt, va_list va)
{
    CHECK_INIT (true, "_rw_vdiag()");

    // check if the diagnostic is expected
    const int expected = 0 != _rw_expected (line);

    if (expected) {
        if (severity) {
            // if the diagnostic is expected to be active,
            // adjust its type and severity
            if (diag_assert == diag)
                diag = diag_xassert;
            else if (diag_warn == diag)
                diag = diag_xwarn;

            severity = diag * severity;
        }
        else {
            // if the diagnostic is expected to be active but isn't,
            // adjust its type to an unexpectdly inactive one
            if (diag_assert == diag || diag_warn == diag)
                diag = diag_expect;

            severity = diag;
        }
    }
    else if (diag) {
        // normalize the severity
        severity = diag * severity;
    }

    if (severity < 0)
        severity = 0;
    else if (N_DIAG_TYPES <= severity)
        severity = N_DIAG_TYPES - 1;

    // increment the diagnostic counter
    ++ndiags [diag][0];

    if (severity) {

        ++ndiags [diag][1];
    }

    const int sevbit = (1 << severity);

    if (0 == (sevbit & _rw_diag_mask)) {
        // issue the diagnostic
        _rw_vissue_diag (diag, severity, file, line, fmt, va);
    }

    if (diag_fatal == diag && severity) {
        // fatal error, terminate test
        longjmp (test_env, severity);
    }
}

/************************************************************************/

_TEST_EXPORT int
rw_fatal (int success, const char *file, int line, const char *fmt, ...)
{
    CHECK_INIT (true, "rw_fatal()");

    va_list va;
    va_start (va, fmt);

    _rw_vdiag (diag_fatal, 0 == success, file, line, fmt, va);

    va_end (va);

    return success;
}

/************************************************************************/

_TEST_EXPORT int
rw_error (int success, const char *file, int line, const char *fmt, ...)
{
    CHECK_INIT (true, "rw_error()");

    va_list va;
    va_start (va, fmt);

    _rw_vdiag (diag_error, 0 == success, file, line, fmt, va);

    va_end (va);

    return success;
}

/************************************************************************/

_TEST_EXPORT int
rw_assert (int success, const char *file, int line, const char *fmt, ...)
{
    CHECK_INIT (true, "rw_assert()");

    va_list va;
    va_start (va, fmt);

    _rw_vdiag (diag_assert, 0 == success, file, line, fmt, va);

    va_end (va);

    return success;
}

/************************************************************************/

_TEST_EXPORT int
rw_warn (int success, const char *file, int line, const char *fmt, ...)
{
    CHECK_INIT (true, "rw_warn()");

    va_list va;
    va_start (va, fmt);

    _rw_vdiag (diag_warn, 0 == success, file, line, fmt, va);

    va_end (va);

    return success;
}

/************************************************************************/

_TEST_EXPORT int
rw_note (int success, const char *file, int line, const char *fmt, ...)
{
    CHECK_INIT (true, "rw_note()");

    va_list va;
    va_start (va, fmt);

    _rw_vdiag (diag_note, 0 == success, file, line, fmt, va);

    va_end (va);

    return success;
}

/************************************************************************/

_TEST_EXPORT int
rw_info (int success, const char *file, int line, const char *fmt, ...)
{
    CHECK_INIT (true, "rw_info()");

    va_list va;
    va_start (va, fmt);

    _rw_vdiag (diag_info, 0 == success, file, line, fmt, va);

    va_end (va);

    return success;
}
