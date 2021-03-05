# slstatus version
VERSION = 0
VERSION := $(VERSION)-sdw

# Customize below to fit your system

# paths
PREFIX = /usr/local
MANPREFIX = $(PREFIX)/share/man

# includes and libs
INCS = `pkg-config --cflags x11`
LIBS = `pkg-config --libs   x11`

# flags
CPPFLAGS += -DVERSION=\"$(VERSION)\"
DEPFLAGS += -MMD -MP
CFLAGS += -Os \
		-Wno-format-nonliteral
CFLAGS += $(INCS) $(CPPFLAGS) $(DEPFLAGS)
LDFLAGS += $(LIBS)
