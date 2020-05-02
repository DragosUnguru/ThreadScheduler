CC = gcc
CFLAGS = -fPIC -Wall -g
LDFLAGS = -g
OBJ_DIR = obj
SRC_DIR = src
MKDIR = mkdir -p

.PHONY: build
build: directories libscheduler.so

.PHONY: directories
directories: ${OBJ_DIR}

${OBJ_DIR}:
	${MKDIR} ${OBJ_DIR}

libscheduler.so: $(OBJ_DIR)/so_scheduler.o $(OBJ_DIR)/priq.o $(OBJ_DIR)/utils.o
	$(CC) $(LDFLAGS) -shared -o $@ $^ -lpthread -lm

$(OBJ_DIR)/so_scheduler.o: $(SRC_DIR)/so_scheduler.c $(SRC_DIR)/so_scheduler.h
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ_DIR)/priq.o: $(SRC_DIR)/priq.c $(SRC_DIR)/priq.h
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ_DIR)/utils.o: $(SRC_DIR)/utils.c $(SRC_DIR)/utils.h
	$(CC) $(CFLAGS) -o $@ -c $<

.PHONY: clean
clean:
	-rm -rf obj libscheduler.so
