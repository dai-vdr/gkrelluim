plugin_PROGRAMS = gkrelluim.so

plugindir = $(libdir)/gkrellm2/plugins/


gkrelluim_so_SOURCES = gkrelluim.c uim-helper.c gkrelluim.h
gkrelluim_so_LDADD = $(GKRELLM_LIBS) $(UIM_LIBS)
AM_CFLAGS = -fPIC @GKRELLM_CFLAGS@ -DUIM_PIXMAPSDIR="\"$(datadir)/uim/pixmaps/\""
AM_LDFLAGS = -shared
