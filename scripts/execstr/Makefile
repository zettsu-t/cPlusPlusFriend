TARGET=execstr
SOURCE=execstr.c

CC=gcc
CPPFLAGS=-O0 -g
LIBPATH=
LDFLAGS=
LIBS=

IN_TEXT=intext.txt
BIN_TEXT=bintext.txt

.PHONY: all run clean

all: $(TARGET)

run: $(TARGET)
	./$<

$(TARGET): $(SOURCE) $(BIN_TEXT) Makefile
	$(CC) $(CPPFLAGS) $(LIBPATH) -o $@ $< $(LDFLAGS) $(LIBS)

$(BIN_TEXT): $(IN_TEXT) Makefile
	cat $< | tr -d '\r\n' | od -t x1 -A none | tr -d '\r\n' | sed -e "s/ /,0x/g" | tail -c +2 | sed -e 's/^/#define BIN_TEXT "/' -e 's/.*/&"/' > $@

clean:
	rm -f $(TARGET) $(BIN_TEXT)
