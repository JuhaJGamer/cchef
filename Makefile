CC			:= gcc
CFLAGS 		:= -Wall --std=c99
DCFLAGS		:= -g
LDFLGAS		:=

TARGETS 	:= cchef
MAINS		:= $(addsuffix .o, $(TARGETS))
OBJS		:= lexer.o $(MAINS)
DEPS 		:= lexer.h

release: $(TARGETS)

.PHONY: all clean debug release

clean:
	rm -f $(TARGETS) $(OBJS)

debug: CFLAGS += $(DCFLAGS)
debug: $(TARGETS)

$(TARGETS) : % : $(filter-out $(MAINS), $(OBJS)) %.o
	$(CC) -o $@ $(LIBS) $^ $(CFLAGS) $(LDFLAGS)




