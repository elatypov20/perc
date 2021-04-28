CC = gcc
FLAGS = -ggdb
DESTINATION = bin/perc
ARGS = -u egor -g emil -p .

all: build run

debug: build
	gdb ./$(DESTINATION)

build: obj/main.o
	mkdir -p bin
	$(CC) obj/* -o $(DESTINATION) $(FLAGS)
	@echo

obj/main.o: src/main.c
	mkdir -p obj
	$(CC) -c src/main.c -o obj/main.o $(FLAGS)

run:
	@ printf "===============================\n\n"
	@sudo ./$(DESTINATION) $(ARGS) >/dev/stdout

test: build
	sudo groupadd roge
	sudo useradd -N egor
	@printf "===============================\n\n"
	@sudo ./$(DESTINATION) $(ARGS) >/dev/stdout
	@sudo userdel egor
	@sudo groupdel roge

clean:
	@rm -rf bin/* obj/*
	@sudo userdel egor | echo
	@sudo groupdel roge | echo