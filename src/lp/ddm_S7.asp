actor(0..n).
cu(0..3).
cu_class(0).
%cu_class(1).
%cu_in_class(0,0..1).
%cu_in_class(1,2..3).
cu_in_class(0,0..3).
%
max_cu_id(C,U) :- U = #max{ I : cu_in_class(C,I) }, cu_class(C).
min_cu_id(C,U) :- U = #min{ I : cu_in_class(C,I) }, cu_class(C).

% number of actors per class ---------------------------------------------------
% class_nactors(C,N,R) holds iff 
% N is the number of actors assigned to class C, and 
% R is the residual number of actors to be assigned to classes [C+1,...,C_l] 
% where C_l is the index of the last class 
1 { class_nactors(0,N,R) : N>=0, N<=n+1, R=(n+1)-N } 1.

1 { class_nactors(C,N,R) : N>=0, N<=n+1, R=R1-N, N<=R1 } 1 :- 
                           cu_class(C), cu_class(C1), C>=1, C=C1+1, class_nactors(C1,N1,R1).
% if C is the index of the last class, then there must be no actors left to assign.
:- class_nactors(C,_,R), R>0, C=#max{I : cu_class(I)}. 

% TODO: check this!
%:- M = #sum{ N : class_nactors(C,N,T) }, M != n+1.

% number of actors per cu ------------------------------------------------------
% cu_nactors(U,N,R) holds iff 
% N is the number of actors assigned to cu U, and 
% R is the residual number of actors to be assigned to cus [U+1,...,U_l] 
% where C_l is the index of the last cu in a given class 
1 { cu_nactors(U,N,R) : N>=1, N<=n+1, N<=A, R=A-N } 1 :- 
                        min_cu_id(C,U), class_nactors(C,A,_), A>0.

1 { cu_nactors(U,N,R) : N>=0, N<=n, N<=A-1, N<=N1, N<=R1, R=R1-N } 1 :- 
                        min_cu_id(C,UMin), U1>=UMin, U=U1+1, cu_nactors(U1,N1,R1), 
                        cu_in_class(C,U), cu_in_class(C,U1), class_nactors(C,A,_), A>0.

% if class C has 0 actors (class_nactors(C,0,_)), 
% then all its cus U cu_in_class(C,U) have 0 cu assigned to them (cu_nactors(U,0,0)).
cu_nactors(U,0,0) :- class_nactors(C,0,_), cu_in_class(C,U).

% TODO: check this!
%:- S = #sum{ N,U : cu_nactors(U,N,_), cu_in_class(C,U) }, class_nactors(C,M,_), S < M.
% if U is the maximum id of a class C, then there must be no actors left to assign.
:- cu_nactors(U,_,N), N>0, max_cu_id(C,U). 
% the number of actors assigned to cu is a monotonic decreasing, then
% 1) if U has no actors assigned to it, then there must be no actors left to assign.
:- cu_nactors(U,0,N), N>0, cu(U). 
% 2) if U2>U1, then the number of actors assigned to U2 can't be greater than the number of actors assigned to U1
:- cu_nactors(U1,N1,_), cu_nactors(U2,N2,_), cu_in_class(C,U1), cu_in_class(C,U2), U1>=0, U2=U1+1, cu(U1), cu(U2), N2>N1.
% 2) if U2>U1, then the number of actors N2 assigned to U2 can't be greater than the number of actors left to assign T1
:- cu_nactors(U1,N1,T1), cu_nactors(U2,N2,T2), cu_in_class(C,U1), cu_in_class(C,U2), U1>=0, U2=U1+1, cu(U1), cu(U2), N2>T1.

% assigning actors to cus ------------------------------------------------------
N { run_on(A,U) : actor(A) } N :- cu_nactors(U,N,_).

1 { run_on(A,U) : cu(U) } 1 :- actor(A).

:- cu(U1), cu(U2), U1<U2, cu_in_class(C,U1), cu_in_class(C,U2),
   N1 = #min{ A1 : run_on(A1,U1) }, 
   N2 = #min{ A2 : run_on(A2,U2) }, 
   N1>N2.

%#show class_nactors/3.
#show cu_nactors/3.
%#show run_on/2.
