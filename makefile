all:		mylang2ir

mylang2ir:	main.o
			g++ -std=c++14 main.o -o mylang2ir

main.o:		main.cpp
			g++ -std=c++14 -c main.cpp -o main.o
