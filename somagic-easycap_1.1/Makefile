################################################################################
# Makefile                                                                     #
#                                                                              #
# Makefile for user space initialization and capture programs for              #
# Somagic EasyCAP DC60 and Somagic EasyCAP 002                                 #
# ##############################################################################
#
# Copyright 2011, 2012 Jeffry Johnston
#
# This file is part of somagic_easycap
# http://code.google.com/p/easycap-somagic-linux/
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, see <http://www.gnu.org/licenses/>.

SHELL = /bin/sh
PREFIX = /usr/local
BINDIR = $(PREFIX)/bin
MANDIR = $(PREFIX)/share/man
PROGRAMS = somagic-init somagic-capture
BETA = somagic-audio-capture somagic-both
MANUALS = man/somagic-init.1 man/somagic-capture.1
CFLAGS = -s -W -Wall
LFLAGS = -lusb-1.0 -lgcrypt

.SUFFIXES:
.SUFFIXES: .c

.PHONY: all
all: $(PROGRAMS)

.c:
	$(CC) $(CFLAGS) $< -o $@ $(LFLAGS)

.PHONY: beta
beta: $(BETA)

.PHONY: install
install: $(PROGRAMS) $(MANUALS)
	mkdir -p $(BINDIR)
	install $(PROGRAMS) $(BINDIR)/
	mkdir -p $(MANDIR)/man1
	install $(MANUALS) $(MANDIR)/man1/

.PHONY: install-beta
install-beta: $(BETA)
	mkdir -p $(BINDIR)
	install $(BETA) $(BINDIR)/

.PHONY: clean
clean:
	-rm -f $(PROGRAMS) $(BETA)
