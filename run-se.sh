#!/bin/bash
export M5_PATH=/home/zhangqianlong/Icache-miss/backup/gem5/fs-images
#./build/X86/gem5.opt  configs/example/fs.py --mem-size=2048MB --disk-image=BigDataBench-gem5-sort.img --kernel=x86_64-vmlinux-2.6.22.9  
#./build/X86/gem5.opt --debug-flags=RubyPort --debug-file=ruby.txt configs/example/se.py -n 4 --ruby -c tests/test-progs/hello/bin/x86/linux/hello
#./build/X86/gem5.debug --debug-flags=Exec,HWPrefetch --debug-file prefetch.txt configs/example/se.py --cpu-type=detailed --caches -c tests/test-progs/hello/bin/x86/linux/hello
./build/X86/gem5.debug --debug-flags=Cache,Exec,Commit --debug-file prefetch.txt configs/example/se.py --cpu-type=detailed --caches --cacheline_size=64  -c tests/test-progs/hello/bin/x86/linux/hello
#./build/X86/gem5.debug --debug-flags=Cache,Exec,HWPrefetch --debug-file stride-prefetch.txt configs/example/se.py --cpu-type=detailed --caches -c tests/test-progs/hello/bin/x86/linux/hello
#./build/ARM/gem5.debug --debug-flags=TLB,Exec,HWPrefetch --debug-file prefetch.txt configs/example/se.py --cpu-type=detailed --caches -c tests/test-progs/hello/bin/arm/linux/hello
    
#ExecEnable, ExecTicks, ExecOpClass, ExecThread, ExecEffAddr, 
#                ExecResult, ExecSymbol, ExecMicro, ExecFaulting, ExecUser, 
#                        ExecKernel


