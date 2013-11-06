CC = gcc

LIB_DIR = lib
BIN_DIR = bin
INC_DIR = include

CFLAGS = -Iinclude/ -g -Wall -Wextra -Werror -Wno-unused-result -Wno-missing-field-initializers -Wno-unused-parameter -Wno-unused-but-set-parameter -Wno-unused-function -std=gnu99 -D_GNU_SOURCE -DCS_DEBUG 
LDFLAGS = -L$(LIB_DIR)/

CS_DIR = caesium
CS_SRCS = $(wildcard caesium/*.c)
CS_OBJS = $(CS_SRCS:.c=.o)
CS_LIBS = -lxxhash -lpthread -lm

TINYC_DIR = $(LIB_DIR)/tinycthread
TINYC_SRCS = $(wildcard $(TINYC_DIR)/*.c)
TINYC_OBJS = $(TINYC_SRCS:.c=.o)

XXHASH_DIR = $(LIB_DIR)/xxhash
XXHASH_SRCS = $(wildcard $(XXHASH_DIR)/*.c)
XXHASH_OBJS = $(XXHASH_SRCS:.c=.o)


all: caesium

caesium: xxhash $(CS_OBJS)
	$(CC) $(LDFLAGS) -o $(BIN_DIR)/caesium $(CS_OBJS) $(CS_LIBS)

xxhash: $(LIB_DIR)/libxxhash.a
$(LIB_DIR)/libxxhash.a: $(XXHASH_OBJS)
	$(AR) rcsv $(LIB_DIR)/libxxhash.a $(XXHASH_OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(BIN_DIR)/* $(LIB_DIR)/*.a *.o
	rm -f $(addprefix $(CS_DIR)/,*.o *.gc*)
	rm -f $(addprefix $(XXHASH_DIR)/,*.o *.gc*)

# Compile with code coverage on
cover: CFLAGS += -fprofile-arcs -ftest-coverage
cover: CS_LIBS += -lgcov -coverage
cover: caesium;