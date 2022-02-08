all: seeker seeker64

seeker: seeker.c
	gcc -o seeker seeker.c

seeker64: seeker64.c
	gcc -o seeker64 seeker64.c

clean:
	rm -f seeker seeker64
