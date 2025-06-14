% run_on(0,0) run_on(1,0) run_on(4,1) run_on(5,1) 
% run_on(6,2) run_on(7,2) run_on(2,3) run_on(3,3)
% a_cc(7,0,5703) a_cc(1,2,5703) a_cc(3,4,5703) a_cc(5,6,5703)
% min_cu_overload(0) max_cu_overload(684)

%%% facts
cu(0..3).
cu_type(0,cpu).
cu_type(1,cpu).
cu_type(2,fpga).
cu_type(3,gpu).
msg_exch_cost(0,0,0).
msg_exch_cost(0,1,1).
msg_exch_cost(0,2,3).
msg_exch_cost(0,3,3).
msg_exch_cost(1,0,1).
msg_exch_cost(1,1,0).
msg_exch_cost(1,2,3).
msg_exch_cost(1,3,3).
msg_exch_cost(2,0,3).
msg_exch_cost(2,1,3).
msg_exch_cost(2,2,0).
msg_exch_cost(2,3,5).
msg_exch_cost(3,0,3).
msg_exch_cost(3,1,3).
msg_exch_cost(3,2,5).
msg_exch_cost(3,3,0).
actor(0..7).
runnable_on(0,7).
runnable_on(1,7).
runnable_on(2,7).
runnable_on(3,7).
runnable_on(4,7).
runnable_on(5,7).
runnable_on(6,7).
runnable_on(7,7).
tasks_forecast(0,22541).
tasks_forecast(1,21861).
tasks_forecast(2,20345).
tasks_forecast(3,21440).
tasks_forecast(4,22546).
tasks_forecast(5,21861).
tasks_forecast(6,20345).
tasks_forecast(7,21435).
cu_capacity(0,45537).
cu_capacity(1,44159).
cu_capacity(2,41096).
cu_capacity(3,43303).
msg_exch_rate(0,0,21601).
msg_exch_rate(0,1,1901).
msg_exch_rate(1,1,21385).
msg_exch_rate(1,2,1901).
msg_exch_rate(2,2,19870).
msg_exch_rate(2,3,1901).
msg_exch_rate(3,3,20727).
msg_exch_rate(3,4,1901).
msg_exch_rate(4,4,21600).
msg_exch_rate(4,5,1901).
msg_exch_rate(5,5,21386).
msg_exch_rate(5,6,1901).
msg_exch_rate(6,6,19871).
msg_exch_rate(6,7,1901).
msg_exch_rate(7,0,1901).
msg_exch_rate(7,7,20723).
mutual_annoyance(0,1,477).
mutual_annoyance(1,2,474).
mutual_annoyance(2,3,712).
mutual_annoyance(3,4,945).
mutual_annoyance(4,5,476).
mutual_annoyance(5,6,474).
mutual_annoyance(6,7,712).
mutual_annoyance(7,0,940).