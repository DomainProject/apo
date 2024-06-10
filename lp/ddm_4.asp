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
% rules for distributing actor(s) on cu(s)
% each actor can run on a computing unit only (if runnable_on)
1 { run_on(A,U) : cu(U), runnable_on(A,T), cu_type(U,T) } 1 :- actor(A).

% T is the total workload of cu U
cu_workload(U,T) :- cu(U), T = #sum{ W,A : run_on(A,U), tasks_forecast(A,W) }. % sum W (1st component)

% cu_overload(U,O) iff O is the overload of U
cu_overload(U,O) :- cu_workload(U,W), cu_capacity(U,T), W > T, O = W-T. 

% - optimization: workload 
#minimize{ O @ 1,U : cu_overload(U,O) }.

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

% actor communication cost
% assumption: the costs of exchanging messages on the same cu is irrelevant (*)
a_cc(A1,A2,C) :- msg_exch_rate(A1,A2,R), 
                 run_on(A1,U1), run_on(A2,U2), U1 != U2, % (*)
                 cu_type(U1,T1), cu_type(U2,T2), 
                 msg_exch_cost(T1,T2,C1), 
                 C = C1*R.

% total communication cost
cc(T) :- T = #sum{ C,A1,A2: a_cc(A1,A2,C) }. % sum C (1st component)

% - optimization: communication cost
#minimize{ C @ 2 : cc(C) }.

% mutual_annoyance(A1,A2,N) holds iff 
% A1 and A2 experienced N rollbacks (when executed on different CUs)
mutual_annoyance(1,2,20).
mutual_annoyance(1,3,10).

annoyance(N) :- N = #sum{ C,A1,A2: 
                mutual_annoyance(A1,A2,C), 
                run_on(A1,U1), run_on(A2,U2),
                U1 != U2 
                }.

% - optimization: annoyance
#minimize{ N @ 3 : annoyance(N) }.

% utility directives
#show run_on/2.
#show annoyance/1.
#show cu_overload/2.
#show a_cc/3.