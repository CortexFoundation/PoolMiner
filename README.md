
### Mining Algorithm

- The proof of work algorithm used is called Cuckoo Cycle, a graph theory-based algorithm that is far less energy-intensive than most other CPU, GPU or ASIC-bound PoW algorithms. The goal is to lower mining requirements, ensuring true decentralization and laying the foundation for future scalability.
- Cortex utilizes CuckAroo 30, a variation of Cukoo Cycle. CuckARoo replaces Siphash-2–4 with Siphash-4–8, focusing more on memory use. The solution time is bound to memory bandwidth, making it resistant to ASIC chips. Unlike other PoW algorithms that require maxing out the capacity of your hardware and consuming a lot of power in the process, a memory-bound algorithm like CuckARoo requires far less energy than most other GPU, CPU or ASIC-bound PoW algorithms.

### Need:
- CUDA Version 9.2+
- NVIDIA Driver Version: 396.37+
- Below is the recommended minimum requirements for the official miner. Third party miners may be available after the Mainnet release which might be optimized to use less memory.
- Recommend: 1080ti, 2080ti, titan V, titan RTX

### Step:
1. tar -zxvf cortex_miner.tar.gz
2. cd cortex_miner
3. config the file: start.sh
```
worker='your worker name'
pool_uri='the remote pool uri, example: xxxx@xxx.com:port'
pool_uri_1='the first candidate pool uri'
pool_uri_2='the second candidate pool uri'
device=the devices used mining, example:0,1,2,3,4,5,6,7,8
account='you wallet address, example:0x0000000000000000000000000'
```
example:
```
worker='miner1'
pool_uri='xxxx@xxx.com:port'
pool_uri_1='yyy@yy.com:port1'
pool_uri_2='127.0.0.1:port'
device=0,1,2,3
account='0x0000000000000000000000000'
```
4. chmod +x start.sh
5. ./start.sh or ./build/bin/cortex_miner -pool_uri=Pool_Uri:Port -account=Your_Account_Address -devices=Your_GPU_Ids
6. Recommend Pool Uri : cuckoo.cortexmint.com:8008
