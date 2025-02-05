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
#minimize{ O @ 0 : cu_overload(U,O) }.

% -----
% U is busy iff there exists an actor associated to U
busy(U) :- run_on(_,U).

% - optimization: maximize the number of active cus 
#maximize{ 1@1,U : busy(U)}.

% -----
% actor communication cost
% assumption: the cost of exchanging messages on the same cu is irrelevant (*)
a_cc(A1,A2,C) :- msg_exch_rate(A1,A2,R), 
                 run_on(A1,U1), run_on(A2,U2), U1 != U2, % (*)
                 msg_exch_cost(U1,U2,C1), 
                 C = C1*R.

% total communication cost
%cc(T) :- T = #sum{ C,A1,A2: a_cc(A1,A2,C) }. % sum C (1st component)

% - optimization: communication cost
%#minimize{ C @ 2 : cc(C) }.
#minimize{ C@2,A1,A2 : a_cc(A1,A2,C) }.

% -----
% annoyance(N) holds iff N is the global annoyance, that is, 
% the sum of the mutual annoyace vales of all actors (when running on different CUs)
%annoyance(N) :- N = #sum{ C,A1,A2: 
%                mutual_annoyance(A1,A2,C), 
%                run_on(A1,U1), run_on(A2,U2),
%                U1 != U2 
%                }.

% - optimization: annoyance
%#minimize{ N @ 3 : annoyance(N) }.
#minimize{ C@3,A1,A2 : mutual_annoyance(A1,A2,C), 
                       run_on(A1,U1), run_on(A2,U2),
                       U1 != U2  }.

% utility directives
#show run_on/2.