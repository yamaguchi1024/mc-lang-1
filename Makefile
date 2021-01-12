all: bin obj bin/mc

bin:
	mkdir -p bin

obj:
	mkdir -p obj

bin/mc: obj/mc.o
	clang++ -pie -pthread -g -O0 -UNDEBUG obj/mc.o `llvm-config --cxxflags --ldflags --system-libs --libs all` -o bin/mc

# obj/mc.d
obj/mc.o: src/mc.cpp src/lexer.h src/parser.h src/codegen.h src/helper/helper.h	
	clang++ -MMD -MP -MF obj/mc.d -c -fPIE -fPIC -g -O0 -UNDEBUG -fno-limit-debug-info `llvm-config --cxxflags --ldflags` src/mc.cpp -o obj/mc.o

clean:
	rm -rf mc mc.o
