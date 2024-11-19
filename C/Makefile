UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    CCFLAGS += -shared
endif
ifeq ($(UNAME_S),Darwin)
    CCFLAGS += -dynamiclib
	INC = -I/opt/homebrew/Cellar/clingo/5.6.2/include
	LIB = -L/opt/homebrew/Cellar/clingo/5.6.2/lib
endif

all: libclingo_solver.so  
	gcc ddm_driver.c ddm.c $(INC) $(LIB) -L. -lclingo_solver -lclingo -o ddm_test

clingo_solver.o: clingo_solver.c clingo_solver.h
	gcc -fPIC $(INC) -c clingo_solver.c

libclingo_solver.so: clingo_solver.o 
	gcc -fPIC $(INC) $(LIB) -lclingo clingo_solver.o -o libclingo_solver.so -dynamiclib

clean:
	rm -f clingo_solver.o libclingo_solver.so ddm_test ddm_tmp.asp