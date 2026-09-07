% -----------------------------------------------------------------------------
% CLUSTERs of CU --------------------------------------------------------------
% -----------------------------------------------------------------------------
% ---------------------------------------------------
% cluster_of_cu maps a computing unit U to cluster C,
% where C is identified by the minumum computing unit identifier in the cluster.
% Each computing unit belongs to a single cluster only 
1 { cluster_of_cu(U,C) : cu(C), C<=U } 1 :- cu(U).  

% if a cluster is identified by C, the C belongs to that cluster
 :- cluster_of_cu(U,C), not cluster_of_cu(C,C).

% computing units of the same type belong to the same cluster
 :- cluster_of_cu(U1,C), cluster_of_cu(U2,C), U1!=U2,
    cu_type(U1,T1), cu_type(U2,T2), T1!=T2. 

% computing units with same capacity belong to the same cluster
 :- cluster_of_cu(U1,C), cluster_of_cu(U2,C), U1!=U2,
    cu_capacity(U1,C1), cu_capacity(U2,C2), C1!=C2. 

% computing units of the same type, capacity and comm. cost
% belong to the same cluster
 :- cluster_of_cu(U1,C1), cluster_of_cu(U2,C2), C1!=C2,
    same_capacity(U1,U2), 
    same_type(U1,U2),
    same_comc(U1,U2).

same_capacity(U1,U2) :- 
  cu_capacity(U1,C), cu_capacity(U2,C).

same_type(U1,U2) :- 
  cu_type(U1,T), cu_type(U2,T).

same_comc(U1,U2) :- 
  msg_exch_cost(U1,U,C), msg_exch_cost(U2,U,C), cu(U), U1!=U2, U1!=U.

% ----------------------------------------------------
% (predicate to associated indexes to clusters)
% C is a cluster, % where C is identified by the 
% minumum computing unit identifier in the cluster.
cluster(C) :- cluster_of_cu(U,C).

% N is the number of clusters.
num_clusters(N) :- 
  N = #count{ C : cluster(C) }.

% cluster_index associated a cluster C with an index I (renaming function)
1 { cluster_index(C,0..N-1) : num_clusters(N) } 1 :- cluster(C).

% cluster_index/2 is strictly monotonic:
% if C1 < C2, then cluster_index(C1) < cluster_index(C2).
:- cluster_index(C1,N1), cluster_index(C2,N2), C1<C2, N1>=N2.

% ----------------------------------------------------
% N is the number of computing unit in cluster C.
cluster_card(CI,N) :- 
  N = #count{ U : cluster_of_cu(U,C) }, cluster(C), cluster_index(C,CI), N>=1.

% Given any cluster C, cluster_idx_cu(C,I,U) maps 
% the index I to the corresponding computing unit U
1 { cluster_idx_cu(CI,0..N-1,U) : cluster_card(CI,N) } 1 :- 
  cluster_of_cu(U,C),
  cluster_index(C,CI). 

% cluster_idx_cu is a monotonically decreasing function
  :- cluster_idx_cu(C,I1,U1), cluster_idx_cu(C,I2,U2),
     I2=I1+1, 
     U1>U2. 

% cluster_idx_cu is a function of (C,I)
  :- cluster_idx_cu(C,I,U1), cluster_idx_cu(C,I,U2),
     U1 != U2. 


cu_num(N) :- N=#count{ I : cu(I) }.


% -----------------------------------------------------------------------------
% INTER-cluster allocation ----------------------------------------------------
% -----------------------------------------------------------------------------

% number of actors per cluster 
% cluster_nactors(C,N,R) holds iff 
% N is the number of actors assigned to cluster C, and 
% R is the residual number of actors to be assigned to clusteres [C+1,...,C_l] 
% where C_l is the index of the last cluster 
1 { cluster_nactors(0,N,R) : N>=0, N<=n+1, R=(n+1)-N } 1.

1 { cluster_nactors(C,N,R) : N>=0, N<=n+1, R=R1-N, N<=R1 } 1 :- 
  C=C1+1, C=1..M-1,
  num_clusters(M),
  cluster_nactors(C1,N1,R1).

% if C is the index of the last cluster, then there must be no actors left to assign.
:- cluster_nactors(C,_,R), R>0, num_clusters(N), C=N-1.   

% -------------------------------------------------------------------------------
% INTRA-cluster allocation ------------------------------------------------------
% -------------------------------------------------------------------------------

% cu_nactors(C,U,N,R) holds iff 
% N is the number of actors assigned to cu U in cluster C, and
% R is the residual number of actors to be assigned to cus [U+1,...,U_l] 
% where U_l is the index of the last cu in a given cluster 

1 { cu_nactors(C,0,N,R) : N>=1, N<=n+1, N<=A, R=A-N } 1 :- 
  cluster_nactors(C,A,_), A>0.

1 { cu_nactors(C,U,N,R) : N>=0, N<=n, N<=A-1, N<=N1, N<=R1, R=R1-N } 1 :- 
  U=U1+1, U=1..M-1,
  cu_nactors(C,U1,N1,R1),
  cluster_card(C,M),
  cluster_nactors(C,A,_), A>0.

% if cluster C has 0 actors (cluster_nactors(C,0,_)), 
% then all its cus U cu_in_cluster(C,U) have 0 cu assigned to them (cu_nactors(C,U,0,0)).
 :- cluster_nactors(C,0,_), cluster_idx_cu(C,I,_), not cu_nactors(C,I,0,0).

 :- cu_nactors(C,I1,_,0), cluster_idx_cu(C,I1,_), cluster_idx_cu(C,I2,_), I2=I1+1, not cu_nactors(C,I2,0,0).

% The number of actors assigned to computing units in a cluster is a monotonically decreasing function of the computing unit identifier.
% 1) If U has no actors assigned to it, then there must be no actors left to assign.
:- cu_nactors(C,U,0,N), N>0. 
% 2) If U2=U1+1, then the number of actors N2 assigned to U2 can't be greater than the number of actors T1 left to assign
:- cu_nactors(C,U1,N1,R1), cu_nactors(C,U2,N2,R2), U1>=0, U2=U1+1, N2>R1.
% 3) If U2=U1+1, then the number of actors N2 assigned to U2 can't be greater than the number of actors N1 assigned to U1
:- cu_nactors(C,U1,N1,R1), cu_nactors(C,U2,N2,R2), U1>=0, U2=U1+1, N2>N1.

% assigning actors to cus ------------------------------------------------------
% U must run exactly N actors
N { run_on(A,U) : acu_runnable_on(A,U) } N :- 
  cu_nactors(C,I,N,_), cluster_idx_cu(C,I,U).  

% an actor runs on a single CU
:- run_on(A,U1), run_on(A,U2), U1!=U2.

% utility predicate (to simplify the rule above)
acu_runnable_on(A,U) :- 
    cu(U), 
    runnable_on(A,O), 
    oct_dev(O,D), 
    cu_type(U,D).

% oct_dev: pairs octal (value) - device (name)  
oct_dev(1,cpu).
                oct_dev(2,gpu).
oct_dev(3,cpu). oct_dev(3,gpu).
                                oct_dev(4,fpga).
oct_dev(5,cpu).                 oct_dev(5,fpga).
                oct_dev(6,gpu). oct_dev(6,fpga).
oct_dev(7,cpu). oct_dev(7,gpu). oct_dev(7,fpga).


% -----------------------------------------------------------------------------
% OPTIMIZATION statements -----------------------------------------------------
% -----------------------------------------------------------------------------

apt(1000).
cpt(1000).

% -----
% parameters
% overload magnitude (om) & priority (op)
om(1).
op(2).
% communication cost magnitude (cm) & priority (cp)
cm(1).
%cp(2).
cp(P) :- P=0, cpe(X), X<T, cpt(T).
cp(P) :- P=1, cpe(X), X>=T, cpt(T).
cpe(R) :- N = #sum{ A,A1,A2 : msg_exch_rate(A1,A2,A) },
         M = #max{ I : actor(I) },
         D = M*M,
         R = N/D.

% annoyance magnitude (am) & priority (ap)
am(1).
ap(P) :- P=0, ape(X), X<T, apt(T).
ap(P) :- P=1, ape(X), X>=T, apt(T).
ape(R) :- N = #sum{ A,A1,A2 : mutual_annoyance(A1,A2,A) },
          M = #max{ I : actor(I) },
          D = M*M,
          R = N/D.

% -----
% T is the total workload of cu U
% cu_workload(U,T) :- cu(U), cu_capacity(U,C),
%     #sum{ W,A : run_on(A,U), tasks_forecast(A,W) } > C, wmean(M), T = N * M,
%     N = #count{ A : run_on(A,U) }.
cu_workload(U,T) :- cu(U),
    wmean(M), T = N * M,
    N = #count{ A : run_on(A,U) }.

% T is the total system workload (all tasks to be processed)
tasks(X) :- X = #sum{ W,A : tasks_forecast(A,W) }.
wmean(M) :- tasks(X), N=#count{ A : actor(A) }, M = X / N.

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

% #minimize{ CM*C@CP,A1,A2 :
%                  msg_exch_rate(A1,A2,R),
%                  run_on(A1,U1), run_on(A2,U2), U1 != U2, % (*)
%                  msg_exch_cost(U1,U2,C1),
%                  C = C1*R, cm(CM), cp(CP) }.

% -----
% - optimization: annoyance
%#minimize{ N @ 3 : annoyance(N) }.
#minimize{ AM*C@AP,A1,A2 : mutual_annoyance(A1,A2,C),
                       run_on(A1,U1), run_on(A2,U2),
                       U1 != U2, am(AM), ap(AP)   }.


%powercap(100000000).
%consumption(cpu, 5).
%consumption(gpu, 2).
%consumption(fpga, 1).

% Discard solutions exceeding power cap
%:- #sum{ W*C,A : run_on(A,U), tasks_forecast(A,W), consumption(D,C), cu_type(U,D) } > PC, powercap(PC).
