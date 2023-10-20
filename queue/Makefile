src := ${shell find -name "*.cpp"}
exec := ${basename ${src}}

run : main
	@./$<

build : ${exec}

% : %.cpp
	g++ $< -o $@ -lpthread -O3

debug :
	@echo ${src}
	@echo ${exec}

clean :
	rm -rf ${exec}

.PHONY : build debug clean run