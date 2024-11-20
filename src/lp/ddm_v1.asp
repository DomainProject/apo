% actor(s) and message(s)
actor(1..8).

runnable_on(A,cpu) :- actor(A).
runnable_on(5..8,gpu).
runnable_on(6..8,fpga).

% msg_exch_rate(A1,A2,R) holds iff 
% A1 and A2 exchange R messages per time window (msg/tw).
msg_exch_rate(1,2,10).
msg_exch_rate(1,3,20).
msg_exch_rate(1,4,10).
msg_exch_rate(1,5,40).
% 
msg_exch_rate(6,7,35).
msg_exch_rate(7,8,20).

% actor workload (task/tw)
workload(1..4,25).
workload(5,20). 
workload(6..7,50).
workload(8,80).

% -------------------
% computing unit(s)
% cpu w/20 cores, 4 gpu, 1 fpga
cu(1..25).

cu_type(1..20,cpu).
cu_type(21..24,gpu).
cu_type(25,fpga).

% CU throughput (task/tw)
throughput(1..20,200).
throughput(21..24,500).
throughput(25,300).

% energy consumption (J)
energy(fpga,75).
energy(cpu,200).
energy(gpu,300).

% -----
% rules for distributing actor(s) on cu(s)
% each actor can run on a computing unit only (if runnable_on)
1 { run_on(A,U) : cu(U), runnable_on(A,T), cu_type(U,T) } 1 :- actor(A).
% if A1 and A2 exchange messages, then A1 and A2 must run_on the same cu 
run_on(A2,U) :- msg_exch_rate(A1,A2,_), run_on(A1,U).
% we get better performance using the rule above w.r.t. the following ic
% :- msg_exch_rate(A1,A2,_), run_on(A1,U1), run_on(A2,U2), U1!=U2.

% T is the total workload of cu U
cu_workload(U,T) :- cu(U), T = #sum{ W,A : run_on(A,U), workload(A,W) }. % sum W (1st component)

% WARNING: quite strong constraint
% the cu_workload of U can't be greater than its throughput
:- cu_workload(U,W), throughput(U,T), W > T. 

% - optimization: workload/throughput aspects
% C is the sum of the msg. exchange rate between actors running on two different cu
inter_cu_msg_exch(C) :- C = #sum{ M : run_on(A1,U1), run_on(A2,U2), U1!=U2, A1!=A2, msg_exch_rate(A1,A2,M) }.

% minimize the inter cu message exchange rate
%#minimize{ C : inter_cu_msg_exch(C) }.
#minimize{ C @ 2 : inter_cu_msg_exch(C) }.

% - optimization: energy
% E is the energy consumption of unit U of type T (computed if something will run on it)
cu_energy(U,E) :- run_on(A,U), energy(T,E), cu_type(U,T).

% total energy consuption (all cu)
tot_energy(T) :- T = #sum{ E,U : cu_energy(U,E) }.

#minimize{ E @ 1 : tot_energy(E) }.


% utility directives
#show run_on/2.
#show inter_cu_msg_exch/1.