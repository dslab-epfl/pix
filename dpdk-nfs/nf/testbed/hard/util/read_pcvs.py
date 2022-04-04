import sys

ip_file = sys.argv[1]
op_file = sys.argv[2]

import numpy as np

def main():

  numbers = list()
  with open(ip_file, 'r') as f:
    numbers = [int(line.strip()) for line in f]

  with open(op_file,'w') as op:
    numbers = np.array(numbers)
    for i in range(101):
      op.write("%d,%d\n" %(i,int(np.percentile(numbers,i))))
  
if __name__ == "__main__":
  main()