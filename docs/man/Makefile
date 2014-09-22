all: prepare html pdf

prepare:
	doxygen Doxyfile

html:

pdf:
	cd latex && patch -p1 < ../refman.tex.patch
	make -C latex

clean:
	-rm -rf html latex

.PHONY: prepare html pdf

