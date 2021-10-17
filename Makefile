build-main:
	g++ -g -c testing.cpp -o testing.o
	g++ -g -c clockwithshift.cpp -o clockwithshift.o
	g++ -g -c main.cpp -o main.o
	g++ -o clockwithshift -g main.o clockwithshift.o testing.o

build-test:
	g++ -g -c testing.cpp -o testing.o
	g++ -g -c clockwithshift.cpp -o clockwithshift.o
	g++ -g -c test.cpp -o test.o
	g++ -o test -g test.o clockwithshift.o testing.o

test: build-test
	./test

clean:
	rm -f *.o test clockwithshift
