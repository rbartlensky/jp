OUT = build

all: jp

jp: src/*.c
	gcc -O3 -o ./jp $? -I ./include

clean:
	rm ./jp

.PHONY : all clean jp
