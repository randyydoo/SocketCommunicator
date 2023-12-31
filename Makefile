
#########################
# DO NOT EDIT THIS FILE #
#########################

# But please, feel free to inspect the targets and design for your own learning.

#
CC := g++
CFLAGS := -Wall -pedantic -g -c -std=c++17
LINKFLAGS := -Wall -pedantic -g -std=c++17 -pthread


#
BIN_NAME := main
BIN := ./$(BIN_NAME)
#
BIN_TESTS_NAME := main-tests
BIN_TESTS := ./$(BIN_TESTS_NAME)


#
DEFAULT_LISTEN_PORT := 8822
DEFAULT_OUT_HOSTNAME := 127.0.0.1
DEFAULT_OUT_PORT := 8823


#
default:	help


#
help:
	@echo "***** Makefile Menu *****"
	@echo
	@echo "make build         ==> Build source files"
	@echo
	@echo "make run           ==> Run the program"
	@echo "make run2          ==> Run the program with the ports switched, so you can communicate with the normal 'run'"
	@echo "make debug         ==> Debug the program with gdb"
	@echo
	@echo "make test          ==> Run tests against your program"
	@echo "make debug-test    ==> Debug the tests run against your program"
	@echo
	@echo "make clean         ==> Clean temporary build files"


#
build:	$(BIN) $(BIN_TESTS)
.PHONY: build


#
run:	run1
run1:	$(BIN)
	$(BIN) "$(DEFAULT_LISTEN_PORT)" "$(DEFAULT_OUT_HOSTNAME)" "$(DEFAULT_OUT_PORT)"
.PHONY: run run1


#
run2:	$(BIN)
	$(BIN) "$(DEFAULT_OUT_PORT)" "$(DEFAULT_OUT_HOSTNAME)" "$(DEFAULT_LISTEN_PORT)"
.PHONY: run2


#
debug:	$(BIN)
	gdb -ex run --args $(BIN) "$(DEFAULT_LISTEN_PORT)" "$(DEFAULT_OUT_HOSTNAME)" "$(DEFAULT_OUT_PORT)"
.PHONY: debug


#
gradescope:	clean
gradescope:	test
tests:		test
test:		$(BIN_TESTS)
	$(BIN_TESTS)
.PHONY: gradescope tests test


#
debug-tests:		debug-test
debug-test:	$(BIN_TESTS)
	gdb $(BIN_TESTS) -ex run
.PHONY: debug-tests debug-test


#
$(BIN):	main.o Chat.o
	$(CC) $(LINKFLAGS) $^ -o $@


#
$(BIN_TESTS):	CPP_Tests.o Chat.o
	$(CC) $(LINKFLAGS) $^ -o $@


#
main.o:	main.cpp Chat.hpp
	$(CC) $(CFLAGS) $< -o $@


#
Chat.o:	Chat.cpp Chat.hpp
	$(CC) $(CFLAGS) $< -o $@


#
CPP_Tests.o:	CPP_Tests.cpp Chat.hpp puhp-tests/*
	$(CC) $(CFLAGS) $< -o $@


#
clean:
	-rm *.o
	-rm $(BIN)
	-rm $(BIN_TESTS)
	-rm results.json
.PHONY: clean









