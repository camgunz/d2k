rm *.prof
CPUPROFILE=d2k.prof cbuild/doom2k -playdemo DEMO3
google-pprof --gv cbuild/doom2k d2k.prof
