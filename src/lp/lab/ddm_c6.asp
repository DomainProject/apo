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
cu_workload(U,T) :- cu(U), 
    T = #sum{ W,A : run_on(A,U), tasks_forecast(A,W) }.

% T is the total system workload (all tasks to be processed)
tasks(X) :- X = #sum{ W,A : tasks_forecast(A,W) }.

% O is the overload of U
%cu_overload(U,O) :- cu(U),
%  O = #max{ 0 ; W-C : cu_workload(U,W), cu_capacity(U,C) }.

% max and min cu_overload
%max_cu_overload(M) :- M = #max{ 0 ; O : cu_overload(U,O) }.
%min_cu_overload(M) :- M = #min{ O : cu_overload(U,O) ; T : tasks(T) }.

% - optimization: difference between max and min overload 
% #minimize{ Mo-Mi : max_cu_overload(Mo), min_cu_overload(Mi) }.

% -----
% actor communication cost
% assumption: the cost of exchanging messages on the same cu is irrelevant (*)
a_cc(A1,A2,C) :- msg_exch_rate(A1,A2,R), 
                 run_on(A1,U1), run_on(A2,U2), U1 != U2, % (*)
                 msg_exch_cost(U1,U2,C1), 
                 C = C1*R.

max_a_cc(C) :- 
    C = #max{ D : a_cc(A1,A2,D) }.

%#minimize{ C@2 : max_a_cc(C) }.
#minimize{ C@1 : a_cc(A1,A2,C) }.

%#minimize{ C@1,A1,A2: a_cc(A1,A2,C) }.

% total communication cost
%cc(T) :- T = #sum{ C,A1,A2: a_cc(A1,A2,C) }. % sum C (1st component)

% - optimization: communication cost
%#minimize{ C@2 : cc(C) }.


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
%#minimize{ C@1,A1,A2 : mutual_annoyance(A1,A2,C), 
%                       run_on(A1,U1), run_on(A2,U2),
%                       U1 != U2  }.

% utility directives
#show run_on/2.

max_task_forecast(F) :- 
    F = #max{ W : tasks_forecast(A,W) }.

num_tasks(K) :-
    K = #sum{ 1,T : tasks_forecast(T,L)}.

num_cus(K) :-
    K = #sum{ 1,T : cu_type(T,L)}.

max_num_unbalanced_cus(K):
    K = T\C, num_cus(C), num_tasks(T).

unbalanced(U,0) :-
   cu_workload(U,T), T <= (K/C)*F, num_tasks(K), num_cus(C), max_task_forecast(F).

unbalanced(U,1) :-
   cu_workload(U,T), T > (K/C)*F, num_tasks(K), num_cus(C), max_task_forecast(F).

num_unbalanced_cus(K) :-
    K = #sum{ 1,T : B=1,unbalanced(T,B) }.

%:- cu_workload(U,T), T > (K/C)*F, num_tasks(K), num_cus(C), max_task_forecast(F).

:- N > K, num_unbalanced_cus(N), max_num_unbalanced_cus(K).

min_actor_with_cpu(A) :-
    A = #min{ B : actor(B), run_on(B,C), cu_type(C,cpu)}.

% TODO handle the case of multiple units beloging to a device which is not a cpu 
:- C1 > C2, A1 < A2, run_on(A1,C1), run_on(A2,C2), cu_type(C1, cpu), cu_type(C2, cpu), min_actor_with_cpu(A1).