actor(0..n).
cu(0..3).
cu_class(0).
%cu_class(1).
%cu_in_class(0,0..1).
%cu_in_class(1,2..3).
cu_in_class(0,0..3).

1 { nactors_on_class(0,N,T) : N>=0, N<=n+1, T=(n+1)-N } 1.

1 { nactors_on_class(C,N,T) : N>=0, N<=n+1, T=T1-N, N<=T1 } 1 :- 
                           cu_class(C), cu_class(C1), C>=1, C=C1+1, nactors_on_class(C1,N1,T1).

:- nactors_on_class(C,_,N), N>0, C=#max{I : cu_class(I)}. 

% TODO: check this!
%:- M = #sum{ N : nactors_on_class(C,N,T) }, M != n+1.

% %%%
min_cu_id(C,U) :- U=#min{ I : cu_in_class(C,I) }, cu_class(C).

1 { n_actors_on_cu(U,N,T) : N>=1, N<=n+1, N<=ASum, T=ASum-N } 1 :- 
   min_cu_id(C,U), cu_class(C), nactors_on_class(C,ASum,_), ASum>0.   

1 { n_actors_on_cu(U,N,T) : N>=0, N<=n, N<=ASum-1, N<=N1, N<=T1, T=T1-N } 1 :- 
                           cu(U), cu(U1), min_cu_id(C,UMin), U1>=UMin, U=U1+1, n_actors_on_cu(U1,N1,T1), 
                           cu_in_class(C,U), cu_in_class(C,U1), nactors_on_class(C,ASum,_), ASum>0.

n_actors_on_cu(U,0,0) :- nactors_on_class(C,0,_), cu_in_class(C,U).

% TODO: check this!
%:- S = #sum{ N,U : n_actors_on_cu(U,N,_), cu_in_class(C,U) }, nactors_on_class(C,M,_), S < M.

max_cu_id(C,U) :- U=#max{ I : cu_in_class(C,I) }, cu_class(C).

:- n_actors_on_cu(U,_,N), N>0, max_cu_id(C,U). 

:- n_actors_on_cu(U,0,N), N>0, cu(U). 

:- n_actors_on_cu(U1,N1,T1), n_actors_on_cu(U2,N2,T2), cu_in_class(C,U1), cu_in_class(C,U2), U1>=0, U2=U1+1, cu(U1), cu(U2), N2>N1.

:- n_actors_on_cu(U1,N1,T1), n_actors_on_cu(U2,N2,T2), cu_in_class(C,U1), cu_in_class(C,U2), U1>=0, U2=U1+1, cu(U1), cu(U2), N2>T1.


N { run_on(A,U) : actor(A) } N :- n_actors_on_cu(U,N,_).

1 { run_on(A,U) : cu(U) } 1 :- actor(A).

:- cu(U1), cu(U2), U1<U2, cu_in_class(C,U1), cu_in_class(C,U2),
   N1 = #min{ A1 : run_on(A1,U1) }, 
   N2 = #min{ A2 : run_on(A2,U2) }, 
   N1>N2.

%#show nactors_on_class/3.
#show n_actors_on_cu/3.
%#show run_on/2.
