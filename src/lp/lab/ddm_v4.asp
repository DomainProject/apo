% -----
% rules for distributing actor(s) on cu(s)
% each actor can run on a computing unit only (if runnable_on)
1 { run_on(A,U) : cu(U), runnable_on(A,T), cu_type(U,T) } 1 :- actor(A).

% T is the total workload of cu U
cu_workload(U,T) :- cu(U), T = #sum{ W,A : run_on(A,U), tasks_forecast(A,W) }. % sum W (1st component)

% cu_overload(U,O) iff O is the overload of U
cu_overload(U,O) :- cu_workload(U,W), cu_capacity(U,C), 
                    W > C, O = W-C. 

% - optimization: workload 
#minimize{ O @ 1,U : cu_overload(U,O) }.

% -----
% actor communication cost
% assumption: the cost of exchanging messages on the same cu is irrelevant (*)
a_cc(A1,A2,C) :- msg_exch_rate(A1,A2,R), 
                 run_on(A1,U1), run_on(A2,U2), U1 != U2, % (*)
                 cu_type(U1,T1), cu_type(U2,T2), 
                 msg_exch_cost(T1,T2,C1),
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
