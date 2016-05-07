#!/bin/sh

DEMODIR=/home/charlie/demos
PORT='/home/charlie/code/d2k/cbuild/d2k -nosfx -nomusic -nomouse -nodraw'
DEMOCMD=fastdemo
DUMPDIR='/home/charlie/code/democomp/d2k'
GOODDUMPDIR='/home/charlie/code/democomp/prquiet'
DEMOCOMP='/home/charlie/code/democomp/build/democomp'

rm -f ${DUMPDIR}/*.bin

${PORT} -iwad doom2.wad -file chextc.wad -deh chex.deh -${DEMOCMD} demo1 -dumpdemo ${DUMPDIR}/chex-demo1.bin
${DEMOCOMP} ${GOODDUMPDIR}/chex-demo1.bin ${DUMPDIR}/chex-demo1.bin || exit 1

${PORT} -iwad doom2.wad -file chextc.wad -deh chex.deh -${DEMOCMD} demo2 -dumpdemo ${DUMPDIR}/chex-demo2.bin
${DEMOCOMP} ${GOODDUMPDIR}/chex-demo2.bin ${DUMPDIR}/chex-demo2.bin || exit 1

${PORT} -iwad doom2.wad -file chextc.wad -deh chex.deh -${DEMOCMD} demo3 -dumpdemo ${DUMPDIR}/chex-demo3.bin
${DEMOCOMP} ${GOODDUMPDIR}/chex-demo3.bin ${DUMPDIR}/chex-demo3.bin || exit 1

${PORT} -iwad doom.wad -${DEMOCMD} ${DEMODIR}/bohfight.lmp -dumpdemo ${DUMPDIR}/doom-bohfight.bin
${DEMOCOMP} ${GOODDUMPDIR}/doom-bohfight.bin ${DUMPDIR}/doom-bohfight.bin || exit 1

${PORT} -iwad doom.wad -${DEMOCMD} ${DEMODIR}/d1shock.lmp -dumpdemo ${DUMPDIR}/doom-d1shock.bin
${DEMOCOMP} ${GOODDUMPDIR}/doom-d1shock.bin ${DUMPDIR}/doom-d1shock.bin || exit 1

${PORT} -iwad doom.wad -${DEMOCMD} ${DEMODIR}/d1slide.lmp -dumpdemo ${DUMPDIR}/doom-d1slide.bin
${DEMOCOMP} ${GOODDUMPDIR}/doom-d1slide.bin ${DUMPDIR}/doom-d1slide.bin || exit 1

${PORT} -iwad doom.wad -${DEMOCMD} demo1 -dumpdemo ${DUMPDIR}/doom-demo1.bin
${DEMOCOMP} ${GOODDUMPDIR}/doom-demo1.bin ${DUMPDIR}/doom-demo1.bin || exit 1

${PORT} -iwad doom.wad -${DEMOCMD} demo2 -dumpdemo ${DUMPDIR}/doom-demo2.bin
${DEMOCOMP} ${GOODDUMPDIR}/doom-demo2.bin ${DUMPDIR}/doom-demo2.bin || exit 1

${PORT} -iwad doom.wad -${DEMOCMD} demo3 -dumpdemo ${DUMPDIR}/doom-demo3.bin
${DEMOCOMP} ${GOODDUMPDIR}/doom-demo3.bin ${DUMPDIR}/doom-demo3.bin || exit 1

${PORT} -iwad doom.wad -${DEMOCMD} ${DEMODIR}/impfight.lmp -dumpdemo ${DUMPDIR}/doom-impfight.bin
${DEMOCOMP} ${GOODDUMPDIR}/doom-impfight.bin ${DUMPDIR}/doom-impfight.bin || exit 1

${PORT} -iwad doom.wad -${DEMOCMD} ${DEMODIR}/speedbmp.lmp -dumpdemo ${DUMPDIR}/doom-speedbmp.bin
${DEMOCOMP} ${GOODDUMPDIR}/doom-speedbmp.bin ${DUMPDIR}/doom-speedbmp.bin || exit 1

${PORT} -iwad doom.wad -${DEMOCMD} ${DEMODIR}/stilhere.lmp -dumpdemo ${DUMPDIR}/doom-stilhere.bin
${DEMOCOMP} ${GOODDUMPDIR}/doom-stilhere.bin ${DUMPDIR}/doom-stilhere.bin || exit 1

${PORT} -iwad doom.wad -${DEMOCMD} ${DEMODIR}/stuckimp.lmp -dumpdemo ${DUMPDIR}/doom-stuckimp.bin
${DEMOCOMP} ${GOODDUMPDIR}/doom-stuckimp.bin ${DUMPDIR}/doom-stuckimp.bin || exit 1

${PORT} -iwad doom.wad -${DEMOCMD} ${DEMODIR}/tapfocus.lmp -dumpdemo ${DUMPDIR}/doom-tapfocus.bin
${DEMOCOMP} ${GOODDUMPDIR}/doom-tapfocus.bin ${DUMPDIR}/doom-tapfocus.bin || exit 1

${PORT} -iwad doom.wad -${DEMOCMD} ${DEMODIR}/wallrun.lmp -dumpdemo ${DUMPDIR}/doom-wallrun.bin
${DEMOCOMP} ${GOODDUMPDIR}/doom-wallrun.bin ${DUMPDIR}/doom-wallrun.bin || exit 1

${PORT} -iwad doom2.wad -${DEMOCMD} ${DEMODIR}/02noview.lmp -dumpdemo ${DUMPDIR}/doom2-02noview.bin
${DEMOCOMP} ${GOODDUMPDIR}/doom2-02noview.bin ${DUMPDIR}/doom2-02noview.bin || exit 1

${PORT} -iwad doom2.wad -${DEMOCMD} ${DEMODIR}/07intowr.lmp -dumpdemo ${DUMPDIR}/doom2-07intowr.bin
${DEMOCOMP} ${GOODDUMPDIR}/doom2-07intowr.bin ${DUMPDIR}/doom2-07intowr.bin || exit 1

${PORT} -iwad doom2.wad -${DEMOCMD} ${DEMODIR}/07tower.lmp -dumpdemo ${DUMPDIR}/doom2-07tower.bin
${DEMOCOMP} ${GOODDUMPDIR}/doom2-07tower.bin ${DUMPDIR}/doom2-07tower.bin || exit 1

${PORT} -iwad doom2.wad -${DEMOCMD} ${DEMODIR}/2ghosts.lmp -dumpdemo ${DUMPDIR}/doom2-2ghosts.bin
${DEMOCOMP} ${GOODDUMPDIR}/doom2-2ghosts.bin ${DUMPDIR}/doom2-2ghosts.bin || exit 1

${PORT} -iwad doom2.wad -${DEMOCMD} demo1 -dumpdemo ${DUMPDIR}/doom2-demo1.bin
${DEMOCOMP} ${GOODDUMPDIR}/doom2-demo1.bin ${DUMPDIR}/doom2-demo1.bin || exit 1

${PORT} -iwad doom2.wad -${DEMOCMD} demo2 -dumpdemo ${DUMPDIR}/doom2-demo2.bin
${DEMOCOMP} ${GOODDUMPDIR}/doom2-demo2.bin ${DUMPDIR}/doom2-demo2.bin || exit 1

${PORT} -iwad doom2.wad -${DEMOCMD} demo3 -dumpdemo ${DUMPDIR}/doom2-demo3.bin
${DEMOCOMP} ${GOODDUMPDIR}/doom2-demo3.bin ${DUMPDIR}/doom2-demo3.bin || exit 1

${PORT} -iwad doom2.wad -${DEMOCMD} ${DEMODIR}/bfgdemo.lmp -dumpdemo ${DUMPDIR}/doom2-bfgdemo.bin
${DEMOCOMP} ${GOODDUMPDIR}/doom2-bfgdemo.bin ${DUMPDIR}/doom2-bfgdemo.bin || exit 1

${PORT} -iwad doom2.wad -${DEMOCMD} ${DEMODIR}/bounce1.lmp -dumpdemo ${DUMPDIR}/doom2-bounce1.bin
${DEMOCOMP} ${GOODDUMPDIR}/doom2-bounce1.bin ${DUMPDIR}/doom2-bounce1.bin || exit 1

${PORT} -iwad doom2.wad -${DEMOCMD} ${DEMODIR}/bounce2.lmp -dumpdemo ${DUMPDIR}/doom2-bounce2.bin
${DEMOCOMP} ${GOODDUMPDIR}/doom2-bounce2.bin ${DUMPDIR}/doom2-bounce2.bin || exit 1

${PORT} -iwad doom2.wad -${DEMOCMD} ${DEMODIR}/burnthru.lmp -dumpdemo ${DUMPDIR}/doom2-burnthru.bin
${DEMOCOMP} ${GOODDUMPDIR}/doom2-burnthru.bin ${DUMPDIR}/doom2-burnthru.bin || exit 1

${PORT} -iwad doom2.wad -${DEMOCMD} ${DEMODIR}/cacofit2.lmp -dumpdemo ${DUMPDIR}/doom2-cacofit.bin
${DEMOCOMP} ${GOODDUMPDIR}/doom2-cacofit.bin ${DUMPDIR}/doom2-cacofit.bin || exit 1

${PORT} -iwad doom2.wad -${DEMOCMD} ${DEMODIR}/cacofite.lmp -dumpdemo ${DUMPDIR}/doom2-cacofite.bin
${DEMOCOMP} ${GOODDUMPDIR}/doom2-cacofite.bin ${DUMPDIR}/doom2-cacofite.bin || exit 1

${PORT} -iwad doom2.wad -${DEMOCMD} ${DEMODIR}/d2shock.lmp -dumpdemo ${DUMPDIR}/doom2-d2shock.bin
${DEMOCOMP} ${GOODDUMPDIR}/doom2-d2shock.bin ${DUMPDIR}/doom2-d2shock.bin || exit 1

${PORT} -iwad doom2.wad -${DEMOCMD} ${DEMODIR}/d2slide.lmp -dumpdemo ${DUMPDIR}/doom2-d2slide.bin
${DEMOCOMP} ${GOODDUMPDIR}/doom2-d2slide.bin ${DUMPDIR}/doom2-d2slide.bin || exit 1

${PORT} -iwad doom2.wad -${DEMOCMD} ${DEMODIR}/dedngone.lmp -dumpdemo ${DUMPDIR}/doom2-dedngone.bin
${DEMOCOMP} ${GOODDUMPDIR}/doom2-dedngone.bin ${DUMPDIR}/doom2-dedngone.bin || exit 1

${PORT} -iwad doom2.wad -${DEMOCMD} ${DEMODIR}/firewall.lmp -dumpdemo ${DUMPDIR}/doom2-firewall.bin
${DEMOCOMP} ${GOODDUMPDIR}/doom2-firewall.bin ${DUMPDIR}/doom2-firewall.bin || exit 1

${PORT} -iwad doom2.wad -${DEMOCMD} ${DEMODIR}/flicker.lmp -dumpdemo ${DUMPDIR}/doom2-flicker.bin
${DEMOCOMP} ${GOODDUMPDIR}/doom2-flicker.bin ${DUMPDIR}/doom2-flicker.bin || exit 1

${PORT} -iwad doom2.wad -${DEMOCMD} ${DEMODIR}/fusedem.lmp -dumpdemo ${DUMPDIR}/doom2-fusedem.bin
${DEMOCOMP} ${GOODDUMPDIR}/doom2-fusedem.bin ${DUMPDIR}/doom2-fusedem.bin || exit 1

${PORT} -iwad doom2.wad -${DEMOCMD} ${DEMODIR}/ghostpe1.lmp -dumpdemo ${DUMPDIR}/doom2-ghostpe1.bin
${DEMOCOMP} ${GOODDUMPDIR}/doom2-ghostpe1.bin ${DUMPDIR}/doom2-ghostpe1.bin || exit 1

${PORT} -iwad doom2.wad -${DEMOCMD} ${DEMODIR}/ghostpe2.lmp -dumpdemo ${DUMPDIR}/doom2-ghostpe2.bin
${DEMOCOMP} ${GOODDUMPDIR}/doom2-ghostpe2.bin ${DUMPDIR}/doom2-ghostpe2.bin || exit 1

${PORT} -iwad doom2.wad -${DEMOCMD} ${DEMODIR}/impghost.lmp -dumpdemo ${DUMPDIR}/doom2-impghost.bin
${DEMOCOMP} ${GOODDUMPDIR}/doom2-impghost.bin ${DUMPDIR}/doom2-impghost.bin || exit 1

${PORT} -iwad doom2.wad -${DEMOCMD} ${DEMODIR}/impmiss.lmp -dumpdemo ${DUMPDIR}/doom2-impmiss.bin
${DEMOCOMP} ${GOODDUMPDIR}/doom2-impmiss.bin ${DUMPDIR}/doom2-impmiss.bin || exit 1

${PORT} -iwad doom2.wad -${DEMOCMD} ${DEMODIR}/lv04-028.lmp -dumpdemo ${DUMPDIR}/doom2-lv04-028.bin
${DEMOCOMP} ${GOODDUMPDIR}/doom2-lv04-028.bin ${DUMPDIR}/doom2-lv04-028.bin || exit 1

${PORT} -iwad doom2.wad -${DEMOCMD} ${DEMODIR}/lv07-046.lmp -dumpdemo ${DUMPDIR}/doom2-lv07-046.bin
${DEMOCOMP} ${GOODDUMPDIR}/doom2-lv07-046.bin ${DUMPDIR}/doom2-lv07-046.bin || exit 1

${PORT} -iwad doom2.wad -${DEMOCMD} ${DEMODIR}/lv12-043.lmp -dumpdemo ${DUMPDIR}/doom2-lv12-043.bin
${DEMOCOMP} ${GOODDUMPDIR}/doom2-lv12-043.bin ${DUMPDIR}/doom2-lv12-043.bin || exit 1

${PORT} -iwad doom2.wad -${DEMOCMD} ${DEMODIR}/lv15s032.lmp -dumpdemo ${DUMPDIR}/doom2-lv15s032.bin
${DEMOCOMP} ${GOODDUMPDIR}/doom2-lv15s032.bin ${DUMPDIR}/doom2-lv15s032.bin || exit 1

${PORT} -iwad doom2.wad -${DEMOCMD} ${DEMODIR}/revorbit.lmp -dumpdemo ${DUMPDIR}/doom2-revorbit.bin
${DEMOCOMP} ${GOODDUMPDIR}/doom2-revorbit.bin ${DUMPDIR}/doom2-revorbit.bin || exit 1

${PORT} -iwad doom2.wad -${DEMOCMD} ${DEMODIR}/smpunch.lmp -dumpdemo ${DUMPDIR}/doom2-smpunch.bin
${DEMOCOMP} ${GOODDUMPDIR}/doom2-smpunch.bin ${DUMPDIR}/doom2-smpunch.bin || exit 1

${PORT} -iwad doom2.wad -${DEMOCMD} ${DEMODIR}/soulpop.lmp -dumpdemo ${DUMPDIR}/doom2-soulpop.bin
${DEMOCOMP} ${GOODDUMPDIR}/doom2-soulpop.bin ${DUMPDIR}/doom2-soulpop.bin || exit 1

${PORT} -iwad doom2.wad -${DEMOCMD} ${DEMODIR}/tallbomb.lmp -dumpdemo ${DUMPDIR}/doom2-tallbomb.bin
${DEMOCOMP} ${GOODDUMPDIR}/doom2-tallbomb.bin ${DUMPDIR}/doom2-tallbomb.bin || exit 1

${PORT} -iwad doom2.wad -${DEMOCMD} ${DEMODIR}/thingrun.lmp -dumpdemo ${DUMPDIR}/doom2-thingrun.bin
${DEMOCOMP} ${GOODDUMPDIR}/doom2-thingrun.bin ${DUMPDIR}/doom2-thingrun.bin || exit 1

${PORT} -iwad doom2.wad -file av.wad -${DEMOCMD} demo1 -dumpdemo ${DUMPDIR}/doom2-av-demo1.bin
${DEMOCOMP} ${GOODDUMPDIR}/doom2-av-demo1.bin ${DUMPDIR}/doom2-av-demo1.bin || exit 1

${PORT} -iwad doom2.wad -file av.wad -${DEMOCMD} demo2 -dumpdemo ${DUMPDIR}/doom2-av-demo2.bin
${DEMOCOMP} ${GOODDUMPDIR}/doom2-av-demo2.bin ${DUMPDIR}/doom2-av-demo2.bin || exit 1

${PORT} -iwad doom2.wad -file av.wad -${DEMOCMD} demo3 -dumpdemo ${DUMPDIR}/doom2-av-demo3.bin
${DEMOCOMP} ${GOODDUMPDIR}/doom2-av-demo3.bin ${DUMPDIR}/doom2-av-demo3.bin || exit 1

${PORT} -iwad doom2.wad -file gothicdm.wad -${DEMOCMD} demo1 -dumpdemo ${DUMPDIR}/doom2-gothicdm-demo1.bin
${DEMOCOMP} ${GOODDUMPDIR}/doom2-gothicdm-demo1.bin ${DUMPDIR}/doom2-gothicdm-demo1.bin || exit 1

${PORT} -iwad doom2.wad -file gothicdm.wad -${DEMOCMD} demo2 -dumpdemo ${DUMPDIR}/doom2-gothicdm-demo2.bin
${DEMOCOMP} ${GOODDUMPDIR}/doom2-gothicdm-demo2.bin ${DUMPDIR}/doom2-gothicdm-demo2.bin || exit 1

${PORT} -iwad doom2.wad -file gothicdm.wad -${DEMOCMD} demo3 -dumpdemo ${DUMPDIR}/doom2-gothicdm-demo3.bin
${DEMOCOMP} ${GOODDUMPDIR}/doom2-gothicdm-demo3.bin ${DUMPDIR}/doom2-gothicdm-demo3.bin || exit 1

${PORT} -iwad doom2.wad -file gothicdm.wad -${DEMOCMD} demo4 -dumpdemo ${DUMPDIR}/doom2-gothicdm-demo4.bin
${DEMOCOMP} ${GOODDUMPDIR}/doom2-gothicdm-demo4.bin ${DUMPDIR}/doom2-gothicdm-demo4.bin || exit 1

${PORT} -iwad doom2.wad -file hr.wad -${DEMOCMD} ${DEMODIR}/hf21tysn.lmp -dumpdemo ${DUMPDIR}/doom2-hr-hf21tysn.bin
${DEMOCOMP} ${GOODDUMPDIR}/doom2-hr-hf21tysn.bin ${DUMPDIR}/doom2-hr-hf21tysn.bin || exit 1

${PORT} -iwad doom2.wad -file hr2final.wad -${DEMOCMD} ${DEMODIR}/hr2-lv01.lmp -dumpdemo ${DUMPDIR}/doom2-hr2final-hr2-lv01.bin
${DEMOCOMP} ${GOODDUMPDIR}/doom2-hr2final-hr2-lv01.bin ${DUMPDIR}/doom2-hr2final-hr2-lv01.bin || exit 1

${PORT} -iwad doom2.wad -file icarus.wad -${DEMOCMD} demo1 -dumpdemo ${DUMPDIR}/doom2-icarus-demo1.bin
${DEMOCOMP} ${GOODDUMPDIR}/doom2-icarus-demo1.bin ${DUMPDIR}/doom2-icarus-demo1.bin || exit 1

${PORT} -iwad doom2.wad -file icarus.wad -${DEMOCMD} demo2 -dumpdemo ${DUMPDIR}/doom2-icarus-demo2.bin
${DEMOCOMP} ${GOODDUMPDIR}/doom2-icarus-demo2.bin ${DUMPDIR}/doom2-icarus-demo2.bin || exit 1

${PORT} -iwad doom2.wad -file icarus.wad -${DEMOCMD} demo3 -dumpdemo ${DUMPDIR}/doom2-icarus-demo3.bin
${DEMOCOMP} ${GOODDUMPDIR}/doom2-icarus-demo3.bin ${DUMPDIR}/doom2-icarus-demo3.bin || exit 1

${PORT} -iwad doom2.wad -file mm2.wad -${DEMOCMD} demo1 -dumpdemo ${DUMPDIR}/doom2-mm2-demo1.bin
${DEMOCOMP} ${GOODDUMPDIR}/doom2-mm2-demo1.bin ${DUMPDIR}/doom2-mm2-demo1.bin || exit 1

${PORT} -iwad doom2.wad -file mm2.wad -${DEMOCMD} demo2 -dumpdemo ${DUMPDIR}/doom2-mm2-demo2.bin
${DEMOCOMP} ${GOODDUMPDIR}/doom2-mm2-demo2.bin ${DUMPDIR}/doom2-mm2-demo2.bin || exit 1

${PORT} -iwad doom2.wad -file mm2.wad -${DEMOCMD} demo3 -dumpdemo ${DUMPDIR}/doom2-mm2-demo3.bin
${DEMOCOMP} ${GOODDUMPDIR}/doom2-mm2-demo3.bin ${DUMPDIR}/doom2-mm2-demo3.bin || exit 1

${PORT} -iwad doom2.wad -file hacx.wad -deh hacx.deh -${DEMOCMD} demo1 -dumpdemo ${DUMPDIR}/doom2-hacx-demo1.bin
${DEMOCOMP} ${GOODDUMPDIR}/doom2-hacx-demo1.bin ${DUMPDIR}/doom2-hacx-demo1.bin || exit 1

${PORT} -iwad doom2.wad -file hacx.wad -deh hacx.deh -${DEMOCMD} demo2 -dumpdemo ${DUMPDIR}/doom2-hacx-demo2.bin
${DEMOCOMP} ${GOODDUMPDIR}/doom2-hacx-demo2.bin ${DUMPDIR}/doom2-hacx-demo2.bin || exit 1

${PORT} -iwad doom2.wad -file hacx.wad -deh hacx.deh -${DEMOCMD} demo3 -dumpdemo ${DUMPDIR}/doom2-hacx-demo3.bin
${DEMOCOMP} ${GOODDUMPDIR}/doom2-hacx-demo3.bin ${DUMPDIR}/doom2-hacx-demo3.bin || exit 1

${PORT} -iwad doom2.wad -file tntgrvnc.wad -${DEMOCMD} demo1 -dumpdemo ${DUMPDIR}/doom2-tntgrvnc-demo1.bin
${DEMOCOMP} ${GOODDUMPDIR}/doom2-tntgrvnc-demo1.bin ${DUMPDIR}/doom2-tntgrvnc-demo1.bin || exit 1

${PORT} -iwad doom2.wad -file tntgrvnc.wad -${DEMOCMD} demo2 -dumpdemo ${DUMPDIR}/doom2-tntgrvnc-demo2.bin
${DEMOCOMP} ${GOODDUMPDIR}/doom2-tntgrvnc-demo2.bin ${DUMPDIR}/doom2-tntgrvnc-demo2.bin || exit 1

${PORT} -iwad doom2.wad -file tntgrvnc.wad -${DEMOCMD} demo3 -dumpdemo ${DUMPDIR}/doom2-tntgrvnc-demo3.bin
${DEMOCOMP} ${GOODDUMPDIR}/doom2-tntgrvnc-demo3.bin ${DUMPDIR}/doom2-tntgrvnc-demo3.bin || exit 1

${PORT} -iwad plutonia.wad -${DEMOCMD} ${DEMODIR}/blindav.lmp -dumpdemo ${DUMPDIR}/plutonia-blindav.bin
${DEMOCOMP} ${GOODDUMPDIR}/plutonia-blindav.bin ${DUMPDIR}/plutonia-blindav.bin || exit 1

${PORT} -iwad plutonia.wad -${DEMOCMD} ${DEMODIR}/deadlift.lmp -dumpdemo ${DUMPDIR}/plutonia-deadlift.bin
${DEMOCOMP} ${GOODDUMPDIR}/plutonia-deadlift.bin ${DUMPDIR}/plutonia-deadlift.bin || exit 1

${PORT} -iwad plutonia.wad -${DEMOCMD} demo1 -dumpdemo ${DUMPDIR}/plutonia-demo1.bin
${DEMOCOMP} ${GOODDUMPDIR}/plutonia-demo1.bin ${DUMPDIR}/plutonia-demo1.bin || exit 1

${PORT} -iwad plutonia.wad -${DEMOCMD} demo2 -dumpdemo ${DUMPDIR}/plutonia-demo2.bin
${DEMOCOMP} ${GOODDUMPDIR}/plutonia-demo2.bin ${DUMPDIR}/plutonia-demo2.bin || exit 1

${PORT} -iwad plutonia.wad -${DEMOCMD} demo3 -dumpdemo ${DUMPDIR}/plutonia-demo3.bin
${DEMOCOMP} ${GOODDUMPDIR}/plutonia-demo3.bin ${DUMPDIR}/plutonia-demo3.bin || exit 1

${PORT} -iwad plutonia.wad -${DEMOCMD} ${DEMODIR}/nontoxic.lmp -dumpdemo ${DUMPDIR}/plutonia-nontoxic.bin
${DEMOCOMP} ${GOODDUMPDIR}/plutonia-nontoxic.bin ${DUMPDIR}/plutonia-nontoxic.bin || exit 1

${PORT} -iwad plutonia.wad -${DEMOCMD} ${DEMODIR}/revfight.lmp -dumpdemo ${DUMPDIR}/plutonia-revfight.bin
${DEMOCOMP} ${GOODDUMPDIR}/plutonia-revfight.bin ${DUMPDIR}/plutonia-revfight.bin || exit 1

${PORT} -iwad tnt.wad -${DEMOCMD} demo1 -dumpdemo ${DUMPDIR}/tnt-demo1.bin
${DEMOCOMP} ${GOODDUMPDIR}/tnt-demo1.bin ${DUMPDIR}/tnt-demo1.bin || exit 1

${PORT} -iwad tnt.wad -${DEMOCMD} demo2 -dumpdemo ${DUMPDIR}/tnt-demo2.bin
${DEMOCOMP} ${GOODDUMPDIR}/tnt-demo2.bin ${DUMPDIR}/tnt-demo2.bin || exit 1

${PORT} -iwad tnt.wad -${DEMOCMD} demo3 -dumpdemo ${DUMPDIR}/tnt-demo3.bin
${DEMOCOMP} ${GOODDUMPDIR}/tnt-demo3.bin ${DUMPDIR}/tnt-demo3.bin || exit 1

${PORT} -iwad tnt.wad -${DEMOCMD} ${DEMODIR}/soulfrag.lmp -dumpdemo ${DUMPDIR}/tnt-soulfrag.bin
${DEMOCOMP} ${GOODDUMPDIR}/tnt-soulfrag.bin ${DUMPDIR}/tnt-soulfrag.bin || exit 1

${PORT} -iwad doom2.wad -file requiem.wad -${DEMOCMD} demo1 -dumpdemo ${DUMPDIR}/doom2-requiem-demo1.bin
${DEMOCOMP} ${GOODDUMPDIR}/doom2-requiem-demo1.bin ${DUMPDIR}/doom2-requiem-demo1.bin || exit 1

${PORT} -iwad doom2.wad -file requiem.wad -${DEMOCMD} demo2 -dumpdemo ${DUMPDIR}/doom2-requiem-demo2.bin
${DEMOCOMP} ${GOODDUMPDIR}/doom2-requiem-demo2.bin ${DUMPDIR}/doom2-requiem-demo2.bin || exit 1

## Long

# ${PORT} -iwad doom2.wad -file requiem.wad -${DEMOCMD} demo3 -dumpdemo ${DUMPDIR}/doom2-requiem-demo3.bin
# ${PORT} -iwad doom.wad -${DEMOCMD} ${DEMODIR}/ep1-0500.lmp -dumpdemo ${DUMPDIR}/doom-ep1-0500.bin
# ${PORT} -iwad doom2.wad -${DEMOCMD} ${DEMODIR}/30nm4048.lmp -dumpdemo ${DUMPDIR}/doom2-30nm4048.bin
# ${PORT} -iwad doom2.wad -${DEMOCMD} ${DEMODIR}/nm30cop1.lmp -dumpdemo ${DUMPDIR}/doom2-nm30cop1.bin
# ${PORT} -iwad doom2.wad -${DEMOCMD} ${DEMODIR}/nm30cop2.lmp -dumpdemo ${DUMPDIR}/doom2-nm30cop2.bin
# ${PORT} -iwad doom2.wad -${DEMOCMD} ${DEMODIR}/coopuv30.lmp -dumpdemo ${DUMPDIR}/doom2-coopuv30.bin
