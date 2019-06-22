#! /bin/bash
export LD_LIBRARY_PATH=/usr/local/cuda/lib64/:/usr/local/cuda/lib64/stubs:$LD_LIBRARY_PATH
export LIBRARY_PATH=/usr/local/cuda/lib64/:/usr/local/cuda/lib64/stubs:$LIBRARY_PATH
export PATH=/usr/local/go/bin:/usr/local/cuda/bin/:$PATH
NVIDIA_COUNT=`nvidia-smi -L | grep 'GeForce' | wc -l`
echo "there has "$NVIDIA_COUNT "gpu"
str=""
for ((i=0; i < $NVIDIA_COUNT; i++))
do
 str=$str$i
 if [ $i -lt $[$NVIDIA_COUNT - 1] ]; then
  str=$str,
 fi
done

worker=`hostname`
pool_uri='cuckoo.cortexlabs.ai:8008'
device=$str
account='0xE893BA644128a0065B75d2c4f642615710802D4F'
start='./build/bin/cortex_miner -pool_uri='$pool_uri' -worker='$worker' -devices='$str' -account='$account
echo $start
$start
