CFLAGS = -g -Wall -MMD -std=gnu99
LDFLAGS=

CC     = gcc
CCLD   = gcc
RM     = rm -rf
AR     = ar

MAKEFLAGS+="-R --no-print-directory"

ifndef V
	QUIET_CC   = @ echo '    ' CC $@;
	QUIET_LD   = @ echo '    ' LD $@;
	QUIET_AR   = @ echo '    ' AR $@;
	QUIET_CCLD = @ echo '    ' CCLD $@;
endif

.PHONY: all
all : maze-test


maze-test : gen_maze.o fastest_path.o
maze-test : LDFLAGS += -lncurses

.PHONY: clean
clean :
	$(RM) *.[oda]

.SECONDARY:
%.o : %.c
	$(QUIET_CC)$(CC) $(CFLAGS) -c -o $@ $<

%.so  :
	$(QUIET_CCLD)$(CCLD) $(CFLAGS) $(LDFLAGS) -shared -o $@ $^

%.a   :
	$(QUIET_AR)$(AR) rcsD $@ $^

% :
	$(QUIET_LD)$(CCLD) $(CFLAGS) $(LDFLAGS) -o $@ $^

-include $(wildcard *.d)
