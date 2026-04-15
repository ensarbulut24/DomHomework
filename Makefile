CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -I/usr/include/libxml2
LDFLAGS = -lxml2

TARGET = flightTool
SRCS = flightTool.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

%.o: %.c flightTool.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) flightdata.dat flightlogs.xml flightlogs_utf16le.xml flightlogs_utf16be.xml flightlogs_utf8.xml

test: $(TARGET)
	@echo "--- 1. CSV to Binary ---"
	./$(TARGET) flightlog.csv flightdata.dat 1 -separator 1 -opsys 2
	@echo "--- 2. Binary to XML ---"
	./$(TARGET) flightdata.dat flightlogs.xml 2 -separator 1 -opsys 2
	@echo "--- 3. XSD Validation ---"
	./$(TARGET) flightlogs.xml flightlogs.xsd 3 -separator 1 -opsys 2
	@echo "--- 4. Encoding UTF-16LE ---"
	./$(TARGET) flightlogs.xml flightlogs_utf16le.xml 4 -separator 1 -opsys 2 -encoding 1
	@echo "--- 5. Encoding UTF-16BE ---"
	./$(TARGET) flightlogs.xml flightlogs_utf16be.xml 4 -separator 1 -opsys 2 -encoding 2
	@echo "--- 6. Encoding UTF-8 ---"
	./$(TARGET) flightlogs.xml flightlogs_utf8.xml 4 -separator 1 -opsys 2 -encoding 3
	@echo "--- TUM TESTLER TAMAMLANDI ---"
