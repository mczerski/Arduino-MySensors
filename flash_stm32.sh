#/usr/bin/env bash
image=`ls -t1 /tmp/arduino_build_*/*.ino.elf | head -n1`

openocd\
    -f /usr/share/openocd/scripts/interface/stlink-v2.cfg\
    -f /usr/share/openocd/scripts/target/stm32f1x.cfg\
    -c init\
    -c targets\
    -c "reset halt"\
    -c "flash write_image erase $image"\
    -c "verify_image $image"\
    -c "reset run"\
    -c shutdown
