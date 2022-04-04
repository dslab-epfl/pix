make clean
make
sudo ~/pin/pin -t ../pin/counts.so -- ./build/lb -c 0x01 -n 2 -- --flow-expiration 3600 --flow-capacity 65536 --backend-capacity 128 --cht-height 293 --backend-expiration 10
