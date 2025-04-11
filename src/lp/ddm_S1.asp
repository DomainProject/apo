actor(0..n).
run_on_class(0..n,c1).
%run_on_class(32..63,c2).
cu(0..3).
cu_class(0..3,c1).
%cu_class(32..63,c2).

1 {  run_on(A,U) : run_on_class(A,C), cu_class(U,C) } 1 :- actor(A).

:- cu(U1), cu(U2), U1<U2, 
   cu_class(U1,C), cu_class(U2,C), 
   N1 = #count{ A1 : run_on(A1,U1) }, 
   N2 = #count{ A2 : run_on(A2,U2) }, 
   N1<N2.

:- cu(U1), cu(U2), U1<U2, 
   cu_class(U1,C), cu_class(U2,C), 
   N1 = #min{ A1 : run_on(A1,U1) }, 
   N2 = #min{ A2 : run_on(A2,U2) }, 
   N1>N2.

#show run_on/2.