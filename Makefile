CC			:= gcc
CFLAGS 	:= -Wall --std=c99
DCFLAGS		:= -g
LDFLGAS		:=

TARGETS 	:= cchef
MAINS		:= $(addsuffix .o, $(TARGETS))
OBJS		:= lexer.o $(MAINS)
DEPS 		:= lexer.h

all: $(TARGETS)

.PHONY: all clean debug

clean:
	rm -f $(TARGETS) $(OBJS)

debug: CCFLAGS += $(DCFLAGS)
debug: $(TARGETS)

$(TARGETS) : % : $(filter-out $(MAINS), $(OBJS)) %.o
	$(CC) -o $@ $(LIBS) $^ $(CCFLAGS) $(LDFLAGS)




