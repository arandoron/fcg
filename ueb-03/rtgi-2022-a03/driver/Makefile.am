bin_PROGRAMS = rtgi

AM_CPPFLAGS = -I$(top_srcdir)
rtgi_SOURCES = main.cpp cmdline.cpp interaction.cpp
noinst_HEADERS = interaction.h cmdline.h

if have_gl
rtgi_SOURCES += preview.cpp
noinst_HEADERS += preview.h
endif

rtgi_LDADD  = ../gi/libprimary-hit.a ../rt/cpu/libcputracing.a

if have_gl
rtgi_LDADD += ../libgi/gl/libgl.a
endif
rtgi_LDADD += ../libgi/libgi.a
rtgi_LDADD += $(WAND_LIBS)

