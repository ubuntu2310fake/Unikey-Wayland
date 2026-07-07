SRCS = charset.cpp convert.cpp data.cpp error.cpp
OBJS = charset.o convert.o data.o error.o

all: vnconv.a

vnconv.a : ${OBJS}
	${AR} ${ARFLAGS} $@ $?

${OBJS}: %.o: %.cpp
	${CXX} ${CXXFLAGS} -c $< -o $@

clean:
	rm *.o
#	rm vnconv.a

depend:
	makedepend -- ${CFLAGS} -- ${SRCS}
# DO NOT DELETE

charset.o: stdafx.h /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
charset.o: /usr/include/search.h /usr/include/features.h
charset.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
charset.o: /usr/include/memory.h /usr/include/string.h /usr/include/ctype.h
charset.o: /usr/include/bits/types.h /usr/include/endian.h
charset.o: /usr/include/bits/endian.h /usr/include/stdlib.h charset.h
charset.o: vnconv.h
convert.o: stdafx.h charset.h vnconv.h /usr/include/stdio.h
convert.o: /usr/include/features.h /usr/include/sys/cdefs.h
convert.o: /usr/include/gnu/stubs.h
convert.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
convert.o: /usr/include/bits/types.h /usr/include/libio.h
convert.o: /usr/include/_G_config.h /usr/include/wchar.h
convert.o: /usr/include/bits/wchar.h /usr/include/gconv.h
convert.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
convert.o: /usr/include/bits/stdio_lim.h /usr/include/stdlib.h
convert.o: /usr/include/string.h
data.o: stdafx.h charset.h vnconv.h
error.o: stdafx.h vnconv.h
