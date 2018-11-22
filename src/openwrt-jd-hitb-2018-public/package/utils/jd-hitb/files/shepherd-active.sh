#!/bin/sh

# blink the led light


# reset the wifi
MAC=$(uci get network.lan_dev.macaddr | sed -e "s/://g")
SSID=GP${MAC}
uci set wireless.default_radio0.ssid=${SSID}
MD5=$(echo ${MAC}"JD-HITB" | md5sum)
KEY=${MD5:2:8}
uci set wireless.default_radio0.key=${KEY}
# wait active
uci set wireless.radio0.disabled=0
uci commit
/etc/init.d/network restart

# remove self
rm -f /etc/shepherd-active.sh
