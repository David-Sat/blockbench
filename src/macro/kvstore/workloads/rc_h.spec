# Yahoo! Cloud System Benchmark
# Workload A: Update heavy workload
#   Application example: Session store recording recent actions
#                        
#   Read/update ratio: 50/50
#   Default data size: 1 KB records (10 fields, 100 bytes each, plus key)
#   Request distribution: zipfian

recordcount=1400
operationcount=100000
workload=ycsb

readallfields=true

readproportion=0.7
updateproportion=0.3
scanproportion=0
insertproportion=0

requestdistribution=zipfian

