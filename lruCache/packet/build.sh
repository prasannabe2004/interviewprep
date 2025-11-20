rm -rf build
mkdir -p build
cd build
# optionally: cmake -DCAPACITY=16 -DMAX_PACKET_SIZE=512 ..
cmake ..
cmake --build . -- -j$(nproc)
./lru_test
