CC      = $(CROSS_COMPILER_PREFIX)gcc
CFLAGS  = -O2 -g -Wall -I../../../include
#LDFLAGS = -L ./ -lexpat 
#-ldl -lpthread ../../../lib/libexpat.a
LDFLAGS = -L../../../lib -shared -rdynamic -ldl -lpthread -lexpat
targets = websvr.p
websvr_p_objs = websvr.o psia.o DeviceInfo.o Network.o SystemTime.o Stream.o Image.o Osd.o Serial.o Geographic.o Ptz.o Event.o SystemAudio.o mongoose.o

all: $(targets)

websvr.p: $(websvr_p_objs)
	@echo "[ Building] $@"
	@$(CC) -shared -rdynamic  $^ -o $@ $(LDFLAGS) 

%.o: %.c
	@echo "[Compiling] $^"
	@$(CC) $(CFLAGS) -c $^ -o $@

clean:
	-rm $(targets) $(websvr_p_objs)
