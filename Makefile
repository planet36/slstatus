# slstatus - suckless status monitor
# See LICENSE file for copyright and license details.

include config.mk

SRCS = $(wildcard *.c components/*.c)
DEPS = $(SRCS:.c=.d)
OBJS = $(SRCS:.c=.o)

BIN = slstatus

$(BIN): $(OBJS)
	$(CC) $^ -o $@ $(LDLIBS)

%.o: %.c
	$(CC) -o $@ $(CFLAGS) -c $<

$(OBJS): config.h config.mk

config.h:
	cp config.def.h config.h

options:
	@echo $(BIN) build options:
	@echo "CFLAGS  = $(CFLAGS)"
	@echo "LDLIBS  = $(LDLIBS)"
	@echo "CC      = $(CC)"

clean:
	@$(RM) --verbose -- $(BIN) $(OBJS) $(DEPS) $(BIN)-$(VERSION).tar.xz

dist:
	git archive --prefix $(BIN)-$(VERSION)/ HEAD | xz > $(BIN)-$(VERSION).tar.xz

install: $(BIN)
	mkdir -p "$(DESTDIR)$(BINDIR)"
	cp -f $(BIN) "$(DESTDIR)$(BINDIR)"
	chmod 755 "$(DESTDIR)$(BINDIR)/$(BIN)"
	mkdir -p "$(DESTDIR)$(MANDIR)/man1"
	sed "s/VERSION/$(VERSION)/g" < $(BIN).1 > "$(DESTDIR)$(MANDIR)/man1/$(BIN).1"
	chmod 644 "$(DESTDIR)$(MANDIR)/man1/$(BIN).1"

uninstall:
	@$(RM) "$(DESTDIR)$(BINDIR)/$(BIN)" \
		"$(DESTDIR)$(MANDIR)/man1/$(BIN).1"

lint:
	clang-tidy --quiet $(SRCS) -- $(CFLAGS) $(LDLIBS)

.PHONY: options clean dist install uninstall lint

-include $(DEPS)
