/* GObject - GLib Type, Object, Parameter and Signal Library
 * Copyright (C) 2001, 2003 Red Hat, Inc.
 * Portions copyright (c) 2009 Nokia Corporation.  All rights reserved.
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#undef	G_LOG_DOMAIN
#define	G_LOG_DOMAIN "TestDynamicType"

#undef G_DISABLE_ASSERT
#undef G_DISABLE_CHECKS
#undef G_DISABLE_CAST_CHECKS

#include <glib-object.h>

#include "testcommon.h"
#include "testmodule.h"
#ifdef __SYMBIAN32__
#include "mrt2_glib2_test.h"
#endif //__SYMBIAN32__

/* This test tests the macros for defining dynamic types.
 */

static gboolean loaded = FALSE;

#define DYNAMIC_OBJECT_TYPE (dynamic_object_get_type ())

typedef GObject DynamicObject;
typedef struct _DynamicObjectClass DynamicObjectClass;

struct _DynamicObjectClass
{
  GObjectClass parent_class;
  guint val;
};

G_DEFINE_DYNAMIC_TYPE(DynamicObject, dynamic_object, G_TYPE_OBJECT);

static void 
dynamic_object_class_init (DynamicObjectClass *class)
{
  class->val = 42;
  loaded = TRUE;
}

static void
dynamic_object_class_finalize (DynamicObjectClass *class)
{
  loaded = FALSE;
}

static void 
dynamic_object_init (DynamicObject *dynamic_object)
{
}


static void
module_register (GTypeModule *module)
{
  dynamic_object_register_type (module);
}

static void
test_dynamic_type (void)
{
  GTypeModule *module;
  DynamicObjectClass *class;

  module = test_module_new (module_register);

  /* Not loaded until we call ref for the first time */
  class = g_type_class_peek (DYNAMIC_OBJECT_TYPE);
  g_assert (class == NULL);
  g_assert (!loaded);

  /* Ref loads */
  class = g_type_class_ref (DYNAMIC_OBJECT_TYPE);
  g_assert (class && class->val == 42);
  g_assert (loaded);

  /* Peek then works */
  class = g_type_class_peek (DYNAMIC_OBJECT_TYPE);
  g_assert (class && class->val == 42);
  g_assert (loaded);
  
  /* Unref causes finalize */
  g_type_class_unref (class);

  /* Peek returns NULL */
  class = g_type_class_peek (DYNAMIC_OBJECT_TYPE);
  g_assert (!class);
  g_assert (!loaded);
  
  /* Ref reloads */
  class = g_type_class_ref (DYNAMIC_OBJECT_TYPE);
  g_assert (class && class->val == 42);
  g_assert (loaded);

  /* And Unref causes finalize once more*/
  g_type_class_unref (class);
  class = g_type_class_peek (DYNAMIC_OBJECT_TYPE);
  g_assert (!class);
  g_assert (!loaded);
}

int
main (int   argc,
      char *argv[])
{
  g_log_set_always_fatal (g_log_set_always_fatal (G_LOG_FATAL_MASK) |
			  G_LOG_LEVEL_WARNING |
			  G_LOG_LEVEL_CRITICAL);
  #ifdef __SYMBIAN32__
  g_log_set_handler (NULL,  G_LOG_FLAG_FATAL| G_LOG_FLAG_RECURSION | G_LOG_LEVEL_CRITICAL | G_LOG_LEVEL_WARNING | G_LOG_LEVEL_MESSAGE | G_LOG_LEVEL_INFO | G_LOG_LEVEL_DEBUG, &mrtLogHandler, NULL);
  g_set_print_handler(mrtPrintHandler);
  #endif /*__SYMBIAN32__*/
			  
  g_type_init ();

  test_dynamic_type ();
    #ifdef __SYMBIAN32__
  testResultXml("dynamictype");
  #endif //__SYMBIAN32__

  return 0;
}
