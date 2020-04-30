CC = gcc
CFLAGS = -fPIC -m32 -Wall -g
LDFLAGS = -m32 -g
OBJ_DIR = obj
SRC_DIR = src
MKDIR = mkdir -p

.PHONY: build
.PHONY: directories
build: directories libscheduler.so

directories: ${OBJ_DIR}

${OBJ_DIR}:
	${MKDIR} ${OBJ_DIR}

libscheduler.so: $(OBJ_DIR)/so_scheduler.o $(OBJ_DIR)/priq.o
	$(CC) $(LDFLAGS) -shared -o $@ $^ -lpthread

$(OBJ_DIR)/so_scheduler.o: $(SRC_DIR)/so_scheduler.c $(SRC_DIR)/so_scheduler.h
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ_DIR)/priq.o: $(SRC_DIR)/priq.c $(SRC_DIR)/priq.h
	$(CC) $(CFLAGS) -o $@ -c $<

.PHONY: clean
clean:
	-rm -rf obj libscheduler.so
