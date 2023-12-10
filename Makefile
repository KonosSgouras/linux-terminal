CC = g++

TARGET = mysh
SOURCES= main.cpp general_util.cpp syscall_util.cpp
all:$(TARGET)

$(TARGET):$(SOURCES)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES)

clean:
	$(RM) $(TARGET)