hexdump -v -e '16/1 "_x%02X" "\n"' $1 | sed 's/_/\\/g; s/\\x  //g; s/.*/    "&"/'
