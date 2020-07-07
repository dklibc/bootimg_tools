.PHONY: all, clean

all: bootimg_print bootimg_split bootimg_combine

LDFLAGS+=-s

bootimg_print: bootimg_print.c
	gcc $(LDFLAGS) $(CFLAGS) -o $@ $^

bootimg_split: bootimg_split.c
	gcc $(LDFLAGS) $(CFLAGS) -o $@ $^

bootimg_combine: bootimg_combine.c
	gcc $(LDFLAGS) $(CFLAGS) -o $@ $^

clean:
	rm bootimg_split bootimg_combine bootimg_print
