SHELL		= /bin/sh

TARGET		= libuthread.so

OFILES		= uthread.o \
			  uthread_ctx.o uthread_queue.o uthread_mtx.o \
			  uthread_cond.o uthread_sched.o

# user executables, test code... wowza
EXECS		= test.o

CC			= gcc

CFLAGS		= -g -Wall -fPIC -D_GNU_SOURCE
IFLAGS		=
LFLAGS		= -L. -Wl,--rpath,.

LIBS		= -ldl

.PHONY: all cscope clean handin

all: cscope $(TARGET) $(EXECS)
	@for exec in $(EXECS); do \
		$(CC) $(CFLAGS) $(LFLAGS) -o `basename $$exec .o` $$exec -luthread; \
	done \

cscope:
	@find -name "*.[chS]" > cscope.files
	cscope -k -b -q -v

$(TARGET): $(OFILES)
	$(CC) -g -shared $(LFLAGS) -o $(TARGET) $(OFILES) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) $(IFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) *.o
	rm -f cscope.files cscope.out cscope.in.out cscope.po.out
	for exec in $(EXECS) ; do \
		if [ -f `basename $$exec .o` ] ; then \
			rm `basename $$exec .o` ; \
		fi \
	done

# This is magic for implementing make nyi - basically, the grep command prepends
# the file and line number, so a line looks like
#
# foo.c:3:		Function_you_need_to_implement("PROJECT: bar")
#
# The sed command finds the relevant parts and separates them to be printed by awk
SED_REGEX := 's/^\(.*:.*:\).*"\(.*\):\(.*\)".*/\1 \2 \3/'

PROJ_FILTER := $(foreach def,$(COMPILE_CONFIG_BOOLS), \
	$(if $(findstring 0,$($(def))),grep -v $(def) |,))

FILTER := grep -v define | $(PROJ_FILTER) \
	sed -e $(SED_REGEX) | awk '{printf("%30s %30s() %10s\n", $$1, $$3, $$2)}'

nyi:
	@find . -name "*.c" | xargs grep -n Function_you_need_to_implement | $(FILTER)
