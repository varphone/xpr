include ../Common.mk
-include ../Config.mk

CFLAGS   = $(BUILD_OPT_FLAGS) -I../include -I../contrib/include
CXXFLAGS = $(CFLAGS)
#LDFLAGS = -L../lib -L../contrib/lib -Wl,-rpath,\$$ORIGIN -Wl,-rpath,\$$ORIGIN/lib -Wl,-rpath,\$$ORIGIN/plugins


LDFLAGS = -L../lib -L../contrib/lib -Wl,-Bdynamic -lxpr -lmpi -lmal -Wl,-Bstatic  -lresampler  -laec -lanr -ljpeg -lpciv  -lhdmi -ljpeg -lVoiceEngine -lvqev2 -lexpat -laacenc -Wl,-Bdynamic -ldl


targets =  test
objs = test.o

all: $(targets)

$(targets): $(objs)
	@echo "[ Building] $@"
	$(CC)  $^ -o $@ $(LDFLAGS) 

%.o: %.c
	@echo "[Compiling] $^"
	$(CC) $(CFLAGS) -c $^ -o $@


clean:
	-rm $(targets) $(objs)

#test: $(targets)
#	$(call run_test, $^)

.PHONY: clean distclean install test
