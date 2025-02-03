% -----
% rules for distributing actor(s) on cu(s)
% each actor can run on a computing unit only (if runnable_on)
1 { run_on(A,U) : cu(U), runnable_on(A,O), oct_dev(O,D), cu_type(U,D) } 1 :- actor(A).

% oct_dev: pairs octal (value) - device (name)  
oct_dev(1,cpu).
                oct_dev(2,gpu).
oct_dev(3,cpu). oct_dev(3,gpu).
                                oct_dev(4,fpga).
oct_dev(5,cpu).                 oct_dev(5,fpga).
                oct_dev(6,gpu). oct_dev(6,fpga).
oct_dev(7,cpu). oct_dev(7,gpu). oct_dev(7,fpga).


% T is the total workload of cu U
cu_workload(U,T) :- cu(U), T = #sum{ W,A : run_on(A,U), tasks_forecast(A,W) }. % sum W (1st component)

% cu_overload(U,O) iff O is the overload of U
cu_overload(U,O) :- cu_workload(U,W), cu_capacity(U,C), 
                    W > C, O = W-C. 

% - optimization: workload 
%#minimize{ O @ 1,U : cu_overload(U,O) }.
#minimize{ O @ 1 : cu_overload(U,O) }.

% -----
% actor communication cost
% assumption: the cost of exchanging messages on the same cu is irrelevant (*)
a_cc(A1,A2,C) :- msg_exch_rate(A1,A2,R), 
                 run_on(A1,U1), run_on(A2,U2), U1 != U2, % (*)
                 msg_exch_cost(U1,U2,C1), 
                 C = C1*R.

% total communication cost
cc(T) :- T = #sum{ C,A1,A2: a_cc(A1,A2,C) }. % sum C (1st component)

% - optimization: communication cost
#minimize{ C @ 2 : cc(C) }.

% -----
% annoyance(N) holds iff N is the global annoyance, that is, 
% the sum of the mutual annoyace vales of all actors (when running on different CUs)
annoyance(N) :- N = #sum{ C,A1,A2: 
                mutual_annoyance(A1,A2,C), 
                run_on(A1,U1), run_on(A2,U2),
                U1 != U2 
                }.

% - optimization: annoyance
#minimize{ N @ 3 : annoyance(N) }.

% utility directives
#show run_on/2.
%%% facts
cu(0..3).
cu_type(0,cpu).
cu_type(1,cpu).
cu_type(2,gpu).
cu_type(3,fpga).
msg_exch_cost(0,0,1).
msg_exch_cost(0,1,1).
msg_exch_cost(0,2,2).
msg_exch_cost(0,3,2).
msg_exch_cost(1,0,1).
msg_exch_cost(1,1,1).
msg_exch_cost(1,2,2).
msg_exch_cost(1,3,2).
msg_exch_cost(2,0,2).
msg_exch_cost(2,1,2).
msg_exch_cost(2,2,4).
msg_exch_cost(2,3,4).
msg_exch_cost(3,0,2).
msg_exch_cost(3,1,2).
msg_exch_cost(3,2,4).
msg_exch_cost(3,3,4).
actor(0..7).
runnable_on(0,7).
runnable_on(1,7).
runnable_on(2,7).
runnable_on(3,7).
runnable_on(4,7).
runnable_on(5,7).
runnable_on(6,7).
runnable_on(7,7).
tasks_forecast(0,10).
tasks_forecast(1,10).
tasks_forecast(2,10).
tasks_forecast(3,10).
tasks_forecast(4,10).
tasks_forecast(5,10).
tasks_forecast(6,10).
tasks_forecast(7,10).
cu_capacity(0,1).
cu_capacity(1,1).
cu_capacity(2,1).
cu_capacity(3,1).
msg_exch_rate(0,0,1).
msg_exch_rate(1,1,1).
msg_exch_rate(2,2,1).
msg_exch_rate(3,3,1).
msg_exch_rate(4,4,1).
msg_exch_rate(5,5,1).
msg_exch_rate(6,6,1).
msg_exch_rate(7,7,1).

#show cu_overload/2.