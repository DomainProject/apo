# src

Actors are represented by `actor(A)` facts, 
where `A` is an actor identifier.

Computing units are represented similarly by `cu(U)` facts,
where `U` is a computing unit identifier (such as CPU cores and GPUs).

Each computing unit `U`

- has a **type** represented by `cu_type(U,T)`,
where `T` is the computing unit type (e.g., `cu_type(1,gpu)`), and 
- has a **throughput** represented by `throughput(U,W)`,
where `W` is the computing unit throughput measured as *tasks for time window* (t/tw).

Each actor `A` 
- has a workload `W` represented by `workload(A,W)`,
measured as *tasks for time window* (t/tw), and
- its code can be executed on **at least one** computing unit of type `T`,
i.e., `runnable_on(A,T)`.

Messages exchanged between actors are represented by a **strictly upper triangular matrix**, encoded by `msg_exch_rate(A1,A2,M)`,
where: `A1` and `A2` are actors, and `M` is the number of messages per time window (msg/tw) exchanged between `A1` and `A2`.
Exchanging messages has a cost that depends on the type of the computing unit, encoded by `msg_exch_cost(U1,U2,C)`.

![](./parallelism.jpg)

`mutual_annoyance(A1,A2,V)`: quanto due agenti si danno fastidio a vicenda. 

TODO: Da gestire (a livello di Runtime Monitoring module) con una media esponenziale (?).

TODO: migration_cost(U1,U1) costo di migrazione tra due cu
