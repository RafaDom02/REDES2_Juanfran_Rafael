CC = gcc
CFLAGS = -Wall -I include -g
EXE = server
SRC_DIR = src
SRCLIB_DIR = srclib
LIB_DIR = lib
INCLUDE_DIR = include
OBJ_DIR = obj
LDFLAGS = -lconfuse $(LIB_DIR)/conf.a $(LIB_DIR)/fileparser.a $(LIB_DIR)/ip.a $(LIB_DIR)/picohttpparser.a 
MAIN_SRC = $(SRC_DIR)/main.c
SRCLIB_SRCS = $(wildcard $(SRCLIB_DIR)/*.c)
SRCLIB_OBJS = $(SRCLIB_SRCS:$(SRCLIB_DIR)/%.c=$(OBJ_DIR)/%.o)
LIBS = $(LIB_DIR)/conf.a $(LIB_DIR)/fileparser.a $(LIB_DIR)/ip.a $(LIB_DIR)/picohttpparser.a

.PHONY: all clean

all: clean objdir libdir $(EXE)

objdir:
	mkdir -p obj

libdir:
	mkdir -p lib

$(EXE): $(OBJ_DIR)/main.o $(OBJ_DIR)/http.o $(LIBS) 
	$(CC) $(CFLAGS) $(LDFLAGS) $^  -o $@ 

$(OBJ_DIR)/main.o: $(MAIN_SRC) $(wildcard $(INCLUDE_DIR)/*.h)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/http.o: $(SRC_DIR)/http.c $(wildcard $(INCLUDE_DIR)/*.h)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(SRCLIB_DIR)/%.c $(wildcard $(INCLUDE_DIR)/*.h)
	$(CC) $(CFLAGS) -c $< -o $@

$(LIB_DIR)/conf.a: $(OBJ_DIR)/conf.o
	ar rcs $@ $<
	
$(LIB_DIR)/fileparser.a: $(OBJ_DIR)/fileparser.o
	ar rcs $@ $<

$(LIB_DIR)/picohttpparser.a: $(OBJ_DIR)/picohttpparser.o
	ar rcs $@ $<

$(LIB_DIR)/ip.a: $(OBJ_DIR)/ip.o
	ar rcs $@ $<
	
	
	

clean:
	rm -f $(OBJ_DIR)/*.o $(LIB_DIR)/*.a app
