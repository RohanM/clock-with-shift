build:
	g++ -c testing.cpp -o testing.o
	g++ -c clockwithshift.cpp -o clockwithshift.o
	g++ -c main.cpp -o main.o
	g++ -c test.cpp -o test.o
	g++ -o clockwithshift main.o clockwithshift.o testing.o
	g++ -o test test.o clockwithshift.o testing.o

test: build
	./test
