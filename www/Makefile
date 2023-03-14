CC = gcc
CFLAGS = -Wall -I include -g
EXE = server
SRC_DIR = src
SRCLIB_DIR = srclib
INCLUDE_DIR = include
OBJ_DIR = obj
LDFLAGS = -lconfuse ./srclib/conf.a ./srclib/fileparser.a ./srclib/ip.a ./srclib/picohttpparser.a 
MAIN_SRC = $(SRC_DIR)/main.c
SRCLIB_SRCS = $(wildcard $(SRCLIB_DIR)/*.c)
SRCLIB_OBJS = $(SRCLIB_SRCS:$(SRCLIB_DIR)/%.c=$(OBJ_DIR)/%.o)
LIBS = $(SRCLIB_DIR)/conf.a $(SRCLIB_DIR)/fileparser.a $(SRCLIB_DIR)/ip.a $(SRCLIB_DIR)/picohttpparser.a

.PHONY: all clean

all: $(EXE)

$(EXE): $(OBJ_DIR)/main.o $(OBJ_DIR)/http.o $(LIBS) 
	$(CC) $(CFLAGS) $(LDFLAGS) $^  -o $@ 

$(OBJ_DIR)/main.o: $(MAIN_SRC) $(wildcard $(INCLUDE_DIR)/*.h)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/http.o: $(SRC_DIR)/http.c $(wildcard $(INCLUDE_DIR)/*.h)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(SRCLIB_DIR)/%.c $(wildcard $(INCLUDE_DIR)/*.h)
	$(CC) $(CFLAGS) -c $< -o $@

$(SRCLIB_DIR)/conf.a: $(OBJ_DIR)/conf.o
	ar rcs $@ $<
	
$(SRCLIB_DIR)/fileparser.a: $(OBJ_DIR)/fileparser.o
	ar rcs $@ $<

$(SRCLIB_DIR)/picohttpparser.a: $(OBJ_DIR)/picohttpparser.o
	ar rcs $@ $<

$(SRCLIB_DIR)/ip.a: $(OBJ_DIR)/ip.o
	ar rcs $@ $<
	
	
	

clean:
	rm -f $(OBJ_DIR)/*.o $(SRCLIB_DIR)/*.a app
