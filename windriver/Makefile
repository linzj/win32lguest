OBJECTS = main.o
TARGET = ddk.sys
TARGET_SIGNED = ddk_signed.sys
CFLAGS = -s -I/usr/x86_64-w64-mingw32/include/ddk/
CC=x86_64-w64-mingw32-gcc-win32

all: $(TARGET_SIGNED)

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
	$(SIGN_CMD)

SIGN_SEVER=100.84.43.103
ifneq ($(SIGN_SEVER), )
SIGN_CMD=python sign_client.py $(SIGN_SEVER)  8000 ddk.sys ddk_signed.sys
$(TARGET_SIGNED): $(TARGET)
	$(SIGN_CMD)
endif

