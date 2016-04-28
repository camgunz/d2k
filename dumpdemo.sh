#!/bin/sh

DEMODIR=/home/charlie/demos

# PORT='/home/charlie/bin/prboom-plus -nosfx -nomusic -nomouse -nodraw'
# DEMOCMD=fastdemo

# PORT=/home/charlie/bin/odamex
# DEMOCMD=playdemo

PORT='/home/charlie/code/d2k/cbuild/d2k -nosfx -nomusic -nomouse -nodraw'
DEMOCMD=fastdemo
DUMPDIR='/home/charlie/code/democomp/d2k'

# ${PORT} -iwad doom2.wad -file chextc.wad -deh chex.deh -${DEMOCMD} demo1 -dumpdemo ${DUMPDIR}/chex-demo1.bin
# ${PORT} -iwad doom2.wad -file chextc.wad -deh chex.deh -${DEMOCMD} demo2 -dumpdemo ${DUMPDIR}/chex-demo2.bin
# ${PORT} -iwad doom2.wad -file chextc.wad -deh chex.deh -${DEMOCMD} demo3 -dumpdemo ${DUMPDIR}/chex-demo3.bin

# ${PORT} -iwad doom.wad -${DEMOCMD} ${DEMODIR}/bohfight.lmp -dumpdemo ${DUMPDIR}/doom-bohfight.bin
# ${PORT} -iwad doom.wad -${DEMOCMD} ${DEMODIR}/d1shock.lmp -dumpdemo ${DUMPDIR}/doom-d1shock.bin
# ${PORT} -iwad doom.wad -${DEMOCMD} ${DEMODIR}/d1slide.lmp -dumpdemo ${DUMPDIR}/doom-d1slide.bin
# ${PORT} -iwad doom.wad -${DEMOCMD} demo1 -dumpdemo ${DUMPDIR}/doom-demo1.bin
# ${PORT} -iwad doom.wad -${DEMOCMD} demo2 -dumpdemo ${DUMPDIR}/doom-demo2.bin
# ${PORT} -iwad doom.wad -${DEMOCMD} demo3 -dumpdemo ${DUMPDIR}/doom-demo3.bin
# ${PORT} -iwad doom.wad -${DEMOCMD} ${DEMODIR}/impfight.lmp -dumpdemo ${DUMPDIR}/doom-impfight.bin
# ${PORT} -iwad doom.wad -${DEMOCMD} ${DEMODIR}/speedbmp.lmp -dumpdemo ${DUMPDIR}/doom-speedbmp.bin
# ${PORT} -iwad doom.wad -${DEMOCMD} ${DEMODIR}/stilhere.lmp -dumpdemo ${DUMPDIR}/doom-stilhere.bin
# ${PORT} -iwad doom.wad -${DEMOCMD} ${DEMODIR}/stuckimp.lmp -dumpdemo ${DUMPDIR}/doom-stuckimp.bin
# ${PORT} -iwad doom.wad -${DEMOCMD} ${DEMODIR}/tapfocus.lmp -dumpdemo ${DUMPDIR}/doom-tapfocus.bin
# ${PORT} -iwad doom.wad -${DEMOCMD} ${DEMODIR}/wallrun.lmp -dumpdemo ${DUMPDIR}/doom-wallrun.bin
# 
# ${PORT} -iwad doom2.wad -${DEMOCMD} ${DEMODIR}/02noview.lmp -dumpdemo ${DUMPDIR}/doom2-02noview.bin
# ${PORT} -iwad doom2.wad -${DEMOCMD} ${DEMODIR}/07intowr.lmp -dumpdemo ${DUMPDIR}/doom2-07intowr.bin
# ${PORT} -iwad doom2.wad -${DEMOCMD} ${DEMODIR}/07tower.lmp -dumpdemo ${DUMPDIR}/doom2-07tower.bin
# ${PORT} -iwad doom2.wad -${DEMOCMD} ${DEMODIR}/2ghosts.lmp -dumpdemo ${DUMPDIR}/doom2-2ghosts.bin
# ${PORT} -iwad doom2.wad -${DEMOCMD} demo1 -dumpdemo ${DUMPDIR}/doom2-demo1.bin
# ${PORT} -iwad doom2.wad -${DEMOCMD} demo2 -dumpdemo ${DUMPDIR}/doom2-demo2.bin
# ${PORT} -iwad doom2.wad -${DEMOCMD} demo3 -dumpdemo ${DUMPDIR}/doom2-demo3.bin
# ${PORT} -iwad doom2.wad -${DEMOCMD} ${DEMODIR}/bfgdemo.lmp -dumpdemo ${DUMPDIR}/doom2-bfgdemo.bin
# ${PORT} -iwad doom2.wad -${DEMOCMD} ${DEMODIR}/bounce1.lmp -dumpdemo ${DUMPDIR}/doom2-bounce1.bin
# ${PORT} -iwad doom2.wad -${DEMOCMD} ${DEMODIR}/bounce2.lmp -dumpdemo ${DUMPDIR}/doom2-bounce2.bin
# ${PORT} -iwad doom2.wad -${DEMOCMD} ${DEMODIR}/burnthru.lmp -dumpdemo ${DUMPDIR}/doom2-burnthru.bin
# ${PORT} -iwad doom2.wad -${DEMOCMD} ${DEMODIR}/cacofit2.lmp -dumpdemo ${DUMPDIR}/doom2-cacofit.bin
# ${PORT} -iwad doom2.wad -${DEMOCMD} ${DEMODIR}/cacofite.lmp -dumpdemo ${DUMPDIR}/doom2-cacofite.bin
# ${PORT} -iwad doom2.wad -${DEMOCMD} ${DEMODIR}/d2shock.lmp -dumpdemo ${DUMPDIR}/doom2-d2shock.bin
# ${PORT} -iwad doom2.wad -${DEMOCMD} ${DEMODIR}/d2slide.lmp -dumpdemo ${DUMPDIR}/doom2-d2slide.bin
# ${PORT} -iwad doom2.wad -${DEMOCMD} ${DEMODIR}/dedngone.lmp -dumpdemo ${DUMPDIR}/doom2-dedngone.bin
# ${PORT} -iwad doom2.wad -${DEMOCMD} ${DEMODIR}/firewall.lmp -dumpdemo ${DUMPDIR}/doom2-firewall.bin
# ${PORT} -iwad doom2.wad -${DEMOCMD} ${DEMODIR}/flicker.lmp -dumpdemo ${DUMPDIR}/doom2-flicker.bin
# ${PORT} -iwad doom2.wad -${DEMOCMD} ${DEMODIR}/fusedem.lmp -dumpdemo ${DUMPDIR}/doom2-fusedem.bin
# ${PORT} -iwad doom2.wad -${DEMOCMD} ${DEMODIR}/ghostpe1.lmp -dumpdemo ${DUMPDIR}/doom2-ghostpe1.bin
# ${PORT} -iwad doom2.wad -${DEMOCMD} ${DEMODIR}/ghostpe2.lmp -dumpdemo ${DUMPDIR}/doom2-ghostpe2.bin
# ${PORT} -iwad doom2.wad -${DEMOCMD} ${DEMODIR}/impghost.lmp -dumpdemo ${DUMPDIR}/doom2-impghost.bin
# ${PORT} -iwad doom2.wad -${DEMOCMD} ${DEMODIR}/impmiss.lmp -dumpdemo ${DUMPDIR}/doom2-impmiss.bin
# ${PORT} -iwad doom2.wad -${DEMOCMD} ${DEMODIR}/lv04-028.lmp -dumpdemo ${DUMPDIR}/doom2-lv04-028.bin
# ${PORT} -iwad doom2.wad -${DEMOCMD} ${DEMODIR}/lv07-046.lmp -dumpdemo ${DUMPDIR}/doom2-lv07-046.bin
# ${PORT} -iwad doom2.wad -${DEMOCMD} ${DEMODIR}/lv12-043.lmp -dumpdemo ${DUMPDIR}/doom2-lv12-043.bin
# ${PORT} -iwad doom2.wad -${DEMOCMD} ${DEMODIR}/lv15s032.lmp -dumpdemo ${DUMPDIR}/doom2-lv15s032.bin
# ${PORT} -iwad doom2.wad -${DEMOCMD} ${DEMODIR}/revorbit.lmp -dumpdemo ${DUMPDIR}/doom2-revorbit.bin
# ${PORT} -iwad doom2.wad -${DEMOCMD} ${DEMODIR}/smpunch.lmp -dumpdemo ${DUMPDIR}/doom2-smpunch.bin
# ${PORT} -iwad doom2.wad -${DEMOCMD} ${DEMODIR}/soulpop.lmp -dumpdemo ${DUMPDIR}/doom2-soulpop.bin
# ${PORT} -iwad doom2.wad -${DEMOCMD} ${DEMODIR}/tallbomb.lmp -dumpdemo ${DUMPDIR}/doom2-tallbomb.bin
# ${PORT} -iwad doom2.wad -${DEMOCMD} ${DEMODIR}/thingrun.lmp -dumpdemo ${DUMPDIR}/doom2-thingrun.bin
# ${PORT} -iwad doom2.wad -file av.wad -${DEMOCMD} demo1 -dumpdemo ${DUMPDIR}/doom2-av-demo1.bin
# ${PORT} -iwad doom2.wad -file av.wad -${DEMOCMD} demo2 -dumpdemo ${DUMPDIR}/doom2-av-demo2.bin
# ${PORT} -iwad doom2.wad -file av.wad -${DEMOCMD} demo3 -dumpdemo ${DUMPDIR}/doom2-av-demo3.bin
${PORT} -iwad doom2.wad -file gothicdm.wad -${DEMOCMD} demo1 -dumpdemo ${DUMPDIR}/doom2-gothicdm-demo1.bin
${PORT} -iwad doom2.wad -file gothicdm.wad -${DEMOCMD} demo2 -dumpdemo ${DUMPDIR}/doom2-gothicdm-demo2.bin
# ${PORT} -iwad doom2.wad -file gothicdm.wad -${DEMOCMD} demo3 -dumpdemo ${DUMPDIR}/doom2-gothicdm-demo3.bin
# ${PORT} -iwad doom2.wad -file gothicdm.wad -${DEMOCMD} demo4 -dumpdemo ${DUMPDIR}/doom2-gothicdm-demo4.bin
# ${PORT} -iwad doom2.wad -file hr.wad -${DEMOCMD} ${DEMODIR}/hf21tysn.lmp -dumpdemo ${DUMPDIR}/doom2-hr-hf21tysn.bin
# ${PORT} -iwad doom2.wad -file hr2final.wad -${DEMOCMD} ${DEMODIR}/hr2-lv01.lmp -dumpdemo ${DUMPDIR}/doom2-hr2final-hr2-lv01.bin
# ${PORT} -iwad doom2.wad -file icarus.wad -${DEMOCMD} demo1 -dumpdemo ${DUMPDIR}/doom2-icarus-demo1.bin
# ${PORT} -iwad doom2.wad -file icarus.wad -${DEMOCMD} demo2 -dumpdemo ${DUMPDIR}/doom2-icarus-demo2.bin
# ${PORT} -iwad doom2.wad -file icarus.wad -${DEMOCMD} demo3 -dumpdemo ${DUMPDIR}/doom2-icarus-demo3.bin
# ${PORT} -iwad doom2.wad -file mm2.wad -${DEMOCMD} demo1 -dumpdemo ${DUMPDIR}/doom2-mm2-demo1.bin
# ${PORT} -iwad doom2.wad -file mm2.wad -${DEMOCMD} demo2 -dumpdemo ${DUMPDIR}/doom2-mm2-demo2.bin
# ${PORT} -iwad doom2.wad -file mm2.wad -${DEMOCMD} demo3 -dumpdemo ${DUMPDIR}/doom2-mm2-demo3.bin
# ${PORT} -iwad doom2.wad -file hacx.wad -deh hacx.deh -${DEMOCMD} demo1 -dumpdemo ${DUMPDIR}/doom2-hacx-demo1.bin
# ${PORT} -iwad doom2.wad -file hacx.wad -deh hacx.deh -${DEMOCMD} demo2 -dumpdemo ${DUMPDIR}/doom2-hacx-demo2.bin
# ${PORT} -iwad doom2.wad -file hacx.wad -deh hacx.deh -${DEMOCMD} demo3 -dumpdemo ${DUMPDIR}/doom2-hacx-demo3.bin
# ${PORT} -iwad doom2.wad -file tntgrvnc.wad -${DEMOCMD} demo1 -dumpdemo ${DUMPDIR}/doom2-tntgrvnc-demo1.bin
${PORT} -iwad doom2.wad -file tntgrvnc.wad -${DEMOCMD} demo2 -dumpdemo ${DUMPDIR}/doom2-tntgrvnc-demo2.bin
# ${PORT} -iwad doom2.wad -file tntgrvnc.wad -${DEMOCMD} demo3 -dumpdemo ${DUMPDIR}/doom2-tntgrvnc-demo3.bin
# ${PORT} -iwad plutonia.wad -${DEMOCMD} ${DEMODIR}/blindav.lmp -dumpdemo ${DUMPDIR}/plutonia-blindav.bin
# ${PORT} -iwad plutonia.wad -${DEMOCMD} ${DEMODIR}/deadlift.lmp -dumpdemo ${DUMPDIR}/plutonia-deadlift.bin
# ${PORT} -iwad plutonia.wad -${DEMOCMD} demo1 -dumpdemo ${DUMPDIR}/plutonia-demo1.bin
# ${PORT} -iwad plutonia.wad -${DEMOCMD} demo2 -dumpdemo ${DUMPDIR}/plutonia-demo2.bin
# ${PORT} -iwad plutonia.wad -${DEMOCMD} demo3 -dumpdemo ${DUMPDIR}/plutonia-demo3.bin
# ${PORT} -iwad plutonia.wad -${DEMOCMD} ${DEMODIR}/nontoxic.lmp -dumpdemo ${DUMPDIR}/plutonia-nontoxic.bin
# ${PORT} -iwad plutonia.wad -${DEMOCMD} ${DEMODIR}/revfight.lmp -dumpdemo ${DUMPDIR}/plutonia-revfight.bin
# ${PORT} -iwad tnt.wad -${DEMOCMD} demo1 -dumpdemo ${DUMPDIR}/tnt-demo1.bin
# ${PORT} -iwad tnt.wad -${DEMOCMD} demo2 -dumpdemo ${DUMPDIR}/tnt-demo2.bin
# ${PORT} -iwad tnt.wad -${DEMOCMD} demo3 -dumpdemo ${DUMPDIR}/tnt-demo3.bin
# ${PORT} -iwad tnt.wad -${DEMOCMD} ${DEMODIR}/soulfrag.lmp -dumpdemo ${DUMPDIR}/tnt-soulfrag.bin

# ${PORT} -iwad doom2.wad -file requiem.wad -${DEMOCMD} demo1 -dumpdemo ${DUMPDIR}/doom2-requiem-demo1.bin
# ${PORT} -iwad doom2.wad -file requiem.wad -${DEMOCMD} demo2 -dumpdemo ${DUMPDIR}/doom2-requiem-demo2.bin

## Long

# ${PORT} -iwad doom2.wad -file requiem.wad -${DEMOCMD} demo3 -dumpdemo ${DUMPDIR}/doom2-requiem-demo3.bin
# ${PORT} -iwad doom.wad -${DEMOCMD} ${DEMODIR}/ep1-0500.lmp -dumpdemo ${DUMPDIR}/doom-ep1-0500.bin
# ${PORT} -iwad doom2.wad -${DEMOCMD} ${DEMODIR}/30nm4048.lmp -dumpdemo ${DUMPDIR}/doom2-30nm4048.bin
# ${PORT} -iwad doom2.wad -${DEMOCMD} ${DEMODIR}/nm30cop1.lmp -dumpdemo ${DUMPDIR}/doom2-nm30cop1.bin
# ${PORT} -iwad doom2.wad -${DEMOCMD} ${DEMODIR}/nm30cop2.lmp -dumpdemo ${DUMPDIR}/doom2-nm30cop2.bin
## ${PORT} -iwad doom2.wad -${DEMOCMD} ${DEMODIR}/coopuv30.lmp -dumpdemo ${DUMPDIR}/doom2-coopuv30.bin
