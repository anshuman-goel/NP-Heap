# sudo insmod kernel_module/npheap.ko
# sudo chmod 777 /dev/npheap
./benchmark/benchmark 64 64 4
cat *.log > trace
sort -n -k3 trace > sorted_trace
./benchmark/validate 64 < sorted_trace
rm -f *.log
# sudo rmmod npheap
