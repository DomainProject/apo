% utility directives
#show run_on/2.

% -----
% parameters
om(1). op(1). % overload magnitude (om) & priority (op)
cm(1). cp(1). % communication cost magnitude (cm) & priority (cp)
am(1). ap(0). % annoyance magnitude (am) & priority (ap)

% -----
% rules for distributing actor(s) on cu(s)
% each actor can run on a computing unit only (if runnable_on)
% 1 { run_on(A,U) : cu(U), runnable_on(A,O), oct_dev(O,D), cu_type(U,D) } 1 :- actor(A).
1 { run_on(A,U) : acu_runnable_on(A,U) } 1 :- actor(A).

% utility predicate (to simplify the rule above)
acu_runnable_on(A,U) :- cu(U), runnable_on(A,O), oct_dev(O,D), cu_type(U,D).

#show acu_runnable_on(1,U) : acu_runnable_on(1,U).

% oct_dev: pairs octal (value) - device (name)  
oct_dev(1,cpu).
                oct_dev(2,gpu).
oct_dev(3,cpu). oct_dev(3,gpu).
                                oct_dev(4,fpga).
oct_dev(5,cpu).                 oct_dev(5,fpga).
                oct_dev(6,gpu). oct_dev(6,fpga).
oct_dev(7,cpu). oct_dev(7,gpu). oct_dev(7,fpga).


% -----
% T is the total workload of cu U
cu_workload(U,T) :- cu(U), 
    T = #sum{ W,A : run_on(A,U), tasks_forecast(A,W) }.

% T is the total system workload (all tasks to be processed)
tasks(X) :- X = #sum{ W,A : tasks_forecast(A,W) }.

% O is the overload of U
cu_overload(U,O) :- cu(U),
  O = #max{ 0 ; W-C : cu_workload(U,W), cu_capacity(U,C) }.

% max and min cu_overload
max_cu_overload(M) :- M = #max{ 0 ; O : cu_overload(U,O) }.
min_cu_overload(M) :- M = #min{ O : cu_overload(U,O) ; T : tasks(T) }.

% - optimization: difference between max and min overload 
#minimize{ OM*D@OP : D=Mo-Mi, max_cu_overload(Mo), min_cu_overload(Mi), om(OM), op(OP) }.

% -----
% actor communication cost
% assumption: the cost of exchanging messages on the same cu is irrelevant (*)
a_cc(A1,A2,C) :- msg_exch_rate(A1,A2,R), 
                 run_on(A1,U1), run_on(A2,U2), U1 != U2, % (*)
                 msg_exch_cost(U1,U2,C1), 
                 C = C1*R.

% - optimization: communication cost
%#minimize{ C @ 2 : cc(C) }.
#minimize{ CM*C@CP,A1,A2 : a_cc(A1,A2,C), cm(CM), cp(CP) }.

% -----
% - optimization: annoyance
%#minimize{ N @ 3 : annoyance(N) }.
#minimize{ AM*C@AP,A1,A2 : mutual_annoyance(A1,A2,C), 
                       run_on(A1,U1), run_on(A2,U2),
                       U1 != U2, am(AM), ap(AP)   }.
