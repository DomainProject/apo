actor(0..n).
%run_on_class(0..n,c1).
%run_on_class(32..63,c2).
cu(0..3).
%cu_class(0..7,c1).
%cu_class(32..63,c2).

num(0..n+1).

1 { dc(U1,U2,U3,U4) : U1+U2+U3+U4 = n+1, U1 >= U2, U2 >= U3, U3 >= U4, num(U1), num(U2), num(U3), num(U4) } 1.

actors_on_cu(N,0) :- dc(N,_,_,_).
actors_on_cu(N,1) :- dc(_,N,_,_).
actors_on_cu(N,2) :- dc(_,_,N,_).
actors_on_cu(N,3) :- dc(_,_,_,N). 

N { run_on(A,U) : actor(A) } N :- cu(U), actors_on_cu(N,U).

:- cu(U1), cu(U2), U1<U2, 
   N1 = #min{ A1 : run_on(A1,U1) }, 
   N2 = #min{ A2 : run_on(A2,U2) }, 
   N1>N2.


1 { run_on(A,U) : min_ia_on_cu(A1,U), A>=A1, cu(U) } 1 :- actor(A).

run_on(A,U) :- min_ia_on_cu(A,U), actor(A).

vid(42,0) :- actors_on_cu(N,0), N=0.
vid(43,1) :- actors_on_cu(N,1), N=0.
vid(44,2) :- actors_on_cu(N,2), N=0.
vid(45,3) :- actors_on_cu(N,3), N=0.
vid(A,U) :- actor(A), actors_on_cu(N,U), N>0.


1 { ia(A1,A2,A3,A4) :  A1 < A2, A2 < A3, A3 < A4, %actor(A1), actor(A2), actor(A3), actor(A4),
                      vid(A1,0), vid(A2,1), vid(A3,2), vid(A4,3) } 1.

min_ia_on_cu(N,0) :- ia(N,_,_,_).
min_ia_on_cu(N,1) :- ia(_,N,_,_).
min_ia_on_cu(N,2) :- ia(_,_,N,_).
min_ia_on_cu(N,3) :- ia(_,_,_,N). 

%#show ia/4.
%#show dc/4.
#show run_on/2.
