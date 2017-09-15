sudo insmod kernel_module/npheap.ko
sudo chmod 777 /dev/npheap
rm -f *.log
rm -f *trace*
./benchmark/benchmark 4 8192 4
sleep 5
cat *.log > trace
sort -n -k3 trace > sorted_trace
./benchmark/validate 4 8192
# rm -f *.log
sudo rmmod npheap
