CFLAGS= -Wall -g -I/usr/src/linux-source-4.10.0/linux-source-4.10.0/tools/include

app:test_mtd.o 
	$(CC) $(CFLAGS) -o $@ $< 
test_mtd.o:test_mtd.c 
	$(CC) $(CFLAGS) -c $< 

clean:
	rm -rf *.o 
