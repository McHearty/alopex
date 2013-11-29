
PROG     =  alopex
VER      =  3.0a
CC       ?= gcc
CFLAGS   += `pkg-config --cflags x11 cairo freetype2`
LDFLAGS  += `pkg-config --libs x11 cairo freetype2`
PREFIX   ?= /usr
MODULES  =  alopex draw input key_chain tile
HEADERS  =  alopex.h config.h
MANPAGES =  alopex.1
VPATH    =  src:doc

${PROG}: ${MODULES:%=%.o}
	@cd src && ${CC} -o ../${PROG} ${MODULES:%=%.o} ${LDFLAGS}

%.o: %.c ${HEADERS}
	@${CC} -c -o src/$@ $< ${CFLAGS} ${OPTS}

install: ${PROG}
	@echo Not yet

${MANPAGES}: alopex.%: alopex-%.tex
	@latex2man $< $@

man: ${MANPAGES}

clean:
	@rm -f ${PROG} ${PROG}-${VER}.tar.gz
	@cd src && rm -f ${MODULES:%=%.o}

dist: clean
	@tar -czf ${PROG}-${VER}.tar.gz *

.PHONY: clean dist man
