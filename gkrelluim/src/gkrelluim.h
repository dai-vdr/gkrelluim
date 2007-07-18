/*
  GKrellUIM: GKrellM UIM helper Plugin

  Copyright (C) 2004-2007 dai <d+gkrelluim@vdr.jp>
  All rights reserved.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
*/

#ifndef __GKRELLUIM_H_INCLUDED__
#define __GKRELLUIM_H_INCLUDED__

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

/* GKrellM */
#define PACKAGE        "gkrelluim"
#define CONFIG_KEYWORD PACKAGE
#define STYLE_NAME     PACKAGE
#include <gkrellm2/gkrellm.h>

/* uim */
#include <uim/uim.h>
#define OBJECT_DATA_IM_BUTTON "IM_BUTTON"

extern int uim_fd;
extern uint command_entry_len;

extern gchar *mode_text;
extern gchar *input_text;

#endif /* __GKRELLUIM_H_INCLUDED__ */
