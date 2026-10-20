noinst_LIBRARIES = libcputracing.a

AM_CPPFLAGS = -I$(top_srcdir)

libcputracing_a_SOURCES = seq.cpp
noinst_HEADERS = seq.h

libcputracing_a_SOURCES += bvh.cpp bvh2.cpp
noinst_HEADERS += bvh.h

if have_embree
libcputracing_a_SOURCES += embree.cpp
noinst_HEADERS += embree.h
endif

