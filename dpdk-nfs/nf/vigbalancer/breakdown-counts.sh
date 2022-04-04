mkdir -p cycles
mv pptimes.txt cycles/
pushd cycles
  grep 'class 1' pptimes.txt | sed 's/Time for class .* is \(.*\)/\1/' > heartbeat.txt
  grep 'class 8' pptimes.txt | sed 's/Time for class .* is \(.*\)/\1/' >> heartbeat.txt
  grep 'class 9' pptimes.txt | sed 's/Time for class .* is \(.*\)/\1/' >> heartbeat.txt
  grep 'class 10' pptimes.txt | sed 's/Time for class .* is \(.*\)/\1/' >> heartbeat.txt
  grep 'class 11' pptimes.txt | sed 's/Time for class .* is \(.*\)/\1/' >> heartbeat.txt
  bash ../plot-cdf.sh heartbeat.eps 'Processing time, ns' 0:1500 heartbeat.txt "Heartbeat processing"

  grep 'class 6' pptimes.txt | sed 's/Time for class .* is \(.*\)/\1/' > expired-backend.txt
  bash ../plot-cdf.sh exp-backend.eps 'Processing time, ns' 0:1500 expired-backend.txt "Existing flow, expired backend" 

  grep 'class 7' pptimes.txt | sed 's/Time for class .* is \(.*\)/\1/' > good-backend.txt
  bash ../plot-cdf.sh good-backend.eps 'Processing time, ns' 0:1500 good-backend.txt "Existing flow, existing backend" 

  grep 'class 2' pptimes.txt | sed 's/Time for class .* is \(.*\)/\1/' > new-flow.txt
  grep 'class 3' pptimes.txt | sed 's/Time for class .* is \(.*\)/\1/' >> new-flow.txt
  grep 'class 4' pptimes.txt | sed 's/Time for class .* is \(.*\)/\1/' >> new-flow.txt
  bash ../plot-cdf.sh new-flow.eps 'Processing time, ns' 0:1500 new-flow.txt "New flow" 
popd
