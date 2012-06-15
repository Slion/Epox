/*
 * � Portions copyright (c) 2006-2007 Nokia Corporation.  All rights reserved.
 *
 * Copyright (c) 1999
 * Silicon Graphics Computer Systems, Inc.
 *
 * Copyright (c) 1999 
 * Boris Fomitchev
 *
 * This material is provided "as is", with absolutely no warranty expressed
 * or implied. Any use is at your own risk.
 *
 * Permission to use or copy this software for any purpose is hereby granted 
 * without fee, provided the above notices are retained on all copies.
 * Permission to modify the code and to distribute modified code is granted,
 * provided the above notices are retained, and a notice that the code was
 * modified is included with the above copyright notice.
 *
 */ 
# include "stlport_prefix.h"

#include <istream>
// #include <stl/_istream.h>
#if defined(__LIBSTD_CPP_SYMBIAN32_WSD__) || defined(_STLP_LIBSTD_CPP_NO_STATIC_VAR_)
# include "libstdcppwsd.h"
# endif

#include <stl/_fstream.h>
#include <stdio_streambuf>
#include "aligned_buffer.h"

// boris : note this is repeated in <iostream>
#ifndef _STLP_USE_NAMESPACES
// in case of SGI iostreams, we have to rename our streams not to clash with those
// provided in native lib
# define cin _STLP_cin
# define cout _STLP_cout
# define cerr _STLP_cerr
# define clog _STLP_clog
#endif

_STLP_BEGIN_NAMESPACE

#if defined (__BORLANDC__) && ! defined (_STLP_USE_GLIBC)
using _STLP_VENDOR_CSTD::_streams;
#endif

// This file handles iostream initialization.  It is inherently
// nonportable, since the C++ language definition provides no mechanism
// for controlling order of initialization of nonlocal objects.  
// Initialization has three parts, which must be performed in the following
// order:
//  (1) Initialize the locale system
//  (2) Call the constructors for the eight global stream objects.
//  (3) Create streambufs for the global stream objects, and initialize
//      the stream objects by calling the init() member function.


#if defined (_STLP_MSVC) || defined(__MWERKS__) || defined (__ICL) || defined (__ISCPP__) || defined (__SYMBIAN32__)
// Definitions of the eight global I/O objects that are declared in 
// <iostream>. For VC++ we use the init_seg pragma to put the global I/O
// objects into an intitialization segement that will not
// be executed. We then explicitly invoke the constructors
// with placement new in ios_base::_S_initialize() 

#if defined(__MWERKS__)
# pragma suppress_init_code on
#else
# pragma init_seg("STLPORT_NO_INIT")
#endif

# if !defined(__LIBSTD_CPP_SYMBIAN32_WSD__) && !defined(_STLP_LIBSTD_CPP_NO_STATIC_VAR_)
_STLP_EXP_DECLSPEC istream cin(0);
_STLP_EXP_DECLSPEC ostream cout(0);
_STLP_EXP_DECLSPEC ostream cerr(0);
_STLP_EXP_DECLSPEC ostream clog(0);

#ifndef _STLP_NO_WCHAR_T
_STLP_EXP_DECLSPEC wistream wcin(0);
_STLP_EXP_DECLSPEC wostream wcout(0);
_STLP_EXP_DECLSPEC wostream wcerr(0);
_STLP_EXP_DECLSPEC wostream wclog(0);
#endif
# endif

_STLP_EXP_DECLSPEC ostream& GetCErrStream()
    {
    return *getCErrStream();
    }

_STLP_EXP_DECLSPEC ostream& GetCoutStream()
    {
    return *getCoutStream();
    }

_STLP_EXP_DECLSPEC ostream& GetClogStream()
    {
    return *getClogStream();
    }

_STLP_EXP_DECLSPEC istream& GetCinStream()
    {
    return *getCinStream();
    }

#ifndef _STLP_NO_WCHAR_T    
_STLP_EXP_DECLSPEC wostream& GetWCErrStream()
    {
    return *getWCErrStream();
    }

_STLP_EXP_DECLSPEC wostream& GetWCoutStream()
    {
    return *getWCoutStream();
    }

_STLP_EXP_DECLSPEC wostream& GetWClogStream()
    {
    return *getWClogStream();
    }

_STLP_EXP_DECLSPEC wistream& GetWCinStream()
    {
    return *getWCinStream();
    }   
#endif //_STLP_NO_WCHAR_T    

#if defined(__MWERKS__)
# pragma suppress_init_code off
#endif

#else


// Definitions of the eight global I/O objects that are declared in 
// <iostream>.  Disgusting hack: we deliberately define them with the
// wrong types so that the constructors don't get run automatically.
// We need special tricks to make sure that these objects are struct-
// aligned rather than byte-aligned.

// This is not portable.  Declaring a variable with different types in
// two translations units is "undefined", according to the C++ standard.
// Most compilers, however, silently accept this instead of diagnosing
// it as an error.

#ifndef __DMC__
_Stl_aligned_buffer<istream> cin;
_Stl_aligned_buffer<ostream> cout;
_Stl_aligned_buffer<ostream> cerr;
_Stl_aligned_buffer<ostream> clog;
#else
_Stl_aligned_buffer<istream> cin;
_Stl_aligned_buffer<ostream> cout;
_Stl_aligned_buffer<ostream> cerr;
_Stl_aligned_buffer<ostream> clog;

#pragma alias("?cin@std@@3V?$basic_istream@std@DV?$char_traits@std@D@1@@1@A", "?cin@std@@3T?$_Stl_aligned_buffer@std@V?$basic_istream@std@DV?$char_traits@std@D@1@@1@@1@A")
#pragma alias("?cout@std@@3V?$basic_ostream@std@DV?$char_traits@std@D@1@@1@A", "?cout@std@@3T?$_Stl_aligned_buffer@std@V?$basic_ostream@std@DV?$char_traits@std@D@1@@1@@1@A")
#pragma alias("?cerr@std@@3V?$basic_ostream@std@DV?$char_traits@std@D@1@@1@A", "?cerr@std@@3T?$_Stl_aligned_buffer@std@V?$basic_ostream@std@DV?$char_traits@std@D@1@@1@@1@A")
#pragma alias("?clog@std@@3V?$basic_ostream@std@DV?$char_traits@std@D@1@@1@A", "?clog@std@@3T?$_Stl_aligned_buffer@std@V?$basic_ostream@std@DV?$char_traits@std@D@1@@1@@1@A")
#endif

# ifndef _STLP_NO_WCHAR_T

#ifndef __DMC__
_Stl_aligned_buffer<wistream> wcin;
_Stl_aligned_buffer<wostream> wcout;
_Stl_aligned_buffer<wostream> wcerr;
_Stl_aligned_buffer<wostream> wclog;
#else
_Stl_aligned_buffer<wistream> wcin;
_Stl_aligned_buffer<wostream> wcout;
_Stl_aligned_buffer<wostream> wcerr;
_Stl_aligned_buffer<wostream> wclog;

#pragma alias("?wcin@std@@3V?$basic_istream@std@_YV?$char_traits@std@_Y@1@@1@A", "?wcin@std@@3T?$_Stl_aligned_buffer@std@V?$basic_istream@std@_YV?$char_traits@std@_Y@1@@1@@1@A")
#pragma alias("?wcout@std@@3V?$basic_ostream@std@_YV?$char_traits@std@_Y@1@@1@A", "?wcout@std@@3T?$_Stl_aligned_buffer@std@V?$basic_ostream@std@_YV?$char_traits@std@_Y@1@@1@@1@A")
#pragma alias("?wcerr@std@@3V?$basic_ostream@std@_YV?$char_traits@std@_Y@1@@1@A", "?wcerr@std@@3T?$_Stl_aligned_buffer@std@V?$basic_ostream@std@_YV?$char_traits@std@_Y@1@@1@@1@A")
#pragma alias("?wclog@std@@3V?$basic_ostream@std@_YV?$char_traits@std@_Y@1@@1@A", "?wclog@std@@3T?$_Stl_aligned_buffer@std@V?$basic_ostream@std@_YV?$char_traits@std@_Y@1@@1@@1@A")
#endif
# endif

#endif /* STL_MSVC || __MWERKS__ */

// Member functions from class ios_base and ios_base::Init

#if defined(__LIBSTD_CPP_SYMBIAN32_WSD__) || defined(_STLP_LIBSTD_CPP_NO_STATIC_VAR_)
void ios_base_Init_S_count_init()
{
	get_ios_base_Init_S_count() = 0;	
}
void ios_base_S_was_synced_init()
{
	get_ios_base_S_was_synced() = true;	
}
# else
long ios_base::Init::_S_count = 0;
// by default, those are synced
bool ios_base::_S_was_synced = true;
# endif //__LIBSTD_CPP_SYMBIAN32_WSD__


_STLP_DECLSPEC extern void LibStdCppInit();


void CallIosInit()
    {
    ios_base::_S_initialize();
    }


_STLP_EXP_DECLSPEC ios_base::Init::Init() {
  //    if (_S_count == 0)
      //ios_base::_S_initialize();
      LibStdCppInit();
}

_STLP_EXP_DECLSPEC ios_base::Init::~Init() {
  //    if (_S_count > 0)
      ios_base::_S_uninitialize();
}


filebuf*
_Stl_create_filebuf(FILE* f, ios_base::openmode mode )
{
  basic_filebuf<char, char_traits<char> >* result;
  
  result = new basic_filebuf<char, char_traits<char> >();

  _STLP_TRY {
    result->_M_open(_FILE_fd(f), mode);
  }
  _STLP_CATCH_ALL {}

  if (!result->is_open()) {
    delete result;
    result = 0;
  }
  return result;
}

# ifndef _STLP_NO_WCHAR_T

wfilebuf*
_Stl_create_wfilebuf(FILE* f, ios_base::openmode mode )
{
  basic_filebuf<wchar_t, char_traits<wchar_t> >* result;
  
  result = new basic_filebuf<wchar_t, char_traits<wchar_t> >();

  _STLP_TRY {
    result->_M_open(_FILE_fd(f), mode);
  }
  _STLP_CATCH_ALL {}

  if (!result->is_open()) {
    delete result;
    result = 0;
  }
  return result;
}

# endif

void  _STLP_CALL ios_base::_S_initialize()
{
#if defined(__LIBSTD_CPP_SYMBIAN32_WSD__) || defined(_STLP_LIBSTD_CPP_NO_STATIC_VAR_)
	if (get_ios_base_Init_S_count()++ > 0) 
    	return ;
# else
	if (ios_base::Init::_S_count++ > 0) 
    	return ;
# endif //__LIBSTD_CPP_SYMBIAN32_WSD__
  
# if !defined(_STLP_HAS_NO_NAMESPACES) && !defined(_STLP_WINCE)
  using _SgI::stdio_istreambuf;
  using _SgI::stdio_ostreambuf;
# endif
  _STLP_TRY {
    // Run constructors for the four narrow stream objects.
    // check with locale system
    //    if (_Loc_init::_S_count == 0) {
      locale::_S_initialize();
      //    }
#if !defined(_STLP_WINCE)
#if defined(__LIBSTD_CPP_SYMBIAN32_WSD__) || defined(_STLP_LIBSTD_CPP_NO_STATIC_VAR_)
    istream* ptr_cin  = new((void*)&GetCinStream())  istream(0);
    ostream* ptr_cout = new((void*)&GetCoutStream()) ostream(0);
    ostream* ptr_cerr = new((void*)&GetCErrStream()) ostream(0);
    ostream* ptr_clog = new((void*)&GetClogStream()) ostream(0);
# else
	istream* ptr_cin  = new((void*)&cin)  istream(0);
    ostream* ptr_cout = new((void*)&cout) ostream(0);
    ostream* ptr_cerr = new((void*)&cerr) ostream(0);
    ostream* ptr_clog = new((void*)&clog) ostream(0);
# endif //__LIBSTD_CPP_SYMBIAN32_WSD__
    // Initialize the four narrow stream objects.
#if defined(__LIBSTD_CPP_SYMBIAN32_WSD__) || defined(_STLP_LIBSTD_CPP_NO_STATIC_VAR_)
	if (get_ios_base_S_was_synced()) {
# else
	if (_S_was_synced) {
# endif        
      ptr_cin->init(new stdio_istreambuf(stdin));
      ptr_cout->init(new stdio_ostreambuf(stdout));
      ptr_cerr->init(new stdio_ostreambuf(stderr));
      ptr_clog->init(new stdio_ostreambuf(stderr));
    } else {
      ptr_cin->init(_Stl_create_filebuf(stdin, ios_base::in));
      ptr_cin->init(_Stl_create_filebuf(stdout, ios_base::out));
      ptr_cin->init(_Stl_create_filebuf(stderr, ios_base::out));
      ptr_cin->init(_Stl_create_filebuf(stderr, ios_base::out)); 
    }
    ptr_cin->tie(ptr_cout);
    ptr_cerr->setf(ios_base::unitbuf);

# ifndef _STLP_NO_WCHAR_T
#if defined(__LIBSTD_CPP_SYMBIAN32_WSD__) || defined(_STLP_LIBSTD_CPP_NO_STATIC_VAR_)
    // Run constructors for the four wide stream objects.
    wistream* ptr_wcin  = new(&GetWCinStream())  wistream(0);
    wostream* ptr_wcout = new(&GetWCoutStream()) wostream(0);
    wostream* ptr_wcerr = new(&GetWCErrStream()) wostream(0);
    wostream* ptr_wclog = new(&GetWClogStream()) wostream(0);
# else
	// Run constructors for the four wide stream objects.
    wistream* ptr_wcin  = new(&wcin)  wistream(0);
    wostream* ptr_wcout = new(&wcout) wostream(0);
    wostream* ptr_wcerr = new(&wcerr) wostream(0);
    wostream* ptr_wclog = new(&wclog) wostream(0);
# endif //__LIBSTD_CPP_SYMBIAN32_WSD__

    wfilebuf* win  = _Stl_create_wfilebuf(stdin, ios_base::in);
    wfilebuf* wout = _Stl_create_wfilebuf(stdout, ios_base::out);;
    wfilebuf* werr = _Stl_create_wfilebuf(stderr, ios_base::out);
    wfilebuf* wlog = _Stl_create_wfilebuf(stderr, ios_base::out);

    ptr_wcin->init(win);
    ptr_wcout->init(wout);
    ptr_wcerr->init(werr);
    ptr_wclog->init(wlog);

    ptr_wcin->tie(ptr_wcout);
    ptr_wcerr->setf(ios_base::unitbuf);
    
# endif /*  _STLP_NO_WCHAR_T */
#endif /* _STLP_WINCE */

    //    ++ios_base::Init::_S_count;
  }

  _STLP_CATCH_ALL {}
}

void _STLP_CALL ios_base::_S_uninitialize()
{
#if defined(__LIBSTD_CPP_SYMBIAN32_WSD__) || defined(_STLP_LIBSTD_CPP_NO_STATIC_VAR_)
  if (--get_ios_base_Init_S_count() != 0) 
    return ;
# else
	  if (--ios_base::Init::_S_count != 0) 
    return ;
# endif //__LIBSTD_CPP_SYMBIAN32_WSD__

#if defined(__LIBSTD_CPP_SYMBIAN32_WSD__) || defined(_STLP_LIBSTD_CPP_NO_STATIC_VAR_)
  // Note that destroying output streambufs flushes the buffers.
  istream* ptr_cin  = __REINTERPRET_CAST(istream*,&GetCinStream());
  ostream* ptr_cout = __REINTERPRET_CAST(ostream*,&GetCoutStream());
  ostream* ptr_cerr = __REINTERPRET_CAST(ostream*,&GetCErrStream());
  ostream* ptr_clog = __REINTERPRET_CAST(ostream*,&GetClogStream());

# ifndef _STLP_NO_WCHAR_T
  wistream* ptr_wcin  = __REINTERPRET_CAST(wistream*,&GetWCinStream());
  wostream* ptr_wcout = __REINTERPRET_CAST(wostream*,&GetWCoutStream());
  wostream* ptr_wcerr = __REINTERPRET_CAST(wostream*,&GetWCErrStream());
  wostream* ptr_wclog = __REINTERPRET_CAST(wostream*,&GetWClogStream());
# endif //_STLP_NO_WCHAR_T
# else
  // Note that destroying output streambufs flushes the buffers.
  istream* ptr_cin  = __REINTERPRET_CAST(istream*,&cin);
  ostream* ptr_cout = __REINTERPRET_CAST(ostream*,&cout);
  ostream* ptr_cerr = __REINTERPRET_CAST(ostream*,&cerr);
  ostream* ptr_clog = __REINTERPRET_CAST(ostream*,&clog);

# ifndef _STLP_NO_WCHAR_T
  wistream* ptr_wcin  = __REINTERPRET_CAST(wistream*,&wcin);
  wostream* ptr_wcout = __REINTERPRET_CAST(wostream*,&wcout);
  wostream* ptr_wcerr = __REINTERPRET_CAST(wostream*,&wcerr);
  wostream* ptr_wclog = __REINTERPRET_CAST(wostream*,&wclog);
# endif
# endif //__LIBSTD_CPP_SYMBIAN32_WSD__


  // we don't want any exceptions being thrown here
  ptr_cin->exceptions(0);
  ptr_cout->exceptions(0);
  ptr_cerr->exceptions(0);
  ptr_clog->exceptions(0);

  delete ptr_cin->rdbuf(0);
  delete ptr_cout->rdbuf(0);
  delete ptr_cerr->rdbuf(0);
  delete ptr_clog->rdbuf(0);

  _Destroy(ptr_cin);
  _Destroy(ptr_cout);
  _Destroy(ptr_cerr);
  _Destroy(ptr_clog);

# ifndef _STLP_NO_WCHAR_T
  // we don't want any exceptions being thrown here
  ptr_wcin->exceptions(0);
  ptr_wcout->exceptions(0);
  ptr_wcerr->exceptions(0);
  ptr_wclog->exceptions(0);

  delete ptr_wcin->rdbuf(0);
  delete ptr_wcout->rdbuf(0);
  delete ptr_wcerr->rdbuf(0);
  delete ptr_wclog->rdbuf(0);

  _Destroy(ptr_wcin);
  _Destroy(ptr_wcout);
  _Destroy(ptr_wcerr);
  _Destroy(ptr_wclog);

# endif

#if defined(__LIBSTD_CPP_SYMBIAN32_WSD__) || defined(_STLP_LIBSTD_CPP_NO_STATIC_VAR_)
	if (get_ios_base_Loc_init_S_count() > 0) {
# else
    if (_Loc_init::_S_count > 0) {
# endif // __LIBSTD_CPP_SYMBIAN32_WSD__  
      locale::_S_uninitialize();
    }

#if defined(__LIBSTD_CPP_SYMBIAN32_WSD__) || defined(_STLP_LIBSTD_CPP_NO_STATIC_VAR_)
	--get_ios_base_Init_S_count();
# else
	--ios_base::Init::_S_count;
# endif //__LIBSTD_CPP_SYMBIAN32_WSD__
    
}


_STLP_EXP_DECLSPEC bool _STLP_CALL ios_base::sync_with_stdio(bool sync) {
#if !defined(STLP_WINCE)
# ifndef _STLP_HAS_NO_NAMESPACES
  using _SgI::stdio_istreambuf;
  using _SgI::stdio_ostreambuf;
# endif

#if defined(__LIBSTD_CPP_SYMBIAN32_WSD__) || defined(_STLP_LIBSTD_CPP_NO_STATIC_VAR_)
	bool was_synced =  get_ios_base_S_was_synced();
# else
	bool was_synced =  _S_was_synced;
# endif  //__LIBSTD_CPP_SYMBIAN32_WSD__

  // if by any chance we got there before std streams initialization,
  // just set the sync flag and exit
#if defined(__LIBSTD_CPP_SYMBIAN32_WSD__) || defined(_STLP_LIBSTD_CPP_NO_STATIC_VAR_)
	if (get_ios_base_Init_S_count() == 0) {
    get_ios_base_S_was_synced() = sync;
# else
	if (Init::_S_count == 0) {
    _S_was_synced = sync;
# endif //__LIBSTD_CPP_SYMBIAN32_WSD__ 
    return was_synced;
  }

#if defined(__LIBSTD_CPP_SYMBIAN32_WSD__) || defined(_STLP_LIBSTD_CPP_NO_STATIC_VAR_)
  istream* ptr_cin  = __REINTERPRET_CAST(istream*,&GetCinStream());
  ostream* ptr_cout = __REINTERPRET_CAST(ostream*,&GetCoutStream());
  ostream* ptr_cerr = __REINTERPRET_CAST(ostream*,&GetCErrStream());
  ostream* ptr_clog = __REINTERPRET_CAST(ostream*,&GetClogStream());
# else
  istream* ptr_cin  = __REINTERPRET_CAST(istream*,&cin);
  ostream* ptr_cout = __REINTERPRET_CAST(ostream*,&cout);
  ostream* ptr_cerr = __REINTERPRET_CAST(ostream*,&cerr);
  ostream* ptr_clog = __REINTERPRET_CAST(ostream*,&clog);	
# endif //__LIBSTD_CPP_SYMBIAN32_WSD__

  streambuf* old_cin  = ptr_cin->rdbuf();
  streambuf* old_cout = ptr_cout->rdbuf();
  streambuf* old_cerr = ptr_cerr->rdbuf();
  streambuf* old_clog = ptr_clog->rdbuf();

  streambuf* new_cin  = 0;
  streambuf* new_cout = 0;
  streambuf* new_cerr = 0;
  streambuf* new_clog = 0;

  _STLP_TRY {
    if (sync && !was_synced) {
      new_cin  = new stdio_istreambuf(stdin);
      new_cout = new stdio_ostreambuf(stdout);
      new_cerr = new stdio_ostreambuf(stderr);
      new_clog = new stdio_ostreambuf(stderr);
    }
    else if (!sync && was_synced) {
      new_cin  = _Stl_create_filebuf(stdin, ios_base::in);
      new_cout = _Stl_create_filebuf(stdout, ios_base::out);
      new_cerr = _Stl_create_filebuf(stderr, ios_base::out);
      new_clog = _Stl_create_filebuf(stderr, ios_base::out);
    }
  }
  _STLP_CATCH_ALL {}

  if (new_cin && new_cout && new_cerr && new_clog) {
    ptr_cin->rdbuf(new_cin);
    ptr_cout->rdbuf(new_cout);
    ptr_cerr->rdbuf(new_cerr);
    ptr_clog->rdbuf(new_clog);

    delete old_cin;
    delete old_cout;
    delete old_cerr;
    delete old_clog;
  }
  else {
    delete new_cin;
    delete new_cout;
    delete new_cerr;
    delete new_clog;
  }

  return was_synced;
#else
  return false;
#endif /* _STLP_WINCE */
}

_STLP_END_NAMESPACE

// Local Variables:
// mode:C++
// End:
