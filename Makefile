
TARGET = lsystem
SRCS = lsystem.c parse.c utils.c

TARGET2 = lsystemOpenMP
SRCS2 = lsystemOpenMP.c parse.c utils.c

TARGET3 = lsystemNoGrafico
SRCS3 = lsystemNoGrafico.c parse.c utils.c

TARGET4 = lsystemNoGraficoParalelo
SRCS4 = lsystemNoGraficoParalelo.c parse.c utils.c

CC = gcc
CFLAGS = -Wall -O3 `sdl2-config --cflags`

LDFLAGS = `sdl2-config --libs` -lm
LDFLAGS2 = `sdl2-config --libs` -lm -fopenmp

all:
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LDFLAGS2)

paralel:
	$(CC) $(CFLAGS) -o $(TARGET2) $(SRCS2) $(LDFLAGS2)

nografico:
	$(CC) $(CFLAGS) -o $(TARGET3) $(SRCS3) $(LDFLAGS2)

nograficoparalelo:
	$(CC) $(CFLAGS) -o $(TARGET4) $(SRCS4) $(LDFLAGS2)

clean:
	rm -f $(TARGET) $(TARGET2) $(TARGET3) $(TARGET4)
