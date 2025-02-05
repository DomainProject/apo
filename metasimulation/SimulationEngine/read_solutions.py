import os

def process_line(res, line):
	line = line.strip()
	line = line.replace('\x00', '')
	line = line.replace(" ", "")
	line = line.replace(')', '')
	line = line.replace('(', '')
	line = line.replace("'", "")
	th = line.split(',')[-1]
	
	line = line.split(',')[:-1]
	tmp  = []
	for l in line: tmp += [l.strip()] 
	line = tuple(line)
	res[line] = th

def load_ground_truth_file(path):
	print(f"loading ground truth {path}...", end='')
	res = {}
	f = open(path)
	for line in f.readlines():
		process_line(res, line)
	f.close()
	print('end')
	return res



def load_ground_truth(fsolutions):
	maximum_th = 0
	ground_truth = {}
	if os.path.isfile(fsolutions):
	    ground_truth = load_ground_truth_file(fsolutions)
	    for k in ground_truth:
	        maximum_th = max(maximum_th, float(ground_truth[k]))
	if maximum_th != 0: print(f"Found optimal solution {maximum_th}")
	return maximum_th, ground_truth