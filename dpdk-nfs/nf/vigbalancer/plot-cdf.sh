#!/bin/bash

set -e

OUTPUT="$1"
XLABEL="$2"
RANGE="$3"
shift 3

HISTOGRAM=$(mktemp)

GLOBAL_MIN=9223372036854775807 # INT_MAX
GLOBAL_MAX=-9223372036854775808 # INT_MIN

PLOT_LINES="plot"
while (("$#")); do
  CSV="$1"
  TITLE="$2"
  CDF="$CSV.cdf"

  if [ -s "$CSV" ]; then
    if [ "$CSV" -nt "$CDF" ]; then
      echo "Processing $CSV."

      sort -n --parallel=$(nproc) $CSV \
          | uniq -c > $HISTOGRAM

      TOTAL=$(awk '{sum += $1} END {print sum;}' $HISTOGRAM)

      CDFS+=($(mktemp))
      cat $HISTOGRAM | awk "
        BEGIN {
          print \"0,0\";
        }
        {
          print \$2 \",\" (acc / $TOTAL);
          acc += \$1;
          print \$2 \",\" (acc / $TOTAL);
        }" > $CDF
    fi

    MIN=$(sed '2q;d' $CDF | cut -d , -f 1)
    MAX=$(tail -n 1 $CDF | cut -d , -f 1)

    if [ "$MIN" -lt "$GLOBAL_MIN" ]; then
      GLOBAL_MIN="$MIN"
    fi
    if [ "$MAX" -gt "$GLOBAL_MAX" ]; then
      GLOBAL_MAX="$MAX"
    fi

    #PLOT_LINES+=" '$CDF' using 1:(1-\$2) title '$TITLE' with line lw 5,"
    PLOT_LINES+=" '$CDF' using 1:2 title '$TITLE' with line lw 5," 
 else
    echo "No data in $CSV. Skipping."
  fi

  shift 2
done

if [ "$RANGE" == "auto" ]; then
  RANGE="$GLOBAL_MIN:$GLOBAL_MAX"
  echo "Auto range: $RANGE"
fi

echo "Plotting CDF into $OUTPUT."

gnuplot <<EOF
  set ylabel 'CDF'
  set xlabel '$XLABEL'
  set grid
  set xr [$RANGE]
  set yr [0:1]
  set term epscairo
  set output '$OUTPUT'
  set datafile separator ","
  $PLOT_LINES
EOF

rm $HISTOGRAM
