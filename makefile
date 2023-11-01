all:
	g++ -D _DEBUG -ggdb3 -std=c++17 -O0 -Wall -Wextra -Weffc++ -Wc++14-compat -Wmissing-declarations							\
	-Wcast-qual -Wchar-subscripts -Wconversion -Wctor-dtor-privacy -Wempty-body									\
	-Wfloat-equal -Wformat-nonliteral -Wformat-security -Wformat=2 -Winline -Wnon-virtual-dtor						\
	-Wopenmp -Woverloaded-virtual -Wpacked -Wpointer-arith -Winit-self -Wredundant-decls -Wshadow -Wsign-conversion				\
	-Wstrict-overflow=2																					\
	-Wsuggest-override -Wswitch-default -Wswitch-enum -Wundef -Wunreachable-code -Wunused										\
	-Wvariadic-macros  -Wno-reserved-user-defined-literal -Wno-missing-field-initializers -Wno-narrowing -Wno-old-style-cast	\
	-Wno-varargs -Wstack-protector -fcheck-new -fsized-deallocation -fstack-protector -fstrict-overflow 						\
	-fno-omit-frame-pointer -Wlarger-than=8192 -Wframe-larger-than=8192 -fPIE													\
	-fsanitize=address,bool,bounds,enum,float-cast-overflow,float-divide-by-zero,integer-divide-by-zero,				\
	assembler.cpp ASM_funcs.cpp -o assembler
