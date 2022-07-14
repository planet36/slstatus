# slstatus version
VERSION = 0
VERSION := $(shell git describe --tags)-planet36

# Customize below to fit your system

# paths
PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin
MANDIR ?= $(PREFIX)/share/man

# includes and libs
INCS = `pkg-config --cflags x11`
LIBS = `pkg-config --libs   x11`

# flags
CPPFLAGS += -DVERSION=\"$(VERSION)\" -D_DEFAULT_SOURCE $(INCS)
DEPFLAGS += -MMD -MP
CFLAGS += -O2 \
		-Wno-format-nonliteral
LDLIBS += $(LIBS)
# OpenBSD: add -lsndio
# FreeBSD: add -lkvm -lsndio
