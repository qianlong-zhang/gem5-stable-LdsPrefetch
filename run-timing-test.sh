#!/bin/bash
#export M5_PATH=/home/zhangqianlong/Icache-miss/backup/gem5/fs-images
#./build/X86/gem5.opt  configs/example/fs.py --mem-size=2048MB --disk-image=BigDataBench-gem5-sort.img --kernel=x86_64-vmlinux-2.6.22.9  
./build/X86/gem5.debug configs/example/se.py --caches --l2cache --cpu-type=TimingSimpleCPU -c tests/test-progs/hello/bin/x86/linux/hello


