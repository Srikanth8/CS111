CC = gcc
CFLAGS = -g -Wall -Wextra -Wno-unused -Werror -pthread -lrt
LAB = 4
DISTDIR = lab4-Srikanth

DIST_SOURCES = main.c main2.c SortedList.h SortedList.c answers.txt Lab4_Graphs.pdf Makefile

all: main.c main2.c SortedList.c
	$(CC) $(CFLAGS) -o addtest main.c
	$(CC) $(CFLAGS) -o slttest main2.c SortedList.c

clean:
	rm addtest slttest

dist: $(DISTDIR).tar.gz

$(DISTDIR).tar.gz: $(DIST_SOURCES)
	rm -fr $(DISTDIR)
	tar -czf $@.tmp --transform='s,^,$(DISTDIR)/,' $(DIST_SOURCES)
	mv $@.tmp $@
