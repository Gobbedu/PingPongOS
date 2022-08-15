# Eduardo Gobbo Willi Vasconcellos Gonçalves GRR20203892

CFLAGS= -Wall
# PPOS_TEST=pingpong-semaphore
PPOS_TEST=pingpong-racecond
.PHONY: $(PPOS_TEST)

all: $(PPOS_TEST)

$(PPOS_TEST):
	@gcc $(CFLAGS) $(PPOS_TEST).c ppos_core.c ppos_ipc.c queue.c -o $(PPOS_TEST)


debug: CFLAGS+= -DCONTAB
debug: all

clean:
	@rm -f pingpong-semaphore
	@rm -f pingpong-racecond
	


