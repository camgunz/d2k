rm *.prof
CPUPROFILE=prb.prof cbuild/prboom-plus -playdemo DEMO3
google-pprof --gv cbuild/prboom-plus prb.prof
