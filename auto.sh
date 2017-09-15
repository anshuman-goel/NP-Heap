sudo rmmod npheap
cd kernel_module
make
sudo make install
# sudo insmod npheap.ko
# sudo chmod 777 /dev/npheap
cd ../library
make
sudo make install
cd ../benchmark
make
