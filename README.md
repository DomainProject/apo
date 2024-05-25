# src

Actors are represented by `actor(A)` facts, 
where `A` is an actor identifier.

Computing units are represented similarly by `cu(U)` facts,
where `U` is a computing unit identifier (such as CPU cores and GPUs).

Each computing unit `U`

- has a **type** represented by `cu_type(U,T)`,
where `T` is the computing unit type (e.g., `cu_type(1,gpu)`), and 
- has a **throughput** represented by `throughput(U,W)`,
where `W` is the computing unit throughtput measured as *tasks for time window* (t/tw).

Each actor `A` 
- has a workload `W` represented by `workload(A,W)`,
measured as *tasks for time window* (t/tw), and
- its code can be executed on **at least one** computing unit of type `T`,
i.e., `runnable_on(A,T)`.

The messages exchanged between actors is represented by a **strictly upper triangular matrix**, encoded by the following predicate.

`msg_exch_rate(A1,A2,M)`

holds iff `A1` and `A2` exchanges `M` messages per time window (msg/tw).






