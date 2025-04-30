# slstatus version
VERSION = 1.1

VERSION := $(shell git describe --tags --abbrev=0)-planet36

# Customize below to fit your system

# paths
PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin
MANDIR ?= $(PREFIX)/share/man

# includes and libs
#INCS = `pkg-config --cflags x11`
LIBS = `pkg-config --libs   x11`

# flags
CPPFLAGS = -MMD -MP
CPPFLAGS += -DVERSION=\"$(VERSION)\" -D_DEFAULT_SOURCE $(INCS)

CFLAGS = -std=c23
CFLAGS += -pipe -Wall -Wextra -Wpedantic -Wfatal-errors
CFLAGS += -O3 -flto=auto -march=native -fno-math-errno

LDLIBS = $(LIBS)
# OpenBSD: add -lsndio
# FreeBSD: add -lkvm -lsndio
