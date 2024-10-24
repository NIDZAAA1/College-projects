SRC_DIR = ./src
INC_DIR = ./inc
MISC_DIR = ./misc
TESTS_DIR = ./tests

clean:
	rm -f parser.tab.c parser.tab.h lex.yy.c ${TESTS_DIR}/program.hex ${TESTS_DIR}/assembler ${TESTS_DIR}/linker ${TESTS_DIR}/emulator ${TESTS_DIR}/*.o

assembler:  ${MISC_DIR}/lexer.l ${MISC_DIR}/parser.y ${SRC_DIR}/assembler.c ${INC_DIR}/assembler.h
	bison -d ${MISC_DIR}/parser.y 
	flex ${MISC_DIR}/lexer.l
	cc -w -g -o ${TESTS_DIR}/assembler parser.tab.c ${SRC_DIR}/assembler.c -lfl

linker: ${SRC_DIR}/linker.cpp ${INC_DIR}/linker.hpp
	g++ -g -o ${TESTS_DIR}/linker ${SRC_DIR}/linker.cpp

emulator: ${SRC_DIR}/emulator.cpp ${INC_DIR}/emulator.hpp
	g++ -g -o ${TESTS_DIR}/emulator ${SRC_DIR}/emulator.cpp
