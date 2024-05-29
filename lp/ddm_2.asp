% actor(s) and message(s)
actor(1..8).

vcu(X) :- actor(X).

% msg_exch_rate(A1,A2,R) holds iff A1 sends A2 R messages per second (msg/s)
msg_exch_rate(1,2,10).
msg_exch_rate(2,3,20).
msg_exch_rate(3,4,10).
msg_exch_rate(4,5,40).
% 
msg_exch_rate(6,7,35).
msg_exch_rate(7,8,20).

% -----
% rules for distributing actor(s) on vcu(s)
% each actor can run on a virtual computing unit only
1 { vcu_of(A,U) : vcu(U) } 1 :- actor(A).
% if A1 and A2 exchange messages with each other, then A1 and A2 must run_on the same vcu 
vcu_of(A2,U) :- related(A1,A2), vcu_of(A1,U), A1!=A2.
% if A1 and A2 do not exchange messages, they belong to different vcu's
:- vcu_of(A2,U), vcu_of(A1,U), not related(A1,A2), A1!=A2.

related(X,Y) :- msg_exch_rate(X,Y,_).
related(X,Z) :- msg_exch_rate(X,Y,_), related(Y,Z).
related(X,Y) :- related(Y,X).

% T is the total number of vcu
tot_vcu(T) :- T = #count{ U : vcu_of(A,U) }.

% utility directives
#show vcu_of/2.
#show tot_vcu/1.