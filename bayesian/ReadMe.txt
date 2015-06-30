input format:
line 1: a integer N for number of nodes
next N parts for node itself:
	line 1: 3 space separated things, 1 string and 2 integers.
		String for name, integer S for number of states. (default node index is from 0 to N-1)
	line 2: S space separated strings for state name. (default state index is from 0 to S-1)
empty line.
next N parts for structure and conditional possibilities:
	line 1: 2 space separated integers.
		integer D of node id; integer P for number of parent 
	line 2: P space separated integers/strings for parents ids/names (by ascending order).
	line 3 ~ (product of parent states): P (of node D) integers/strings for states indexes/names (ascending order of node id); and S decimals for conditional possibilities (ascending order of state)
	