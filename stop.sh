ps -ef | grep './build/bin/cortex_miner' | grep -v 'grep' | awk  '{print $2}' | xargs sudo kill -9
