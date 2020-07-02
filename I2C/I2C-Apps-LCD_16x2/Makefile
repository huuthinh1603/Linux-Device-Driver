main: main.o lcd_16x2.o
	arm-linux-gnueabihf-gcc -o main main.o lcd_16x2.o
main.o: main.c
	arm-linux-gnueabihf-gcc -c main.c
lcd_16x2.o: lcd_16x2.c lcd_16x2.h
	arm-linux-gnueabihf-gcc -c lcd_16x2.c
clean:
	rm main.o main lcd_16x2.o

