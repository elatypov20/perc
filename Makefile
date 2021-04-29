CC = gcc
DESTINATION = bin/perc

all: build

build: obj/main.o
	mkdir -p bin
	$(CC) obj/* -o $(DESTINATION)
	@echo

obj/main.o: src/main.c
	mkdir -p obj
	$(CC) -c src/main.c -o obj/main.o $(FLAGS)

debug:
	mkdir -p bin
	$(CC) obj/* -o $(DESTINATION) -ggdb
	@echo
	gdb ./$(DESTINATION)


# create tests for the program
generate_test:
	mkdir -p tests
	mkdir -p tests/eugene tests/ivan tests/shared

	sudo groupadd eugene_and_ivan
	sudo groupadd dmitry_group

	sudo useradd eugene -G eugene_and_ivan
	sudo useradd ivan -G eugene_and_ivan
	sudo useradd dmitry -G dmitry_group

	mkdir -p tests/eugene/eugen_readable_dir
	sudo chmod u+r-w-x,g-r-w-x,o-r-w-x tests/eugene/eugen_readable_dir
	sudo chown eugene tests/eugene/eugen_readable_dir
	sudo chgrp eugene_and_ivan tests/eugene/eugen_readable_dir

	mkdir -p tests/eugene/eugen_writable_dir
	sudo chmod u-r+w-x,g-r-w-x,o-r-w-x tests/eugene/eugen_writable_dir
	sudo chown eugene tests/eugene/eugen_writable_dir
	sudo chgrp eugene_and_ivan tests/eugene/eugen_writable_dir

	mkdir -p tests/eugene/recursive/dir/1/2/3/
	touch tests/eugene/recursive/dir/1/2/3/recursion_works
	sudo chmod u-r+w-x,g-r-w-x,o-r-w-x tests/eugene/recursive/dir/1/2/3/recursion_works
	sudo chown eugene tests/eugene/recursive/dir/1/2/3/recursion_works
	sudo chgrp eugene_and_ivan tests/eugene/recursive/dir/1/2/3/recursion_works

	ln -s tests/eugene/eugen_writable_dir tests/eugene/symbolic_link

	touch tests/ivan/cant_open_file1
	sudo chown ivan tests/ivan/cant_open_file1
	sudo chgrp eugene_and_ivan tests/ivan/cant_open_file1
	sudo chmod u-r-w-x,g-r-w-x,o-r-w-x tests/ivan/cant_open_file1
	touch tests/ivan/can_open_file2
	sudo chown ivan tests/ivan/can_open_file2
	sudo chgrp eugene_and_ivan tests/ivan/can_open_file2
	sudo chmod u-r+w-x,g-r-w-x,o-r-w-x tests/ivan/can_open_file2

	touch tests/shared/only_for_eugene_and_ivan.txt
	sudo chgrp eugene_and_ivan tests/shared/only_for_eugene_and_ivan.txt

	touch tests/shared/very_shared.txt
	sudo chmod u+r+w+x,g+r+w+x,o+r+w+x tests/shared/very_shared.txt





test: build generate_test
	echo
	@echo "running test for eugene"
	sudo ./bin/perc -u eugene -g eugene_and_ivan -p tests

	@echo
	@echo "========================================="
	@echo
	@echo "running test for ival"
	sudo ./bin/perc -u ivan -g eugene_and_ivan -p tests

	@echo
	@echo "========================================="
	@echo
	@echo "running test for dmitry"
	sudo ./bin/perc -u dmitry -g dmitry -p tests


clean:
	rm -rf bin/* obj/* tests/
	sudo deluser eugene
	sudo deluser ivan
	sudo deluser dmitry
	sudo delgroup eugene_and_ivan
	sudo delgroup dmitry_group