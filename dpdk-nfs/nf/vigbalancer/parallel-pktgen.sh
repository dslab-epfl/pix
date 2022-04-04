CLIENT_REQUESTS=no-hash-collisions50k.pcap
BACKEND_HEARTBEATS=heartbeat2.pcap
(for i in `seq 5`; do sudo tcpreplay -i p802p2 heartbeat1.pcap && sleep 5; done) &
(for i in `seq 2`; do sudo tcpreplay -i p802p2 --pps=40 $BACKEND_HEARTBEATS &&
  sudo tcpreplay -i p802p2 --pps=64 $BACKEND_HEARTBEATS  && sleep 11; done) &
sudo tcpreplay -i p802p1 --pps=4000 --loop=2 $CLIENT_REQUESTS
