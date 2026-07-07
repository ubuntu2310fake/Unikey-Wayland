OBJS = uvconvert.o
SRCS = uvconvert.cpp
LIBS = ../vnconv/vnconv.a
INCLUDE_DIR = ../vnconv

all: uvconv
uvconv: ${OBJS} vnconvlib
	${CXX} ${CXXFLAGS} -o $@ ${OBJS} ${LIBS}

${OBJS}: %.o: %.cpp
	${CXX} ${CXXFLAGS} -c $< -o $@ -I${INCLUDE_DIR}

.PHONY: vnconvlib
vnconvlib:
	cd ../vnconv && make all
depend:
	makedepend -- ${CFLAGS} -- ${SRCS} -I${INCLUDE_DIR}
	cd ../vnconv && make depend
clean:
	rm *.o
	cd ../vnconv && make clean

# DO NOT DELETE

uvconvert.o: prehdr.h /usr/include/stdlib.h /usr/include/features.h
uvconvert.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
uvconvert.o: /usr/include/ctype.h /usr/include/bits/types.h
uvconvert.o: /usr/include/endian.h /usr/include/bits/endian.h
uvconvert.o: /usr/include/string.h ../vnconv/vnconv.h
