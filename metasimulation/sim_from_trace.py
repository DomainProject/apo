import sys
from heapq import *
from platform import *
from Simulator.sim import *

skip_queue_validation = True

traces = load_trace(sys.argv[1])  #load trace
num_actors = len(traces)			  

ending_simulation= False
time_window_size = 500 #milliseconds
task_unit_costs  = [0.020]*num_actors #milliseconds to execute an event
task_anno_costs  = [0.100]*num_actors #milliseconds to undo an event


annoyance		 = [] #[[0.0]*num_actors]*num_actors
communication    = [] #[[0.0]*num_actors]*num_actors

for i in range(num_actors):
	annoyance += [[]]
	communication += [[]]
	for j in range(num_actors):
		annoyance[i] += [0]
		communication[i] += [0]

last_ts_idx  = [-1]*num_actors
committed_idxs = [0]*num_actors
init_set = set([])
assignment = []
min_vt = float('inf')

def pack_task(ts, a_id_dst, idx, from_id):
	return (ts, a_id_dst, idx, from_id)

def validate_task_queue(queue, size):
	if skip_queue_validation: return
	if not ending_simulation: assert size == len(queue)
	
	ins = set([])
	for _,id,_,_ in queue:
		ins.add(id)
	assert len(queue) == len(ins)
	
def simulation():
	global assignment
	global ending_simulation
	global communication
	global annoyance

	last_ts_idx  = [-1]*num_actors
	committed_idxs = [0]*num_actors


	cu_units   = sorted(build_cunits())
	cu_units_data    = {}
	for k in cu_units: 	cu_units_data[k] = {'last_wct':0, "queue":[], "len":0}


	print(f"starting assignment...", end='')
	cnt = 0
	for k in sorted(traces.keys()):
	  assignment += [cu_units[cnt]]
	  cnt = (cnt + 1) % len(cu_units)
	assignment = ['gpu_0', 'cpu_0', 'cpu_1', 'cpu_1', 'cpu_1', 'cpu_0', 'cpu_1', 'fpga_0', 'gpu_0', 'cpu_0']
	print(f"done")


	print(f"populate device queue...", end='')
	for a_id in range(len(assignment)):
		heappush( cu_units_data[assignment[a_id]]['queue'], pack_task(0, a_id, 0, num_actors))		
		cu_units_data[assignment[a_id]]['len'] += 1
	print(f"done")


	print(f"populate simulator queue...", end='')
	for cu in cu_units_data:
		validate_task_queue(cu_units_data[cu]['queue'], cu_units_data[cu]['len'])
		schedule_event(0, cu, EVT.EXE_BGN)
	schedule_event(time_window_size, 0, EVT.STAT)
	print(f"done")

	print("BEGIN SIMULATION LOOP")

	while is_there_any_pending_evt():
		#print()
		wct_ts, cur_cu, evt_t, a_id, a_ts, actor_from = dequeue_event()

		#event for statistics aka throughput
		if evt_t == EVT.STAT:
			collect_stats(cu_units_data, wct_ts)
			continue

		cu_data = cu_units_data[cur_cu] 	
		validate_task_queue(cu_data['queue'], cu_data['len'])
							
		s = f"past wall clock time event {cur_cu}: {wct_ts}, {cu_data['last_wct']} evt: {(wct_ts, cur_cu, evt_t, a_id, a_ts, actor_from)}"
		assert wct_ts >= cu_data['last_wct'], s
		cu_data['last_wct'] = wct_ts

		# the device can start the processing of the highest priority actor
		# schedules the EXE_END event 
		if evt_t == EVT.EXE_BGN:	begin_exec(cur_cu, cu_data['queue'], cu_data['last_wct'])
		validate_task_queue(cu_data['queue'], cu_data['len'])

		# the device has received some task from some other device
		# this do not schedule anything, it just updates task queue of the device
		if evt_t == EVT.RCV:		
			assert assignment[a_id] == cur_cu
			recv(cu_data['queue'], a_id, a_ts, actor_from)
			validate_task_queue(cu_data['queue'], cu_data['len'])


		# the device has completed its non-preemptable execution
		# now check received messages in the meantime
		# if this event is out-of-order,  reschedule this type of event to manage the cost for solving inconstencies
		# if it is in order, schedule EXE_BGN 
		if evt_t == EVT.EXE_END:    end_exec(cur_cu, cu_data['queue'], cu_data['last_wct'])
		validate_task_queue(cu_data['queue'], cu_data['len'])
		

		#print(cu_data['queue'])
	#build application.py for throughput estimator
	for i in range(num_actors):
		for j in range(num_actors):
			communication[j][i] = communication[j][i]/min_vt
			annoyance[j][i]     = annoyance[j][i]    /min_vt

	print(f"END SIMULATION LOOP @ {min_vt}")
	f = open('application.py', 'w')
	f.write(f"num_actors        = {num_actors}       \n")
	f.write(f"task_unit_costs   = {task_unit_costs}  \n") 
	f.write(f"task_anno_costs   = {task_anno_costs}  \n") 
	f.write(f"comm_unitary_cost = {comm_unitary_cost}    \n") 

	f.write(f"comm_matrix = [\n")
	for i in range(num_actors):
		f.write(f"{communication[i]}")
		if i != num_actors-1:  		f.write(f",")
		f.write(f"\n")
	f.write(f"]\n")
	
	f.write(f"anno_matrix      = [\n")
	for i in range(num_actors):
		f.write(f"{annoyance[i]}")
		if i != num_actors-1:  		f.write(f",")
		f.write(f"\n")
	f.write(f"]\n")


def collect_stats(cu_units_data, wct_ts):
	global min_vt
	if ending_simulation: return
	min_vt = float('inf')
	for k in cu_units_data:
		tmp = float('inf')
		if len(cu_units_data[k]['queue']) > 0:	tmp = cu_units_data[k]['queue'][0][0]
		min_vt = min(min_vt, tmp)
	
	committed = 0
	for k in traces:
		while committed_idxs[k] < len(traces[k]) and traces[k][committed_idxs[k]][0] < min_vt:
			committed_idxs[k]+=1
			committed+=1


	print("GVT", wct_ts, "SVT",min_vt, "COM", committed, "TH (com per msec)", committed/time_window_size)		
	if is_there_any_pending_evt(): schedule_event(wct_ts + time_window_size, 0, EVT.STAT)


def begin_exec(cur_cu, queue, last_wct):
	global ending_simulation
	if len(queue) == 0: return # no events for this device
	else:
		initial = len(queue)

		vts, actor_id, _, _ = queue[0]  #get next task for this device according to task priority
		actor_evt_idx = last_ts_idx[actor_id] +1 #get next step from trace
		#print("PEEK FROM DEVICE QUEUE", vts, actor_id, "EVT IDX", actor_evt_idx)
			
		end_exe_wct = last_wct
		end_exe_wct += task_unit_costs[actor_id] 		#simulate execution
			
		#just for knowing if all actors have been initiated
		if vts == 0:
			if actor_id not in init_set   : init_set.add(actor_id)
			if len(init_set) == num_actors: print("END ACTOR INIT")
		
		# execute all steps of the trace with same ts
		while True:
			#simulate task send
			a_dest_id = traces[actor_id][actor_evt_idx][2]
			a_dest_ts = traces[actor_id][actor_evt_idx][1]
			a_dest_cu = assignment[a_dest_id]

			#emulate send cost
			end_exe_wct += get_communication_latency(cur_cu, a_dest_cu)

			schedule_event(end_exe_wct, a_dest_cu, EVT.RCV, (a_dest_id, a_dest_ts, actor_id))
			#print("sent",cu_data['last_wct'], a_dest_cu, "RECEIVE", a_dest_id, a_dest_ts, actor_id)
	
			actor_evt_idx+=1
			if actor_evt_idx >= len(traces[actor_id]) or vts != traces[actor_id][actor_evt_idx][0]: #send all output events for the current event
				break

		#update device queue
		if actor_evt_idx < len(traces[actor_id]):
			heapreplace(queue, pack_task(traces[actor_id][actor_evt_idx][0], actor_id, actor_evt_idx, num_actors))
		else:
			# an actor has ended its trace - stop the metasimulation with uncommitted stuff within the simulation
			if not ending_simulation: print("ENDED TRACE", actor_id)
			ending_simulation = True
			heappop(queue)

		#last_ts_idx[actor_id] = actor_evt_idx
		schedule_event(end_exe_wct, cur_cu, EVT.EXE_END)
		if not ending_simulation: assert initial == len(queue)

def end_exec(cur_cu, queue, last_wct):
	global ending_simulation

	if len(queue) == 0: return

	initial = len(queue)
	a_ts, a_id, actor_evt_id, from_id = queue[0]
	last_exec_ts = traces[a_id][last_ts_idx[a_id]][0]

	# received a out-of-order task, hence realign
	end_exe_wct = last_wct
	if last_ts_idx[a_id] >= 0 and a_ts < last_exec_ts: 

		end_exe_wct += task_unit_costs[a_id]
		
		#print("managing out of order task", a_ts, "vs", last_exec_ts, "to",  a_id, "from", from_id, "last_idx", last_ts_idx[a_id], queue)

		outputs = {}
		while last_ts_idx[a_id] >= 0 and a_ts <= traces[a_id][last_ts_idx[a_id]][0]:
			dst_id = traces[a_id][last_ts_idx[a_id]][2]
			dst_ts = traces[a_id][last_ts_idx[a_id]][1]

			if dst_id != a_id:
				if dst_id not in outputs: outputs[dst_id] = float('inf')
				outputs[dst_id] = min(outputs[dst_id], dst_ts)
				#print("FOUND ANTIMESSAGE")
			last_ts_idx[a_id]-=1

		for dst_id in outputs:
			dst_cu = assignment[dst_id]
			end_exe_wct += get_communication_latency(cur_cu, dst_cu) 
			schedule_event(end_exe_wct, dst_cu, EVT.RCV, (dst_id, dst_ts, a_id))


		j = -1
		cnt = 0
		for i in range(len(queue)):
			if queue[i][1] == a_id: 
				j = i
				cnt +=1
		
		#print(len(annoyance), a_id,i,j,queue)
		if not ending_simulation:
			if j < 0 or i >= len(queue) or cnt != 1: print(a_id,i,j,cnt,queue)
			if j < 0 or i >= len(queue) or cnt != 1: print(assignment)
			assert i < len(queue)
			assert cnt == 1
			assert j >= 0

		queue[j] = pack_task(a_ts, a_id, last_ts_idx[a_id], num_actors)
		heapify(queue)
		#if not found:
		#	heappush(queue, pack_task(a_ts, a_id, last_ts_idx[a_id], num_actors) )

		#print(cur_cu, last_wct, "ROLLBACK", a_id, found)
		schedule_event(end_exe_wct, cur_cu, EVT.EXE_END)
		#print(assignment)
		#exit()
	else:
		while last_exec_ts == traces[a_id][last_ts_idx[a_id]][0]:
			last_ts_idx[a_id]+=1
			if last_ts_idx[a_id] >= len(traces[a_id]): #send all output events for the current event
				break

		schedule_event(end_exe_wct, cur_cu, EVT.EXE_BGN)
	if not ending_simulation: assert initial == len(queue)

def recv(queue, a_id, a_ts, actor_from):
	global annoyance, communication
	initial = len(queue)
	if initial == 0: return
	ins = set([])
	j = -1
	cnt = 0
	for i in range(len(queue)):
		ins.add(queue[i][1])
		if queue[i][1] == a_id: 
			j = i
			cnt +=1

	
	#print(len(annoyance), a_id,i,j,queue)
	
	if not ending_simulation: 
		if j < 0 or i >= len(queue) or cnt != 1: print(a_id,i,j,queue)
		if j < 0 or i >= len(queue) or cnt != 1: print(assignment)
		assert i < len(queue)
		assert cnt == 1
		assert j >= 0

	communication[a_id][actor_from] += 1

	if j<0 or queue[j][0] <= a_ts: return #in order hence go on
	
	#print(a_id, actor_from)
	annoyance[a_id][actor_from] += 1	
	queue[j] = pack_task(a_ts, a_id, -1, actor_from)
	heapify(queue) #resort queue 

	assert initial == len(queue)

if __name__ == "__main__":
	simulation()
