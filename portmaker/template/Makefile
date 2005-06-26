datadir=$(PREFIX)/share
templatedir = $(datadir)/$(PACKAGE)-$(VERSION)
template_SOURCES = \
	mingwPORT.README \
	mingwPORT.afterconfigure \
	mingwPORT.afterinstall \
	mingwPORT.aftermake \
	mingwPORT.afterpatch \
	mingwPORT.afterquestion \
	mingwPORT.afterrpatch \
	mingwPORT.beforeconfigure \
	mingwPORT.beforeinstall \
	mingwPORT.beforemake \
	mingwPORT.beforepatch \
	mingwPORT.beforequestion \
	mingwPORT.beforerpatch \
	mingwPORT.cleanup \
	mingwPORT.configure \
	mingwPORT.exports \
	mingwPORT.ini \
	mingwPORT.install \
	mingwPORT.make \
	mingwPORT.patch \
	mingwPORT.question \
	mingwPORT.sh \
	mirrors.sf

all: stamp-template

stamp-template:
	@touch stamp-template

install: $(template_SOURCES)
	mkdir -p $(templatedir)
	cp $^ $(templatedir)

clean:
	rm -f stamp-template
