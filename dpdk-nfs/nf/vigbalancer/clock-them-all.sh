make clean
make
sudo taskset -c 8 ./build/lb -c 0x01 -n 2 -- --flow-expiration 3600 --flow-capacity 65536 --backend-capacity 2 --cht-height 3 --backend-expiration 10 | tee pptimes.txt
