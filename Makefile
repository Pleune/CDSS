ROOT=./

CFLAGS := -g -O3 -Wall
LFLAGS := -g

SNAME = libpleune.a
DNAME = libpleune.so

SRCDIR=$(ROOT)src/
TESTDIR=$(ROOT)test/
BUILDS=$(ROOT)build/static/
BUILDD=$(ROOT)build/dynamic/
BUILDTEST=$(ROOT)build/test/
OUTPUTDIR=$(ROOT)bin/

SRC:=$(wildcard $(SRCDIR)*.c)

OBJSS=$(patsubst $(SRCDIR)%.c,$(BUILDS)%.o,$(SRC))
OBJSD=$(patsubst $(SRCDIR)%.c,$(BUILDD)%.o,$(SRC))

SRC_TEST:=$(wildcard $(TESTDIR)*.c)
OBJSTEST=$(patsubst $(TESTDIR)%.c,$(BUILDTEST)%.o,$(SRC_TEST))

.PHONY: clean install uninstall test

all:	$(OUTPUTDIR)$(SNAME) $(OUTPUTDIR)$(DNAME) $(OUTPUTDIR)libpleune_tester

-include $(OBJSS:.o=.d)
-include $(OBJSD:.o=.d)
-include $(OBJSTEST:.o=.d)

$(OUTPUTDIR)$(SNAME): $(OBJSS)
	$(AR) $(ARFLAGS) $@ $^

$(OUTPUTDIR)$(DNAME): CFLAGS += -fPIC
$(OUTPUTDIR)$(DNAME): LFLAGS += -shared
$(OUTPUTDIR)$(DNAME): $(OBJSD)
	$(CC) $(LFLAGS) $^ -o $@

$(BUILDS)%.d: $(SRCDIR)%.c
	$(CC) $(CFLAGS) $< -MM -MT $(@:.d=.o) >$@

$(BUILDD)%.d: $(SRCDIR)%.c
	$(CC) $(CFLAGS) $< -MM -MT $(@:.d=.o) >$@

$(BUILDTEST)%.d: $(TESTDIR)%.c
	$(CC) $< -I$(SRCDIR) -MM -MT $(@:.d=.o) >$@

$(BUILDS)%.o: $(SRCDIR)%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDD)%.o: $(SRCDIR)%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDTEST)%.o: $(TESTDIR)%.c
	$(CC) $(CFLAGS) -I$(SRCDIR) -c $< -o $@

$(OUTPUTDIR)libpleune_tester: $(OBJSTEST) $(OUTPUTDIR)$(SNAME)
	$(CC) $(LFLAGS) -L$(OUTPUTDIR) $^ -lm $(OUTPUTDIR)libpleune.a -pthread -o $@

test: $(OUTPUTDIR)libpleune_tester $(OUTPUTDIR)$(SNAME) $(OUTPUTDIR)$(DNAME)
	LD_LIBRARY_PATH=$(OUTPUTDIR):$LD_LIBRARY_PATH $(OUTPUTDIR)/libpleune_tester
	grep Finished test.log

install: $(OUTPUTDIR)$(SNAME) $(OUTPUTDIR)$(DNAME)
	mkdir -p /usr/local/lib
	mkdir -p /usr/local/include
	install $(OUTPUTDIR)$(SNAME) /usr/local/lib/
	install -s $(OUTPUTDIR)$(DNAME) /usr/local/lib/
	install $(SRCDIR)cdss.h /usr/local/include/

uninstall:
	rm -f /usr/local/lib/$(SNAME)
	rm -f /usr/local/lib/$(DNAME)
	rm -f /usr/local/include/cdss.h

clean:
	rm -rf $(BUILDS)* $(BUILDD)* $(BUILDTEST)* $(OUTPUTDIR)*
