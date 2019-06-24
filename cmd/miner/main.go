package main

import (
	"flag"
	"fmt"
	"time"
	"strings"
	"strconv"
	"sync"
	"github.com/CortexFoundation/PoolMiner/cortexminer"
	"github.com/CortexFoundation/PoolMiner/config"
	"runtime"
)

var help bool
var remote string = ""
var remote1 string = ""
var remote2 string = ""
var account string = ""
var workername string = ""
var strDeviceId string = ""
var verboseLevel int = 0

func init() {
	flag.BoolVar(&help, "help", false, "show help")
	flag.StringVar(&remote, "pool_uri", "", "mining pool address, need")
	flag.StringVar(&remote1, "pool_uri_1", "", "mining pool address, option")
	flag.StringVar(&remote2, "pool_uri_2", "", "mining pool address, option")
	flag.StringVar(&account, "account", "", "miner accounts, need")
	flag.StringVar(&workername, "worker", "", "worker name, option")
	flag.StringVar(&strDeviceId, "devices", "", "which GPU devices use for mining, need")
	flag.IntVar(&verboseLevel, "verbosity", 0, "verbosity level")

	fmt.Printf("**************************************************************\n")
	fmt.Printf("**\t\tCortex GPU Miner\t\t\t**\n")
	fmt.Printf("**************************************************************\n")
}

func main() {
	runtime.GOMAXPROCS(runtime.NumCPU() * 4)
	flag.Parse()

	if account == "" || remote == "" || strDeviceId == ""{
		fmt.Println("Usage:\n ./build/bin/cortex_miner -worker(optional) -pool_uri(need) -account(need) -devices(need)\nexample:-pool_uri=localhost:8009 -account=0xc3d7a1ef810983847510542edfd5bc5551a6321c -devices=0,1")
		return
	}
	var strDeviceIds []string = strings.Split(strDeviceId, ",")
	var deviceNum int = len(strDeviceIds)
	var deviceInfos []config.DeviceInfo
	var start_time int64 = time.Now().UnixNano() / 1e6
	for i := 0; i < deviceNum; i++ {
		var lock sync.Mutex
		v, error := strconv.Atoi(strDeviceIds[i])
		if error != nil || v < 0 {
			fmt.Println("parse deviceIds error ", error)
			return
		}
		var deviceInfo config.DeviceInfo
		deviceInfos = append(deviceInfos, deviceInfo.New(lock, (uint32)(v), start_time, 0, 0, 0, 0))
	}
	if help {
		fmt.Println("Usage:\n ./build/bin/cortex_miner -worker -pool_uri -account -devices\nexample:-pool_uri=localhost:8009 -account=0xc3d7a1ef810983847510542edfd5bc5551a6321c -devices=0,1")
	} else {
		fmt.Println(account, remote)
	}

	var param config.Param
	var cortex cortexminer.Cortex
	cm  := cortex.New(
		deviceInfos,
		param.New(remote, remote1, remote2, account, workername, uint(verboseLevel), 1, 1, false, true, false))

	cm.Mining()
}
