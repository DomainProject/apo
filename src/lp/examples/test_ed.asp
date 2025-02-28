% actor(s) and message(s)
actor(1..8).

runnable_on(1..4,1). % cpu, 
runnable_on(5..8,3). % cpu, gpu
runnable_on(6..8,7). % cpu, gpu & fpga

% msg_exch_rate(A1,A2,R) holds iff 
% A1 and A2 exchange R messages per time window (msg/tw).
msg_exch_rate(1,2,10).
msg_exch_rate(1,3,20).
msg_exch_rate(1,4,10).
msg_exch_rate(1,5,1).
% 
msg_exch_rate(6,7,35).
msg_exch_rate(7,8,20).

% tasks_forecast(A,T) holds iff 
% T is the number of tasks to be executed by the actor A
tasks_forecast(1..4,50).
tasks_forecast(5,20). 
tasks_forecast(6..7,50).
tasks_forecast(8,80).

% -------------------
% computing unit(s)
% cpu w/20 cores, 4 gpu, 1 fpga
cu(1..25).

cu_type(1..20,cpu).
cu_type(21..24,gpu).
cu_type(25,fpga).

% CU capacity (task/tw)
cu_capacity(1..20,200).
cu_capacity(21..24,500).
cu_capacity(25,300).

% -----
% msg_exch_cost(U1,U2,C) holds iff the cost of exchanging a message between cu of different type is C
% U1 <-> U2 of the same type
msg_exch_cost(cpu,cpu,1).
msg_exch_cost(gpu,gpu,1). 
msg_exch_cost(fpga,fpga,1). 
% cpu <-> {gpu,fpga}
msg_exch_cost(cpu,gpu,2). msg_exch_cost(gpu,cpu,2). 
msg_exch_cost(cpu,fpga,2). msg_exch_cost(fpga,cpu,2). 
% gpu <-> fpga
msg_exch_cost(gpu,fpga,3). msg_exch_cost(fpga,gpu,3). 

% mutual_annoyance(A1,A2,N) holds iff 
% A1 and A2 experienced N rollbacks (when executed on different CUs)
mutual_annoyance(1,2,20).
mutual_annoyance(1,3,10).