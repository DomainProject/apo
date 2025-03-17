from metasimulation.SimulationEngine import sim as Simulator
from metasimulation.SimulationModel.event_handlers import EVT
from metasimulation.SimulationModel.hardware import get_dev_from_cu


def assignment_renaming(assignment):
  new_assignment = []
  translation = {}
  used_labels = set([])

  for i in assignment:
    if i not in translation:
      dev = i.split('_')[0]
      count = 0
      while True:
        new_label = f"{dev}_{count}"
        if new_label not in used_labels:
          used_labels.add(new_label)
          translation[i] = new_label
          break
        count += 1
    new_assignment += [translation[i]]
  return new_assignment

def check_and_install_new_binding(operations, wct_ts, maximum_th, ground_truth, sim_state):
    binding = operations.delayed_on_window()
    if binding is None: return False, False

    old_bind = binding
    binding = assignment_renaming(binding)
    print(f"new binding: {old_bind} renamed below")
    if len(ground_truth) > 0:
        print(f"new binding: {binding} expected th {ground_truth[tuple(binding)]} vs max th {maximum_th}")
    new_bid=False
    if binding != sim_state.get_assignment() and sim_state._pending_assigment != binding:
        new_bid=True
        sim_state.put_pending_assignment(binding)
        to_updated = set([])
        for i in range(len(sim_state._assignment)):
            if sim_state._pending_assignment[i] != sim_state._assignment[i]:
                to_updated.add(sim_state._assignment[i])
        for cu in to_updated:
            Simulator.schedule_event(wct_ts, cu, EVT.REASSIGN)
    return True, new_bid

def filter_assignment(assignment):
    d = {}
    for cu in assignment:
        dev = get_dev_from_cu(cu)
        if dev not in d: d[dev] = []
        d[dev] += [int(cu.replace(dev+"_", ""))]

    for dev in d:
        used_id = set([])
        order_appearence = []
        for id in d[dev]:
            if id not in used_id:
                order_appearence += [id]
                used_id.add(id)

        if len(order_appearence) == 1:
            if order_appearence[0] != 0:
                return True
        else:
            for i in range(len(order_appearence)-1):
                if order_appearence[i] > order_appearence[i+1]:
                    #print(f"skipping {assignment}")
                    return True

    return False

def estimate_filter_reduction(units):
    d = {}
    for cu in units:
        dev = get_dev_from_cu(cu)
        if dev not in d: d[dev] = []
        d[dev] += [int(cu.replace(dev+"_", ""))]
    cnt = 1
    for dev in d:
        tmp = len(d[dev])
        factorial = 1
        while tmp > 1:
            factorial *= tmp
            tmp-=1
        cnt *= factorial
    return cnt
