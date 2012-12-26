HEADERS  = tprpg.h
OBJECTS  = tprpg.o print_hex.o randombytes.o
BINARIES = tprpg cards blackjack
LIBS =

CC=gcc
#CC=clang -fcatch-undefined-behavior
#CFLAGS=-Os -Wall -m32 -g -std=gnu99 -pedantic -Wall -Wextra -funsigned-char -Wno-pointer-sign -fdata-sections -ffunction-sections -Wl,--gc-sections
#CFLAGS=-Os -Wall -m32 -g -std=gnu99 -pedantic -Wall -Wextra -funsigned-char -Wno-pointer-sign
CFLAGS=-O3 -Wall -g -std=gnu99 -pedantic -Wall -Wextra -funsigned-char -Wno-pointer-sign

all: $(BINARIES)

.o:
	$(CC) $(CFLAGS) -c $< -o $@

test:  test.o  $(OBJECTS)
	$(CC) $(CFLAGS) test.o $(OBJECTS) $(LIBS) -o test

tprpg: main.o $(OBJECTS)
	$(CC) $(CFLAGS) main.o $(OBJECTS) $(LIBS) -o tprpg

cards: cards.o $(OBJECTS)
	$(CC) $(CFLAGS) cards.o $(OBJECTS) $(LIBS) -o cards

blackjack: blackjack.o $(OBJECTS)
	$(CC) $(CFLAGS) blackjack.o $(OBJECTS) $(LIBS) -o blackjack

clean:
	rm -rf $(BINARIES) *.o
