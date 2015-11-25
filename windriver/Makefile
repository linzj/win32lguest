OBJECTS = main.o
TARGET = ddk.sys
CFLAGS = -s -I/usr/x86_64-w64-mingw32/include/ddk/
CC=x86_64-w64-mingw32-gcc-win32

all: $(TARGET)

clean:
	rm -f ddk.o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.a: %.def
	x86_64-w64-mingw32-dlltool -d $< -l $@ -kill-at

$(TARGET): $(OBJECTS)
	$(CC) -Wl,-base-file,base.tmp -Wl,-entry,DriverEntry \
	-nostartfiles -nostdlib -o junk.tmp $(OBJECTS) -lntoskrnl -lhal -s
	rm junk.tmp
	x86_64-w64-mingw32-dlltool --as=as --dllname $(TARGET) --base-file base.tmp \
	--output-exp temp.exp
	rm base.tmp
	$(CC) $(CFLAGS) -Wl,-subsystem,native -Wl,-image-base,0x10000 \
	-Wl,-file-alignment,0x1000 -Wl,-section-alignment,0x1000 \
	-Wl,-entry,DriverEntry -Wl,-stack,0x40000 -Wl,temp.exp \
	-mdll -nostartfiles -nostdlib -o $(TARGET) \
	$(OBJECTS) -lntoskrnl -lhal
