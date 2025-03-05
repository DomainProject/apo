import sys
import os

import importlib.util

import metasimulation.SimulationEngine.runtime_modules



def validate_command_line(argv):
    simulation_folder= None
    simulation_trace = None
    fsolutions = None
    wops_string = None

    required_files = ["hardware.py", "global_constants.py"]


    if len(argv) < 3:
        print("Usage: python3 sim_from_trace.py <operations> <simulation folder> [parallel processes]")
        print("Valid values for <operations>: ddm, metis, random, stats")
        sys.exit(1)
    
    wops_string = sys.argv[1]
    simulation_folder=argv[2]

    if not os.path.exists(argv[2]):
        print("Usage: python3 sim_from_trace.py <operations> <simulation folder> [parallel processes]")
        print(f"{simulation_folder} does not exists")
        sys.exit(1)

    simulation_folder_cnts = [f for f in os.listdir(simulation_folder) if os.path.isfile(os.path.join(simulation_folder, f))]

    for f in required_files:
        if f not in simulation_folder_cnts:
            print(f"{simulation_folder} does not contain a '{f}' file")
            sys.exit(1)
        else:
            print(f"{f} found")
            file_path = os.path.join(simulation_folder, f)
            module_name = f.replace('.py', '')
            print(f"loading {f} from {file_path} into module {module_name}")
            spec = importlib.util.spec_from_file_location(module_name, file_path)
            if 'global' in f:
                metasimulation.SimulationEngine.runtime_modules.global_constants_parameter_module = importlib.util.module_from_spec(spec)
                spec.loader.exec_module(metasimulation.SimulationEngine.runtime_modules.global_constants_parameter_module)
            else:
                metasimulation.SimulationEngine.runtime_modules.hardware_parameter_module = importlib.util.module_from_spec(spec)
                spec.loader.exec_module(metasimulation.SimulationEngine.runtime_modules.hardware_parameter_module)
    cnt=0
    for f in simulation_folder_cnts:
        if f[-6:] == '.trace':
            cnt += 1
            simulation_trace = os.path.join(simulation_folder, f)
            fsolutions = simulation_trace+".solutions"

    if cnt == 0:
        print(f"{simulation_folder} does not contain a '.trace' file")
        sys.exit(1)
    if cnt > 1:
        print(f"{simulation_folder} contains multiple '.trace' files")
        sys.exit(1)

    print(f"trace found ({simulation_trace})")

    return simulation_folder, simulation_trace, fsolutions, wops_string





