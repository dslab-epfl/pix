bash plot-cdf.sh instrs/heartbeat.eps 'Instructions' 0:4000 instrs/heartbeat.counts "Heartbeat processing"
bash plot-cdf.sh instrs/exp-backend.eps 'Instructions' 0:4000 instrs/expired-backend.counts "Existing flow, expired backend" 
bash plot-cdf.sh instrs/good-backend.eps 'Instructions' 0:4000 instrs/good-backend.counts "Existing flow, existing backend" 
bash plot-cdf.sh instrs/new-flow.eps 'Instructions' 0:4000 instrs/new-flow.counts "New flow" 
#find . -name '*.eps' -exec epstopdf '{}' \;
