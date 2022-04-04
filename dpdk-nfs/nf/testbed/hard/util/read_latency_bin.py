import sys

ip_file = sys.argv[1]
op_file = sys.argv[2]
metric = sys.argv[3]

endiannes="little" # Please change as per your machine 
data_chunk_size=8  # sizeof(long long) on s2

import numpy as np

def main():

  numbers = list()
  with open(ip_file, 'br') as f:
    data = f.read(data_chunk_size)
    while data:
      if(metric == "latency"):
        number = int.from_bytes(data,endiannes)
      else:
        insns = int.from_bytes(data,endiannes)
        data = f.read(data_chunk_size)
        assert(data)
        cycles = int.from_bytes(data,endiannes)
        number = insns


      numbers.append(number)
      data = f.read(data_chunk_size)

  with open(op_file,'w') as op:
    numbers = np.array(numbers)
    for i in range(101):
      if(metric == "latency"):
        op.write("%d,%d\n" %(i,int(np.percentile(numbers,i))))
      else:
        op.write("%d,%d\n" %(i,np.percentile(numbers,i)))

  
if __name__ == "__main__":
  main()
