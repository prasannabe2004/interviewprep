
#dependecies
sudo apt install liburing-dev pkg-config

#compile
gcc ./cat_io_uring.c -luring -o cat.out

#4GB file
dd if=/dev/zero of=~/100MB.file bs=1024 count=10M

#Run
./cat.out ~/100MB.file ~/100MB.file ~/100MB.file  ~/100MB.file ~/100MB.file ~/100MB.file