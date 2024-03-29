CC = clang++ -std=c++17

LIBS = -L/Users/grantgiesbrecht/Documents/GitHub/gstd -lgstd -I/Users/grantgiesbrecht/Documents/GitHub/gstd -L/Users/grantgiesbrecht/Documents/GitHub/KTable -lktable -I/Users/grantgiesbrecht/Documents/GitHub/KTable

isvgen: isvgen.cpp subatomic.hpp
	$(CC) -o isvgen isvgen.cpp $(LIBS)

isvpub: isvpub.cpp subatomic.hpp
	$(CC) -o isvpub isvpub.cpp $(LIBS)

LutTypes.o: LutTypes.cpp
	$(CC) -c LutTypes.cpp $(LIBS)

qmm: qmm.cpp subatomic.hpp
	$(CC) -o qmm qmm.cpp $(LIBS)

# qlexer: quarklexer.cpp subatomic.hpp quark_types.hpp
# 	$(CC) -o qlexer quarklexer.cpp $(LIBS)

quark: quark.cpp quarklexer.hpp quarkparser.hpp subatomic.hpp quark_types.hpp data_types.hpp
	$(CC) -o quark quark.cpp $(LIBS)

alloc_test: ram_alloc_test.cpp quarklexer.hpp quarkparser.hpp subatomic.hpp quark_types.hpp data_types.hpp
	$(CC) -o alloc_test ram_alloc_test.cpp $(LIBS)
