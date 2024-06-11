LDFLAGS = -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi -lSDL2
LIBS = -L/usr/lib/x86_64-linux-gnu -lm

CurseEngine:
	gcc -o renderLinDebug CurseEngine.c -g -Og -Iinclude $(LDFLAGS) $(LIBS)

.PHONY: test clear

test:
	./renderLinDebug

clear:
	rm -f renderLinDebug
