.PHONY: clean

hdproductid: hdproductid.c
	gcc -s -O2 -W -Wall -Wextra -Werror -o $@ $<
hdproductid.xstatic: hdproductid.c
	xstatic gcc -s -O2 -W -Wall -Wextra -Werror -o $@ $<

clean:
	rm -f hdproductid hdproductid.xstatic
