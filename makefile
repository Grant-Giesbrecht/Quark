CC = clang++ -std=c++11

LIBS = -L/Users/grantgiesbrecht/Documents/GitHub/gstd -lgstd -I/Users/grantgiesbrecht/Documents/GitHub/gstd -L/Users/grantgiesbrecht/Documents/GitHub/KTable -lktable -I/Users/grantgiesbrecht/Documents/GitHub/KTable

isvgen: isvgen.cpp subatomic.hpp
	$(CC) -o isvgen isvgen.cpp $(LIBS)
