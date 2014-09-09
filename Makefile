include Common.mk

subdirs = src examples tests

all: $(subdirs)
	$(call make_subdir, $^)

clean: $(subdirs)
	$(call make_subdir, $^, clean)

distclean: $(subdirs)
	$(call make_subdir, $^, clean)
	-rm lib/*
