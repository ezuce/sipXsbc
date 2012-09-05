include $(top_srcdir)/config/utility.am

INCLUDES = \
	-I$(top_srcdir)/include

bin_PROGRAMS = sipxsbc

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

sipxsbc_CXXFLAGS = \
	-DOSSLIBS_TMPDIR=\"@SIPX_TMPDIR@\" \
	-DOSSLIBS_CONFDIR=\"@SIPX_CONFDIR@\" \
	-DOSSLIBS_RUNDIR=\"@SIPX_RUNDIR@\" \
	-DOSSLIBS_LOGDIR=\"@SIPX_LOGDIR@\" \
	-DOSSAPP_LIB_VERSION=${version_Current}.${version_Revision}.${version_Age}
	-fno-gcse -O0 -fno-implement-inlines -fno-default-inline \
	-I$(top_srcdir)/include

sipxsbc_SOURCES =  \
	main.cpp \
	SBCDataStore.cpp \
	SBController.cpp

sipxsbc_LDADD = \
	@OSSCORE_LIBS@ \
	${common_libs}

sipxsbc_LDFLAGS = \
    -rdynamic
	


