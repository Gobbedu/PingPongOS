# Eduardo Gobbo Willi Vasconcellos Gonçalves GRR20203892

CFLAGS= -Wall
PPOS_TEST=pingpong-contab
.PHONY: $(PPOS_TEST)

all: $(PPOS_TEST)

$(PPOS_TEST):
	@gcc $(CFLAGS) $(PPOS_TEST).c ppos_core.c queue.c -o $(PPOS_TEST)

stress: PPOS_TEST=pingpong-contab-prio
stress: all

clean:
	@rm -f pingpong-contab
	@rm -f pingpong-contab-prio



