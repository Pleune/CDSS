ROOT=./

CFLAGS := -g -O3 -Wall
LFLAGS := -g -pthread
SNAME = libpleune.a
DNAME = libpleune.so

SRCDIR=$(ROOT)src/
BUILDS=$(ROOT)build/static/
BUILDD=$(ROOT)build/dynamic/
OUTPUTDIR=$(ROOT)bin/

SRC:=$(wildcard $(SRCDIR)*.c)

OBJSS=$(patsubst $(SRCDIR)%.c,$(BUILDS)%.o,$(SRC))
OBJSD=$(patsubst $(SRCDIR)%.c,$(BUILDD)%.o,$(SRC))

.PHONY: clean install uninstall

all:	$(SNAME) $(DNAME)

-include $(OBJSS:.o=.d)
-include $(OBJSD:.o=.d)

$(SNAME): $(OBJSS)
	$(AR) $(ARFLAGS) $(OUTPUTDIR)$@ $^

$(DNAME): CFLAGS += -fPIC
$(DNAME): LFLAGS += -shared
$(DNAME): $(OBJSD)
	$(CC) $(LFLAGS) $^ -o $(OUTPUTDIR)$@

$(BUILDS)%.d:	$(SRCDIR)%.c
	$(CC) $(CFLAGS) $< -MM -MT $(@:.d=.o) >$@

$(BUILDD)%.d:	$(SRCDIR)%.c
	$(CC) $(CFLAGS) $< -MM -MT $(@:.d=.o) >$@

$(BUILDS)%.o:	$(SRCDIR)%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDD)%.o:	$(SRCDIR)%.c
	$(CC) $(CFLAGS) -c $< -o $@

install: $(SNAME) $(DNAME)
	mkdir -p /usr/local/lib
	mkdir -p /usr/local/include
	install -s $(OUTPUTDIR)$(SNAME) /usr/local/lib/
	install -s $(OUTPUTDIR)$(DNAME) /usr/local/lib/
	install $(SRCDIR)pleune.h /usr/local/include/

uninstall:
	rm -f /usr/local/lib/$(SNAME)
	rm -f /usr/local/lib/$(DNAME)
	rm -f /usr/local/include/pleune.h

clean:
	rm -rf $(BUILDS)* $(BUILDD)* $(OUTPUTDIR)*
