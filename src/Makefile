include ../common.mk
INC    = ../include
SRC    = ../src
OBJ    = ../obj
BIN    = ../bin
CC     = gcc $(UNIXCCOPS) -I$(INC) -I../../CMlib/include -I../../MFlib/include
LINK   = gcc
LIBS   = -L../../CMlib/lib -lCM10 $(UNIXLIBS)

TARGET = $(BIN)/routing.bin

INCLUDES = $(INC)/ncr.h

OBJS = $(OBJ)/NCRmain.o\
       $(OBJ)/NCRnetwork.o\
       $(OBJ)/NCRnci.o\
       $(OBJ)/NCRinput.o\
       $(OBJ)/NCRoutput.o\
       $(OBJ)/NCRrouting.o\
       $(OBJ)/NCRdistance.o

all: ${TARGET}

clean:
	rm $(TARGET) $(OBJS)

$(TARGET): $(OBJS) $(SRC)/Makefile
	$(LINK) $(OBJS) $(LIBS) -o $(TARGET)

$(OBJ)/%.o: $(SRC)/%.c $(INCLUDES)
	$(CC) -o $(OBJ)/$*.o -c $(SRC)/$*.c
