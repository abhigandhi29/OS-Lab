all: demo1 demo2 
  
demo1: demo1.o libmemlab.a 
	g++ -Wall -O2 demo1.o -lmemlab -L. -lpthread -o demo1 
  
demo2: demo2.o libmemlab.a 
	g++ -Wall -O2 demo2.o -lmemlab -L. -lpthread -o demo2 
  
  
demo1.o: demo1.cpp
	g++ -Wall -O2 -c demo1.cpp 
  
demo2.o: demo2.cpp 
	g++ -Wall -O2 -c demo2.cpp
  
  
libmemlab.a: memlab.o 
	ar -rcs libmemlab.a memlab.o  
  
memlab.o: memlab.cpp memlab.h 
	g++ -Wall -O2 -c memlab.cpp 
  
clean: 
	rm -f demo1 demo2 demo1.o demo2.o libmemlab.a memlab.o