## Process this file with automake to produce Makefile.in
include $(top_srcdir)/config/subdir.am

SUBDIRS = .

INCLUDES = \
	-I$(top_srcdir)/include

lib_LTLIBRARIES = libsipxsbc.la

## Libtool version info - update only immediately before a release
# Current - increment if any interfaces are added, removed, or changed
version_Current=4
# Revision - set to 0 if Current was incremented
#            increment if any implementation has changed
version_Revision=6
# Age - set to 0 of any interfaces have been removed
#       increment if any interfaces have been added
version_Age=0

common_libs = \
	-lboost_date_time-mt \
	-lboost_filesystem-mt \
	-lboost_system-mt \
	-lboost_thread-mt \
	-lboost_regex-mt \
	-lboost_program_options-mt \
	-lboost_iostreams-mt \
	-lPocoFoundation \
	-lPocoUtil \
	-lPocoNet \
	-lPocoXML \
	-lhiredis \
	-lconfig++ \
	-lv8 \
	-lltdl \
	-lmcrypt \
	-lssl \
	-lcrypto \
	-lpthread \
	-ldl \
	-lrt \
	-lcrypt \
	-lssl \
	-lresolv

common_cxx_flags = \
	-DOSSLIBS_TMPDIR=\"@OSSLIBS_TMPDIR@\" \
	-DOSSLIBS_CONFDIR=\"@OSSLIBS_CONFDIR@\" \
	-DOSSLIBS_RUNDIR=\"@OSSLIBS_RUNDIR@\" \
	-DOSSLIBS_LOGDIR=\"@OSSLIBS_LOGDIR@\" \
	-DOSSAPP_LIB_VERSION=${version_Current}.${version_Revision}.${version_Age}

libsipxsbc_la_LDFLAGS = \
	-version-info ${version_Current}:${version_Revision}:${version_Age}


libsipxsbc_la_LIBADD = ${common_libs}
	


# this is helpful if you're debugging something with lots of inlines (like the UtlLink code)
# 		-fno-gcse -O0 -fno-implement-inlines -fno-default-inline
libsipxsbc_la_CXXFLAGS = \
	${common_cxx_flags} \
	-fno-gcse -O0 -fno-implement-inlines -fno-default-inline

#libsipxsbc_la_CFLAGS = 	-D_BSD_SOURCE -std=gnu99 -Wno-uninitialized -g -O2 -MT


libsipxsbc_la_SOURCES =  \
	SBCDataStore.cpp \
	SBController.cpp


bin_PROGRAMS = sipXsbc


sipXsbc_CXXFLAGS = \
	${common_cxx_flags} \
	-fno-gcse -O0 -fno-implement-inlines -fno-default-inline

sipXsbc_LDFLAGS = \
	-rdynamic

sipXsbc_LDADD = \
	@OSSLIBSCORE_LIBS@ \
	libsipxsbc.la \
	${common_libs}

sipXsbc_SOURCES = main.cpp
	


