CFLAGS+= -Os -std=gnu99

OBJECTS = jd-hitb-ctf

all : $(OBJECTS)

PKG_NAME = jd-hitb-ctf


jd-hitb-ctf: jd-hitb-ctf.o oled.o
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -lpthread

clean:
	rm -rf jd-hitb-ctf *.d *.o
