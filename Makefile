CC = clang
CFLAGS = -g -Wall
TARGET = bank.out
OBJS = main.o clientMenu.o serverBankManagement.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^
	rm -f $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean check
