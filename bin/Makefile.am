include $(top_srcdir)/config/utility.am

EXTRA_DIST = \
	$(initd_SCRIPTS:=.in)

initddir = @SIPX_SERVICEDIR@
initd_SCRIPTS = \
    sipxsbc

$(initd_SCRIPTS) : % : %.in Makefile
	@$(call SearchAndReplace,$<,$@)

CLEANFILES = $(initd_SCRIPTS)
