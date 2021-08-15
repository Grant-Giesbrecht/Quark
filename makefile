CC = clang++

LIBS = -L/Users/grantgiesbrecht/Documents/GitHub/gstd -lgstd -I/Users/grantgiesbrecht/Documents/GitHub/gstd

isvgen: isvgen.cpp subatomic.hpp
	$(CC) -o isvgen isvgen.cpp $(LIBS)


