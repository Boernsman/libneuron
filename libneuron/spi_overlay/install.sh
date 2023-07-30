#!/bin/bash

install_overlay() {
    echo "Enabling SPI overlay"
    sh compile-dtc
    if ! grep -q 'dtoverlay=neuron-spi' /boot/config.txt ;then
        echo -e "$(cat /boot/config.txt) \n\n#Enable UniPi Neuron SPI overlay\ndtoverlay=neuron-spi" > /boot/config.txt
    fi
    if ! grep -q '#dtparam=spi=on' /boot/config.txt ;then
            sed -i '/dtparam=spi=on/s/^/#/g' /etc/modprobe.d/raspi-blacklist.conf
        fi
    cd ..
}

if [ "$EUID" -ne 0 ]
  then echo "Please run as root."
  exit
fi

install_overlay

if ask "Reboot is required. Do you want to reboot now?"; then
    reboot
else
    echo 'Changes will become effective after reboot.'
fi
echo ' '
