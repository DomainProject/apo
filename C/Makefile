all: libclingo_solver.so  
	gcc ddm_driver.c ddm.c -I/opt/homebrew/Cellar/clingo/5.6.2/include -L/opt/homebrew/Cellar/clingo/5.6.2/lib -L. -lclingo_solver -lclingo -o ddm_test

clingo_solver.o: clingo_solver.c clingo_solver.h
	gcc -fPIC -I/opt/homebrew/Cellar/clingo/5.6.2/include -c clingo_solver.c

libclingo_solver.so: clingo_solver.o 
	gcc -fPIC -I/opt/homebrew/Cellar/clingo/5.6.2/include \
	-L/opt/homebrew/Cellar/clingo/5.6.2/lib -lclingo clingo_solver.o \
	-o libclingo_solver.so -dynamiclib

clean:
	rm -f clingo_solver.o libclingo_solver.so ddm_test ddm_tmp.asp