all:  scaf_feeder.o

test: scaf_feeder_test

OBJ = scaf_feeder_test.o scaf_feeder.o


scaf_feeder_test: $(OBJ)
	gcc -o scaf_feeder_test *.o -g -ldl

.s.o:
	as -o $*.o $*.s

.c.o:
	gcc -c $*.c -o $*.o -g

clean:
	rm -f *~
	rm -f *.o
	rm -f scaf_feeder_test

