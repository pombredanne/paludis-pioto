/* $Id: generate_defs_gtk.cc,v 1.22 2006/05/16 18:03:21 murrayc Exp $ */

/* generate_defs_gtk.cc
 *
 * Copyright (C) 2001 The Free Software Foundation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "glibmm_generate_extra_defs/generate_extra_defs.h"
#include <glib-object.h>
#include <vte/vte.h>
#include <vte/reaper.h>

int main (int argc, char *argv[])
{
  g_type_init();

std::cout << get_defs(VTE_TYPE_TERMINAL)
	  << get_defs(VTE_TYPE_REAPER);

  return 0;
}