
### Mining Algorithm
- The proof of work algorithm used is called Cuckoo Cycle, a graph theory-based algorithm that is far less energy-intensive than most other CPU, GPU or ASIC-bound PoW algorithms. The goal is to lower mining requirements, ensuring true decentralization and laying the foundation for future scalability.
- Cortex utilizes CuckAroo 30, a variation of Cukoo Cycle. CuckARoo replaces Siphash-2–4 with Siphash-4–8, focusing more on memory use. The solution time is bound to memory bandwidth, making it resistant to ASIC chips. Unlike other PoW algorithms that require maxing out the capacity of your hardware and consuming a lot of power in the process, a memory-bound algorithm like CuckARoo requires far less energy than most other GPU, CPU or ASIC-bound PoW algorithms.

### Need:
- NVIDIA Driver Version: 396.37+
- CUDA Version 9.2+
```
1. export PATH=/usr/local/cuda/bin/:$PATH
2. export LD_LIBRARY_PATH=/usr/local/cuda/lib64/:$LD_LIBRARY_PATH
3. export LIBRARY_PATH=/usr/local/cuda/lib64:$LIBRARY_PATH
```
- GPU Memory > 10.7G
  1. Below is the recommended minimum requirements for the official miner. Third party miners may be available after the Mainnet release which might be optimized to use less memory.
  2. Recommend: 1080ti, 2080ti, titan V, titan RTX
- go 
```
wget https://dl.google.com/go/go1.11.5.linux-amd64.tar.gz
sudo tar -C /usr/local -xzf go1.11.5.linux-amd64.tar.gz
echo 'export PATH="$PATH:/usr/local/go/bin"' >> ~/.bashrc
source ~/.bashrc
```
### Compile Source Code
1. git clone https://github.com/CortexFoundation/PoolMiner.git
2. cd PoolMiner
3. make
4. ./build/bin/cortex_miner --pool_uri=Pool_IP:Port -devices=Your_GPU_Ids -account=Your_Wallet_Address

### Runing Release:
1. download [cortex_miner.tar.gz](https://github.com/CortexFoundation/Cortex_Release/raw/master/cortex-miner/cortex_miner.tar.gz)
2. tar -zxvf cortex_miner.tar.gz
3. cd cortex_miner
4. start by shell:
- vim start.sh and update the parameters, example:
```
worker='miner1'
pool_uri='xxxx@xxx.com:port'
pool_uri_1='yyy@yy.com:port'
pool_uri_2='127.0.0.1:port'
device=0,1,2,3
account='0x0000000000000000000000000'
```
- chmod +x start.sh
- ./start.sh
5. start by command:
- ./cortex_miner -pool_uri=Pool_IP:Port -worker=Your_Worker_name -devices=Your_GPU_Ids -account=Your_Wallet_Address

### pool address
- Recommend : cuckoo.cortexmint.com:8008
- The Pool Link : www.cortexmint.com/
