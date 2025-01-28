from heapq import *

wall_clock_time_events = []
verbose=False

def schedule_event(wct, dest_device, type, payload=None, debug=verbose):
	a = -1
	b = -1
	c = -1
	if payload != None:
		a,b,c = payload
	t = (wct, dest_device, type, b, a, c)
	heappush( wall_clock_time_events, t )
	t = (wct, dest_device, type, a, b, c)
	if debug: print("ADDING TO DEVICE",  t)

def is_there_any_pending_evt():
	return len(wall_clock_time_events) != 0

def peek():
	return wall_clock_time_events[0][0]

def dequeue_event(debug=verbose):
	#print(wall_clock_time_events)
	wct, dest_device, type, b, a, c = heappop(wall_clock_time_events)
	
	t = (wct, dest_device, type, a, b, c)
	if debug: print("SCHEDULING DEVICE", t)
	return t

def get_events_count_vector_in_next_window(window_size, actors):
	res = [0]*actors
	for e in wall_clock_time_events:
		if e[0] < window_size and e[4] != -1:
			res[e[4]] += 1
	return res
