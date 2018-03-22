#!/bin/bash
echo -n "time    : "
date +%s.%N
echo -n "rx bytes: "
cat  /sys/class/net/eth0/statistics/rx_bytes
echo -n "tx bytes: "
cat  /sys/class/net/eth0/statistics/tx_bytes
