main: main.o
	arm-linux-gnueabihf-gcc -o main main.c -lpthread
main.o: main.c
	arm-linux-gnueabihf-gcc -c main.c
clean: main main.o
	rm main main.o
