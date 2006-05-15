##
## Process this file with automake to produce Makefile.in
##

libdir = $(PLUGIN_DIR)


## Conditionals

if HAVE_ESD
esd_ltlibs = audio_esd.la
esd_ldflags = -avoid-version -module
esd_sources = audio_esd.c
else
esd_ltlibs =
esd_ldflags =
esd_sources =
endif


## Libs
lib_LTLIBRARIES = audio_sys.la audio_null.la audio_wav.la $(esd_ltlibs)

audio_sys_la_SOURCES = audio_sys.c
audio_sys_la_LDFLAGS = -rpath $(libdir) -avoid-version -module

audio_null_la_SOURCES = audio_null.c
audio_null_la_LDFLAGS = -avoid-version -module

audio_wav_la_SOURCES = audio_wav.c
audio_wav_la_LDFLAGS = -avoid-version -module

audio_esd_la_SOURCES = $(esd_sources)
audio_esd_la_LDFLAGS = $(esd_ldflags)
