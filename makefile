# SO-051
# Ivo Veiga, nº 44865
# Joao Silva, nº 48782
# Manuel Tovar, nº 49522

CC = gcc
SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj
TEST_DIR = tests
_OBJETOS = main.o cliente.o control.o empresa.o prodcons.o time.o intermediario.o scheduler.o file.o memory.o so.o
OBJETOS = $(patsubst %,$(OBJ_DIR)/%,$(_OBJETOS))

CFLAGS = -Wall -I $(INC_DIR)
MAIN = $(INC_DIR)/main.h
CLIENT = $(INC_DIR)/cliente.h
MEMORY = $(INC_DIR)/memory.h
PRODCONS = $(INC_DIR)/prodcons.h
CONTROL = $(INC_DIR)/control.h
FILE = $(INC_DIR)/file.h
TIME = $(INC_DIR)/sotime.h
SO = $(INC_DIR)/so.h
SCHEDULER = $(INC_DIR)/scheduler.h
EMPRESA = $(INC_DIR)/empresa.h
INTERMEDIARIO = $(INC_DIR)/intermediario.h

# EXECUTAVEL
soadmpor: $(OBJETOS)
	$(CC) $(OBJETOS) -o bin/soadmpor -lrt -lpthread

# MAIN
$(OBJ_DIR)/main.o: $(SRC_DIR)/main.c $(MAIN) $(CLIENT) $(MEMORY) $(PRODCONS) $(CONTROL) $(FILE) $(TIME) $(SO)

# CLIENTE
$(OBJ_DIR)/cliente.o: $(SRC_DIR)/cliente.c $(MAIN) $(MEMORY) $(TIME) $(CLIENT) $(FILE)

#CONTROL
$(OBJ_DIR)/control.o: $(SRC_DIR)/control.c $(MAIN) $(SO) $(MEMORY) $(PRODCONS) $(CONTROL)

#SCHEDULER
$(OBJ_DIR)/scheduler.o: $(SRC_DIR)/scheduler.c $(MAIN) $(SO) $(SCHEDULER)

#PRODCONS
$(OBJ_DIR)/prodcons.o: $(SRC_DIR)/prodcons.c $(MAIN) $(SO) $(CONTROL) $(PRODCONS)

#TIME
$(OBJ_DIR)/time.o: $(SRC_DIR)/time.c $(MAIN) $(SO) $(TIME)

# INTERMEDIARIO
$(OBJ_DIR)/intermediario.o: $(SRC_DIR)/intermediario.c $(MAIN) $(MEMORY)

# EMPRESA
$(OBJ_DIR)/empresa.o: $(SRC_DIR)/empresa.c $(EMPRESA) $(MAIN) $(MEMORY) $(TIME)

# FILE
$(OBJ_DIR)/file.o: $(SRC_DIR)/file.c $(MAIN) $(SO) $(MEMORY) $(SCHEDULER) $(FILE) $(PODCONS) $(TIME)

# MEMORY
$(OBJ_DIR)/memory.o: $(SRC_DIR)/memory.c $(MAIN) $(SO) $(MEMORY) $(PODCONS) $(CONTROL) $(SCHEDULER) $(FILE) $(TIME)

#ALL
$(OBJ_DIR)/%.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f bin/soadmpor
	find ./$(OBJ_DIR) -type f ! -name 'so.o' -delete

testAll:
	./bin/soadmpor ./tests/in/scenario1 ./tests/out/scenario1_stud -l ./tests/log/scenario1.log -t 1000
	./bin/soadmpor ./tests/in/scenario2 ./tests/out/scenario2_stud -l ./tests/log/scenario2.log -t 1000
	./bin/soadmpor ./tests/in/scenario3 ./tests/out/scenario3_stud -l ./tests/log/scenario3.log -t 1000
	./bin/soadmpor ./tests/in/scenario4 ./tests/out/scenario4_stud -l ./tests/log/scenario4.log -t 1000
