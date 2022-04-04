mkdir -p instrs/orig
mv pincounts.log instrs/orig/instructions
pushd instrs
  mkdir -p heartbeat
  mkdir -p expired-backend
  mkdir -p good-backend
  mkdir -p new-flow
  csplit -b %04d -z orig/instructions '/New Packet/' '{*}'
  grep HEARTBEAT xx* -cl | xargs -I '{}' mv '{}' heartbeat/
  grep EXP_BACKEND xx* -cl | xargs -I '{}' mv '{}' expired-backend/
  grep GOOD_BACKEND xx* -cl | xargs -I '{}' mv '{}' good-backend/
  find . -type f -name 'xx*' -size -15c -exec rm '{}' \;
  mv xx* new-flow
  wc -l expired-backend/xx* | grep -v total | awk '{print $1}' > expired-backend.counts
  wc -l heartbeat/xx* | grep -v total | awk '{print $1}' > heartbeat.counts
  wc -l good-backend/xx* | grep -v total | awk '{print $1}' > good-backend.counts
  wc -l new-flow/xx* | grep -v total | awk '{print $1}' > new-flow.counts
popd
