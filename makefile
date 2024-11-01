# Compiler and flags
CC=g++ -I ./ -g -Wall -std=c++17 -Wno-deprecated-declarations 

# wSender-base source files
WSENDERBASE_SRC = wSender-base.cpp helper.cpp
WSENDERBASE_TARGET = wSender-base

WRECEIVERBASE_SRC = wReceiver-base.cpp helper.cpp
WRECEIVERBASE_TARGET = wReceiver-base

all: ${WSENDERBASE_TARGET} ${WRECEIVERBASE_TARGET}

# Compile wSender-base
${WSENDERBASE_TARGET}: ${WSENDERBASE_SRC}
	${CC} -o ${WSENDERBASE_TARGET} ${WSENDERBASE_SRC} -pthread

# Compile wReceiver-base
${WRECEIVERBASE_TARGET}: ${WRECEIVERBASE_SRC}
	${CC} -o ${WRECEIVERBASE_TARGET} ${WRECEIVERBASE_SRC} -pthread
clean:
	rm -f ${WSENDERBASE_TARGET} ${WRECEIVERBASE_TARGET}