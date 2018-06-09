shell = /bin/bash
OBJ_DIR = obj
SRC_DIR = src
INC_DIR = inc
CC = gcc

# Liste des .c a compiler qu'on transforme en .o
L_OBJ = $(shell ls $(SRC_DIR) | grep -e .c$$ | sed 's/\.c$$/.o/')
# On ajoute le obj/ devant les noms des .o
OBJ = $(patsubst %,$(OBJ_DIR)/%,$(L_OBJ))

LIBS = $(shell pkg-config libxml-2.0 --libs)

CFLAGS = -I$(INC_DIR) -Wextra -Wall -Wundef -Wfloat-equal -Wshadow -Wpointer-arith -Wcast-align -Wstrict-prototypes -Wstrict-overflow=5
CFLAGS += -O -pedantic -std=c99
CFLAGS += -Wswitch-default -Wswitch-enum
CFLAGS += -Wunreachable-code -Wconversion -Wcast-qual
CFLAGS += $(shell pkg-config libxml-2.0 --cflags)

BIN = dicalua

all : $(BIN)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@echo cc -c $< -o $@
	@$(CC) -c -g -o $@ $< $(CFLAGS)

$(BIN): $(OBJ)
	@echo cc -o $@ $^
	@$(CC) -o $@ $^ $(LIBS)

clean :
	$(RM) $(OBJ)

distclean: clean
	$(RM) $(BIN)

fast: clean $(BIN)

print-%:
	@echo $* = $($(*))


# $@ select the argument before the two dots
# $^ select all arguments after the two dots
# $< select the first argument after the two dots
