CC = gcc
CFLAGS = -g -Wall -Wextra -Wno-unused -Werror
LAB = 1
DISTDIR = lab1-Srikanth

DIST_SOURCES = main.c Makefile test.sh README

all: main.c
	$(CC) $(CFLAGS) -o simpsh main.c

check:
	./test.sh

clean:
	rm simpsh

dist: $(DISTDIR).tar.gz

$(DISTDIR).tar.gz: $(DIST_SOURCES)
	rm -fr $(DISTDIR)
	tar -czf $@.tmp --transform='s,^,$(DISTDIR)/,' $(DIST_SOURCES)
	mv $@.tmp $@
