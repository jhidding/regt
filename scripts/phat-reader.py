#!/usr/bin/env python3

import sys, numpy as np

def read_phat(fn):
	print("reading ...", fn, file=sys.stderr)
	return np.array([[int(p) for p in line.split()] for line in open(fn)][1:])

def read_alpha_values(fn):
	print("reading ...", fn, file=sys.stderr)
	data = np.array([[float(p) for p in line.split()[:2]] 
		for line in open(fn) 
		if (len(line) > 0 and line[0] != '#')])

	return data[:,0], data[:,1].astype(int)

if __name__ == "__main__":
	file_id = sys.argv[1]
	time = float(sys.argv[2])
	time_str = "%05u" % (time * 10000)

	print("# run: %s -- time: %.4f -- time-str: %s" % (file_id, time, time_str), file=sys.stderr)

	file_name = lambda s: "%s.%s.%s.conan" % (file_id, s, time_str)
	phat_data = read_phat(file_name("phat"))
	alpha, cell_type = read_alpha_values(file_name("alpha"))

	persistence_points = lambda i: alpha[phat_data[np.where(cell_type[phat_data[:,0]] == i)]]

	np.savetxt(file_name("b0"), persistence_points(0))
	np.savetxt(file_name("b1"), persistence_points(1))
	np.savetxt(file_name("b2"), persistence_points(2))

