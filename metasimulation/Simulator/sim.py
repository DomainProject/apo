from enum import Enum
from heapq import *

wall_clock_time_events = []
verbose=False

class EVT(Enum):

	TIME_WINDOW = 0
	RCV  = 1
	EXE_BGN = 2
	EXE_END = 3
	SND_BGN = 2
	SND_END = 3
	def __lt__(self, other):
		if self.__class__ is other.__class__:
			return self.value < other.value
		return NotImplemented

def load_trace(path):
	f = open(path)
	traces = {}
	cnt=0
	for line in f.readlines():
		if cnt == 0 and ',' in line: 
			cnt+=1
			continue
		if cnt != 0 and ',' in line:
			line = line.strip().split(',')
			if int(line[1]) not in traces: traces[int(line[1])] = []
			#print(line)
			traces[int(line[1])] += [ ( float(line[0]), float(line[2]), int(line[3]) ) ]
	f.close()
	for k in traces:
		traces[k] = sorted(traces[k], key=lambda x:x[0])
	return traces

def schedule_event(wct, dest_device, type, payload=None, debug=verbose):
	a = -1
	b = -1
	c = -1
	if payload != None:
		a,b,c = payload
	t = (wct, dest_device, type, a, b, c)
	heappush( wall_clock_time_events, t )
	if debug: print("ADDING TO DEVICE",  t)

def is_there_any_pending_evt():
	return len(wall_clock_time_events) != 0

def dequeue_event(debug=verbose):
	t = heappop(wall_clock_time_events)
	if debug: print("SCHEDULING DEVICE",t )
	return t

def get_events_count_vector_in_next_window(window_size, actors):
	res = [0]*actors
	for e in wall_clock_time_events:
		if e[0] < window_size and e[3] != -1:
			res[e[3]] += 1
	return res
