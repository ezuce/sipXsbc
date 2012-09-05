include $(top_srcdir)/config/utility.am

cfinputsdir = $(SIPX_CFINPUTS)/plugin.d
dist_cfinputs_DATA = \
	sipxsbc.cf

confdir = $(SIPX_CONFDIR)/sipxsbc
dist_conf_DATA = \
	sipxsbc.properties \
	sipxsbc.xml

cfgglobalsdir = $(SIPX_CONFDIR)/sipxsbc/globals
dist_cfgglobals_DATA = $(shell cd $(srcdir); find sipXsbc/globals -type f)

cfghandlersdir = $(SIPX_CONFDIR)/sipxsbc/handlers
dist_cfghandlers_DATA = $(shell cd $(srcdir); find sipXsbc/handlers -type f)

cfghandlersrrdir = $(SIPX_CONFDIR)/sipxsbc/handlers/routeRequest.js.detail
dist_cfghandlersrr_DATA = $(shell cd $(srcdir); find sipXsbc/handlers/routeRequest.js.detail -type f)

