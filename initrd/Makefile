O?=..

all:
	find . ! -path "./.git*" \
	       ! -path "./README.md" \
	       ! -path "./Makefile" \
	| cpio -H newc  -ov > $(O)/initrd.img
