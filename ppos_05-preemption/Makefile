CFLAGS= -Wall
PPOS_N=pingpong-preempcao

all: $(PPOS_N)

$(PPOS_N):
	@cc $(CFLAGS) $(PPOS_N).c ppos_core.c queue.c -o $(PPOS_N)

stress: PPOS_N=pingpong-preempcao-stress
stress: all

timer:
	@cc timer.c -o timer

signal:
	@cc signal.c -o signal

debug: CFLAGS+= -DDEBUG
debug: all

clean:
	@rm -f $(PPOS_N)
	@rm -f $(PPOS_N)-stress
	@rm -f timer
	@rm -f signal


