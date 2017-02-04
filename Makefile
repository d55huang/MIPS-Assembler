CXX = g++-5
CXXFLAGS = -std=c++14 -g -MMD
EXEC = asm
OBJECTS = asm.o kind.o lexer.o
DEPENDS = ${OBJECTS:.o=.d}

${EXEC}: ${OBJECTS}
	${CXX} ${CXXFLAGS} ${OBJECTS} -o ${EXEC}

-include ${DEPENDS}

.PHONY: clean

clean:
	rm ${OBJECTS} ${EXEC} ${DEPENDS}
