# Compiler and flags
CC=g++ -I ./ -g -Wall -std=c++17 -Wno-deprecated-declarations 

# wSender-base source files
WSENDERBASE_SRC = wSender-base.cpp helper.cpp
WSENDERBASE_TARGET = wSender-base

# wReceiver-base source files
WRECEIVERBASE_SRC = wReceiver-base.cpp helper.cpp
WRECEIVERBASE_TARGET = wReceiver-base

# wSender-opt source files
WSENDEROPT_SRC = wSender-opt.cpp helper.cpp
WSENDEROPT_TARGET = wSender-opt

# wReceiver-opt source files
WRECEIVEROPT_SRC = wReceiver-opt.cpp helper.cpp
WRECEIVEROPT_TARGET = wReceiver-opt

all: ${WSENDERBASE_TARGET} ${WRECEIVERBASE_TARGET} ${WSENDEROPT_TARGET} ${WRECEIVEROPT_TARGET}

# Compile wSender-base
${WSENDERBASE_TARGET}: ${WSENDERBASE_SRC}
	${CC} -o ${WSENDERBASE_TARGET} ${WSENDERBASE_SRC} -pthread

# Compile wReceiver-base
${WRECEIVERBASE_TARGET}: ${WRECEIVERBASE_SRC}
	${CC} -o ${WRECEIVERBASE_TARGET} ${WRECEIVERBASE_SRC} -pthread

# Compile wSender-opt
${WSENDEROPT_TARGET}: ${WSENDEROPT_SRC}
	${CC} -o ${WSENDEROPT_TARGET} ${WSENDEROPT_SRC} -pthread

# Compile wReceiver-opt
${WRECEIVEROPT_TARGET}: ${WRECEIVEROPT_SRC}
	${CC} -o ${WRECEIVEROPT_TARGET} ${WRECEIVEROPT_SRC} -pthread
clean:
	rm -f ${WSENDERBASE_TARGET} ${WRECEIVERBASE_TARGET} ${WSENDEROPT_TARGET} ${WRECEIVEROPT_TARGET}