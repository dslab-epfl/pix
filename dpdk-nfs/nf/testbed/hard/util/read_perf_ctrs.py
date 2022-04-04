import sys

ip_file = sys.argv[1]
op_file = sys.argv[2]
perf_ctr = str(sys.argv[3])

import numpy as np

def get_perf_ctr_column(ctr):
  if(ctr == "PAPI_REF_CYC"):
    return 0
  if(ctr == "PAPI_TOT_INS"):
    return 1
  if(ctr == "PAPI_L1_ICM"):
    return 2
  if(ctr == "PAPI_L1_DCM"):
    return 3
  if(ctr == "PAPI_L2_TCM"):
    return 4
  if(ctr == "PAPI_L3_TCM"):
    return 5
  return 0


def main():

  column_num = get_perf_ctr_column(perf_ctr)
  assert(column_num >=0 and "Invalid perf counter")
  numbers = list()
  with open(ip_file, 'r') as f:
    numbers = [int(round(float(line.strip().split(" ")[column_num]))) for line in f]

  with open(op_file,'w') as op:
    numbers = np.array(numbers)
    for i in range(101):
      op.write("%d,%d\n" %(i,int(np.percentile(numbers,i))))
  
if __name__ == "__main__":
  main()