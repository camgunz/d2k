rm *.prof
env LD_PRELOAD=/usr/lib/libprofiler.so CPUPROFILE=/home/charlie/Documents/Code/d2k/d2k.prof cbuild/doom2k -file sunder.wad -warp 6 -deltaserve
google-pprof --gv cbuild/doom2k d2k.prof
