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

def load_ground_truth(path):
	res = {}
	f = open(path)
	for line in f.readlines():
		process_line(res, line)
	f.close()
	return res

