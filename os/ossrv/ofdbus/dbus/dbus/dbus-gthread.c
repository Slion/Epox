/* -*- mode: C; c-file-style: "gnu" -*- */
/* dbus-gthread.c  GThread integration
 *
 * Copyright (C) 2002  CodeFactory AB
 * Portion Copyright � 2008 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 * Licensed under the Academic Free License version 2.1
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/* #define G_DEBUG_LOCKS 1 */

#include <glib.h>
#include <dbus/dbus.h>
#include "dbus-glib.h"

/** @addtogroup DBusGLib
 * @{
 */

/**
 * dbus_g_thread_init:
 *
 * Initializes the D-BUS thread system.
 * This function may only be called
 * once and must be called prior to calling any
 * other function in the D-BUS API.
 */
 	#ifdef __SYMBIAN32__
	EXPORT_C
	#endif
void
dbus_g_thread_init (void)
{
  if (!g_thread_supported ())
    g_error ("g_thread_init() must be called before dbus_threads_init()");

  dbus_threads_init_default ();
}

/** @} end of public API */
