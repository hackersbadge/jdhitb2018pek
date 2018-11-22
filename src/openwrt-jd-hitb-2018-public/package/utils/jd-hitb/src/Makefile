CFLAGS+= -Os -std=gnu99

OBJECTS = jd-hitb                                                                                                                        

all : $(OBJECTS)

PKG_NAME = jd-hitb


jd-hitb: jd-hitb.o oled.o
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -lpthread

clean:
	rm -rf jd-hitb *.d *.o
