OBJECTS = main.o
TARGET = ddk.sys
TARGET_SIGNED = ddk_signed.sys
CFLAGS = -s -I/usr/x86_64-w64-mingw32/include/ddk/
CC=x86_64-w64-mingw32-gcc-win32

all: $(TARGET_SIGNED)

clean:
	rm -f $(OBJECTS) $(TARGET) $(TARGET_SIGNED) $(OBJECTS:.o=.d)

-include $(OBJECTS:.o=.d)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
	$(CC) -MM $(CFLAGS) $*.c > $*.d

%.a: %.def
	x86_64-w64-mingw32-dlltool -d $< -l $@ -kill-at


$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -shared -Wl,--subsystem,native -Wl,--image-base,0x10000 \
-Wl,--file-alignment,0x1000 -Wl,--section-alignment,0x1000 \
-Wl,--entry,DriverEntry@8 -Wl,--stack,0x40000 -Wl,--dynamicbase -Wl,--nxcompat \
-nostartfiles -nostdlib -o $(TARGET) \
$(OBJECTS) -lntoskrnl -lhal
	$(SIGN_CMD)

SIGN_SEVER=100.84.43.103
ifneq ($(SIGN_SEVER), )
SIGN_CMD=python sign_client.py $(SIGN_SEVER)  8000 ddk.sys ddk_signed.sys
$(TARGET_SIGNED): $(TARGET)
	$(SIGN_CMD)
endif

