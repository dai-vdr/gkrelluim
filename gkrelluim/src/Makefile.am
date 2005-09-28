plugin_PROGRAMS = gkrelluim.so

plugindir = $(libdir)/gkrellm2/plugins/

gkrelluim_so_SOURCES = gkrelluim.c uim-helper.c
AM_CFLAGS = -fPIC @GKRELLM_CFLAGS@
AM_LDFLAGS = -Wl -shared @GKRELLM_LIBS@ @UIM_LIBS@
