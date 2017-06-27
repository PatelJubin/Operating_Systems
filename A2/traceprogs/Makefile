SRCS = simpleloop.c matmul.c blocked.c simplestack.c
PROGS = simpleloop matmul blocked simplestack

all : $(PROGS)

$(PROGS) : % : %.c
	gcc -Wall -g -o $@ $<


traces: $(PROGS)
	./runit simpleloop
	./runit matmul 100
	./runit blocked 100 25
	./runit simplestack

.PHONY: clean
clean : 
	rm -f simpleloop matmul blocked simplestack tr-*.ref *.marker *~
