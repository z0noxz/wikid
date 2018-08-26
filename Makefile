.POSIX:

include config.mk

SRC = util.c term.c wikid.c
OBJ = ${SRC:.c=.o}

all: options wikid

options:
	@echo wikid build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

.c.o:
	@echo CC $<
	@${CC} ${CFLAGS} -c $<

wikid.o: util.h term.h config.h

${OBJ}: config.h config.mk

config.h:
	@echo creating $@ from config.def.h
	@cp config.def.h $@

wikid: ${OBJ}
	@echo CC -o $@
	@${CC} -g -o $@ ${OBJ} ${LDFLAGS}

clean:
	@echo cleaning
	@rm -f wikid ${OBJ}
	@rm -f config.h

install: wikid
	mkdir -p /usr/local/bin
	cp -f wikid /usr/local/bin
	chmod 755 /usr/local/bin/wikid

uninstall:
	rm -f /usr/local/bin/wikid

.PHONY: all options clean install uninstall
