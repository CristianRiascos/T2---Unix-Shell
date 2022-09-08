CC = gcc
%.o: %.c  
	$(CC) -c -o $@ $<

t2: t2.o
	gcc -o t2 t2.o

clean:
	rm -f *.o t2