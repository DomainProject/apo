actor(0..n).
cu(0..3).


1 { n_actors_on_cu(0,N,T) : N>=1, N<=n+1, T=(n+1)-N } 1.

1 { n_actors_on_cu(U,N,T) : N>=0, N<=n, N<=N1, N<=T1, T=T1-N } 1 :- 
                           cu(U), cu(U1), U>=1, U=U1+1, n_actors_on_cu(U1,N1,T1).


:- n_actors_on_cu(U,_,N), N>0, U=#max{I : cu(I)}. 

:- n_actors_on_cu(U,0,N), N>0, cu(U). 

:- n_actors_on_cu(U1,N1,T1), n_actors_on_cu(U2,N2,T2), U1>=0, U2=U1+1, cu(U1), cu(U2), N2>N1.

:- n_actors_on_cu(U1,N1,T1), n_actors_on_cu(U2,N2,T2), U1>=0, U2=U1+1, cu(U1), cu(U2), N2>T1.


N { run_on(A,U) : actor(A) } N :- n_actors_on_cu(U,N,_).

1 { run_on(A,U) : cu(U) } 1 :- actor(A).

:- cu(U1), cu(U2), U1<U2, 
   N1 = #min{ A1 : run_on(A1,U1) }, 
   N2 = #min{ A2 : run_on(A2,U2) }, 
   N1>N2.

#show n_actors_on_cu/3.
%#show run_on/2.
