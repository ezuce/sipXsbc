include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am

SUBDIRS = \
	.

EXTRA_DIST = \
	$(sipxsbc_SRC) \
	$(sipxsbc_RESOURCES)

jardir = @SIPX_JAVADIR@/sipXconfig/plugins
JAR_FILE = sipxsbc-config.jar
jar_DATA = $(JAR_FILE)

sipxsbc_SRC = $(shell cd $(srcdir); find src -type f -name '*.java')

sipxsbc_RESOURCES = \
	$(shell cd $(srcdir); find plugin -type f) \
	sipxsbc-plugin.properties \
	sipxplugin.beans.xml

sipxsbc_PKGS = \
	sipxcommons \
	sipxconfig-neoconf \
	sipxconfig-web \
	commons-io \
	commons-lang \
	mongo \
	tapestry-annotations \
	tapestry-framework \
	org.springframework.core \
	org.springframework.transaction \
	org.springframework.jdbc \
	org.springframework.context \
	org.springframework.beans \
	hivemind

sipxsbc_DEPS = \
	$(call JavaDep,@SIPX_JAVADIR@/sipXcommons @SIPX_JAVADIR@/sipXconfig,$(sipxsbc_PKGS))

$(JAR_FILE) : javac-sipxsbc $(sipxsbc_RESOURCES)
	jar cf $@ \
	  $(call JarInclude,$(JAVAROOT),.) \
	  $(call JarInclude,$(srcdir),$(sipxsbc_RESOURCES))

style_SRC = $(sipxsbc_SRC)

precommit : style validate;

validate : sipxplugin.beans.xml
	xmllint --noout \
	  --schema $(SIPX_DATADIR)/schema/spring-beans-2.0.xsd  \
	$^
