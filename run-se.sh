#!/bin/bash
export M5_PATH=/home/zhangqianlong/Icache-miss/backup/gem5/fs-images
#./build/X86/gem5.opt  configs/example/fs.py --mem-size=2048MB --disk-image=BigDataBench-gem5-sort.img --kernel=x86_64-vmlinux-2.6.22.9  
#./build/X86/gem5.opt --debug-flags=RubyPort --debug-file=ruby.txt configs/example/se.py -n 4 --ruby -c tests/test-progs/hello/bin/x86/linux/hello
./build/X86/gem5.opt --debug-flags=TLB,PageTableWalker --debug-file=PageTableWalker.txt configs/example/se.py -n 4 --ruby -c tests/test-progs/hello/bin/x86/linux/hello


#./build/X86/gem5.opt configs/example/fs.py --disk-image=BigDataBench-gem5-sort.img   --kernel=/home/zhangqianlong/Icache-miss/Qemu_icache/vmlinux-new
#./build/X86/gem5.opt configs/example/fs.py --disk-image=/home/zhangqianlong/Icache-miss/Qemu_icache/ubuntu-14.img   --kernel=/home/zhangqianlong/Icache-miss/Qemu_icache/vmlinux-new
#./build/X86/gem5.opt configs/example/fs.py --disk-image=BigDataBench-gem5-sort.img  --kernel=/home/zhangqianlong/Icache-miss/Qemu_icache/vmlinux-new


#L1 i size =256kB ok
#./build/X86/gem5.opt configs/example/fs.py --cpu-type=timing --caches --l1d_size=64kB --l1i_size=256kB --l2cache --mem-size=2048MB --disk-image=BigDataBench-gem5-sort.img --kernel=x86_64-vmlinux-2.6.22.9 
#./build/X86/gem5.opt configs/example/fs.py --disk-image=BigDataBench-gem5-sort.img --kernel=x86_64-vmlinux-2.6.22.9 

#Xeon nomal
#./build/X86/gem5.opt --stats-file=Xeon-nomal  configs/example/fs.py --cpu-type=timing --caches --l1d_size=32kB --l1d_assoc=8 --l1i_size=32kB --l1i_assoc=4 --l2cache --l2_size=256kB --l2_assoc=8 --l3_size=12288kB --l3_assoc=16 --cacheline_size=64  --mem-size=2048MB --disk-image=BigDataBench-gem5-sort-larger.img --kernel=x86_64-vmlinux-2.6.22.9 
#./build/X86/gem5.opt --stats-file=Xeon-128kBicache configs/example/fs.py --cpu-type=timing --caches --l1d_size=32kB --l1d_assoc=8 --l1i_size=128kB --l1i_assoc=4 --l2cache --l2_size=256kB --l2_assoc=8 --l3_size=12288kB --l3_assoc=16 --cacheline_size=64  --mem-size=2048MB --disk-image=BigDataBench-gem5-sort.img --kernel=x86_64-vmlinux-2.6.22.9 

#./build/X86/gem5.opt --stats-file=Xeon-nomal  configs/example/fs.py --cpu-type=timing --caches --l1d_size=32kB --l1d_assoc=8 --l1i_size=32kB --l1i_assoc=4 --l2cache --l2_size=256kB --l2_assoc=8 --l3_size=12288kB --l3_assoc=16 --cacheline_size=64  --mem-size=2048MB --disk-image=20g.img --kernel=x86_64-vmlinux-2.6.22.9 

#Xeon nomal deatiled L2 cache=512M
#./build/X86/gem5.opt --stats-file=Xeon-OoO  configs/example/fs.py --cpu-type=detailed --caches --l1d_size=32kB --l1d_assoc=8 --l1i_size=32kB --l1i_assoc=4 --l2cache --l2_size=524288kB --l2_assoc=8 --cacheline_size=64 --mem-size=16384MB --disk-image=BigDataBench-gem5-sort.img --kernel=x86_64-vmlinux-2.6.22.9 

#Xeon nomal deatiled nomal ruby
#./build/X86/gem5.opt --stats-file=Xeon-OoO  configs/example/fs.py --cpu-type=detailed --caches --l1d_size=32kB --l1d_assoc=8 --l1i_size=32kB --l1i_assoc=4 --l2cache --l2_size=256kB --l2_assoc=8 --l3_size=12288kB --l3_assoc=16 --cacheline_size=64 --ruby  --mem-size=2048MB --disk-image=BigDataBench-gem5-sort.img --kernel=x86_64-vmlinux-2.6.22.9 
#./build/X86/gem5.opt --stats-file=Xeon-OoO  configs/example/fs.py --fast-forward=4245334010  --cpu-type=detailed --caches --l1d_size=32kB --l1d_assoc=8 --l1i_size=32kB --l1i_assoc=4 --l2cache --l2_size=256kB --l2_assoc=8 --l3_size=12288kB --l3_assoc=16 --cacheline_size=64  --mem-size=2048MB --disk-image=BigDataBench-gem5-sort.img --kernel=x86_64-vmlinux-2.6.22.9 
#./build/X86/gem5.opt --stats-file=Xeon-OoO128  configs/example/fs.py --fast-forward=9245334010  --cpu-type=detailed --caches --l1d_size=32kB --l1d_assoc=8 --l1i_size=128kB --l1i_assoc=4 --l2cache --l2_size=256kB --l2_assoc=8 --l3_size=12288kB --l3_assoc=16 --cacheline_size=64  --mem-size=2048MB --disk-image=BigDataBench-gem5-sort.img --kernel=x86_64-vmlinux-2.6.22.9 


#20g
#./build/X86/gem5.opt --stats-file=Xeon-nomal configs/example/fs.py --cpu-type=timing --caches --l1d_size=32kB --l1d_assoc=8 --l1i_size=32kB --l1i_assoc=4 --l2cache --l2_size=256kB --l2_assoc=8 --l3_size=12288kB --l3_assoc=16 --cacheline_size=64 --ruby --mem-size=2048MB --disk-image=20g.img --kernel=x86_64-vmlinux-2.6.22.9







#that is ok before, but now is panic
#./build/X86/gem5.opt --stats-file=Xeon-OoO configs/example/fs.py --cpu-type=detailed --caches --l1d_size=32kB --l1d_assoc=8 --l1i_size=32kB --l1i_assoc=4 --l2cache --l2_size=256kB --l2_assoc=8 --l3_size=12288kB --l3_assoc=16 --cacheline_size=64 --ruby --mem-size=2048MB --disk-image=BigDataBench-gem5-sort.img --kernel=x86_64-vmlinux-2.6.22.9
