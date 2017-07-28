all: ext2_ls ext2_ln ext2_mkdir ext2_cp ext2_rm

ext2_ls: ext2_ls.c ext2.h ext2_utilities.h ext2_utilities.c 
	gcc -Wall -o $@ $^

ext2_mkdir: ext2_mkdir.c ext2.h ext2_utilities.h ext2_utilities.c 
	gcc -Wall -o $@ $^

ext2_rm: ext2_rm.c ext2.h ext2_utilities.h ext2_utilities.c  
	gcc -Wall -o $@ $^

ext2_ln: ext2_ln.c ext2.h ext2_utilities.h ext2_utilities.c 
	gcc -Wall -o $@ $^
	
ext2_cp: ext2_cp.c ext2.h ext2_utilities.h ext2_utilities.c 
	gcc -Wall -o $@ $^

clean : 
	rm -f *.o ext2_ls *~
	rm -f *.o ext2_cp *~
	rm -f *.o ext2_ln *~
	rm -f *.o ext2_rm *~
	rm -f *.o ext2_mkdir *~