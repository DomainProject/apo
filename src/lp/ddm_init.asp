% num. of actor(s)
actor(1..8).

% runnable_on(A,C) holds iff
% A can run on the cus beloning to class C (octal value)
runnable_on(1..4,1). % cpu
runnable_on(5..8,3). % cpu & gpu
runnable_on(6..8,7). % cpu & gpu & fpga

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
% num. of computing unit(s)
cu(1..25).

cu_type(1..20,cpu).  % cpu w/20 cores
cu_type(21..24,gpu). % 4 gpu(s)
cu_type(25,fpga).    % 1 fpga

% CU capacity (task/tw)
% cu_capacity(U,C) holds iff
% C is the capacity of the computing unit U
cu_capacity(1..20,200).
cu_capacity(21..24,500).
cu_capacity(25,300).

% msg_exch_cost(U1,U2,C) holds iff 
% the cost of exchanging a message between U1 and U2
% U1 <-> U2 of the same type
msg_exch_cost(1..20,1..20,1).
msg_exch_cost(21..24,21..24,1). 
msg_exch_cost(25,25,1). 
% cpu <-> {gpu,fpga}
msg_exch_cost(1..20,21..24,2). 
msg_exch_cost(1..20,25,2).  
% gpu <-> fpga
msg_exch_cost(21,25,3).
msg_exch_cost(22,25,3).
msg_exch_cost(23,25,3).
msg_exch_cost(24,25,3).

% this rule represents the fact that sending a message from X to Y 
% has the same cost of sending a message from Y to X 
msg_exch_cost(X,Y,C) :- msg_exch_cost(Y,X,C).

% mutual_annoyance(A1,A2,N) holds iff 
% A1 and A2 experienced N rollbacks (when executed on different CUs)
mutual_annoyance(1,2,20).
mutual_annoyance(1,3,10).
