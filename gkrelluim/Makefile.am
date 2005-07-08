plugin_PROGRAMS = gkrelluim.so

plugindir = $(libdir)/gkrellm2/plugins/

gkrelluim_so_SOURCES = gkrelluim.c
gkrelluim_so_CFLAGS = -fPIC @GKRELLM_CFLAGS@
gkrelluim_so_LDFLAGS = -Wl -shared @GKRELLM_LIBS@ @UIM_LIBS@
