OUT = build

all : jp

jp: src/*.c
	gcc -o ./jp $? -I ./include


clean:
	rm ./jp

.PHONY : all jp
