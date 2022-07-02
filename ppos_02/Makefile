
tasks1, tasks2, tasks3:
	cc -Wall ppos_core.c pingpong-tasks1.c -o tasks1
	cc -Wall ppos_core.c pingpong-tasks2.c -o tasks2
	cc -Wall ppos_core.c pingpong-tasks3.c -o tasks3
	
debug:
	cc -Wall -DDEBUG ppos_core.c pingpong-tasks1.c -o tasks1
	cc -Wall -DDEBUG ppos_core.c pingpong-tasks2.c -o tasks2
	cc -Wall -DDEBUG ppos_core.c pingpong-tasks3.c -o tasks3
	

clean:
	rm tasks1 tasks2 tasks3
