CXX = clang++-8
CXXFLAGS = `llvm-config-8 --cxxflags --ldflags --system-libs --libs all`

.PHONY: mc

mc: src/mc.cpp
	$(CXX) $(CXXFLAGS) src/mc.cpp -o mc
clean:
	rm mc output.o
