/************************************************************************
 *
 * system.h - declarations of testsuite helpers
 *
 * $Id: system.h 290012 2005-09-18 23:38:12Z sebor $
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

#ifndef RW_SYSTEM_H_INCLUDED
#define RW_SYSTEM_H_INCLUDED


#include <testdefs.h>


_TEST_EXPORT int
rw_system (const char *cmd, ...);


#endif   // RW_SYSTEM_H_INCLUDED
