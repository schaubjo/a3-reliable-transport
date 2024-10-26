# Compiler and flags
CC=g++ -I ./ -g -Wall -std=c++17 -Wno-deprecated-declarations 

# wSender-base source files
WSENDERBASE_SRC = wSender-base.cpp socket.cpp
WSENDERBASE_TARGET = wSender-base

all: ${WSENDERBASE_TARGET}

# Compile wSender-base
${WSENDERBASE_TARGET}: ${WSENDERBASE_SRC}
	${CC} -o ${WSENDERBASE_TARGET} ${WSENDERBASE_SRC} -pthread

clean:
	rm -f ${WSENDERBASE_TARGET}