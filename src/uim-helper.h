/*
  GKrellUIM uim-helper

  Copyright (C) 2004-2021 dai <d+gkrelluim@vdr.jp>
  All rights reserved.

  Original Author: uim Project https://github.com/uim/uim
  Modifier: dai <d+gkrelluim@vdr.jp>

  Redistribution and use in source and binary forms, with or
  without modification, are permitted provided that the
  following conditions are met:

  1. Redistributions of source code must retain the above
     copyright notice, this list of conditions and the
     following disclaimer.
  2. Redistributions in binary form must reproduce the above
     copyright notice, this list of conditions and the
     following disclaimer in the documentation and/or other
     materials provided with the distribution.
  3. Neither the name of authors nor the names of its
     contributors may be used to endorse or promote products
     derived from this software without specific prior written
     permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
  CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __UIM_HELPER_H_INCLUDED__
#define __UIM_HELPER_H_INCLUDED__

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

/* GTK+ */
#include <gtk/gtk.h>

/* uim */
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <uim/uim.h>
#include <uim/uim-helper.h>
#include <uim/uim-scm.h>

enum {
  TYPE_IM=0,
  TYPE_MODE=1,
  TYPE_INPUT=2
};

/*
 * taken from uim-svn3109/helper/toolbar-common-gtk.c
 */
struct _CommandEntry {
  const gchar *desc;
  const gchar *label;
  const gchar *icon;
  const gchar *command;
  const gchar *custom_button_show_symbol;
  uim_bool show_button;
};

void uim_toolbar_check_helper_connection( GtkWidget* );

const gchar *get_command_entry_desc( gint );
const gchar *get_command_entry_command( gint );
const gchar *get_command_entry_icon( gint );
const gchar *get_command_entry_custom_button_show_symbol( gint );

void create_im_menu( GtkWidget*, GtkWidget* );
void create_mode_menu( GtkWidget*, GtkWidget* );
void create_input_menu( GtkWidget*, GtkWidget* );
void helper_init( GtkWidget* );

#endif /* __UIM_HELPER_H_INCLUDED__ */

/*
 * [EOF]
 */
