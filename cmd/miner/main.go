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
var account string = ""
var workername string = ""
var strDeviceId string = ""
var verboseLevel int = 0

func init() {
	flag.BoolVar(&help, "help", false, "show help")
	flag.StringVar(&remote, "pool_uri", "47.91.2.19:8009", "mining pool address")
	flag.StringVar(&account, "account", "0xc3d7a1ef810983847510542edfd5bc5551a6321c", "miner accounts")
	flag.StringVar(&workername, "worker", "worker111111111", "worker name")
	flag.StringVar(&strDeviceId, "devices", "0", "which GPU device use for mining")
	flag.IntVar(&verboseLevel, "verbosity", 0, "verbosity level")

	fmt.Printf("**************************************************************\n")
	fmt.Printf("**\t\tCortex GPU Miner\t\t\t**\n")
	fmt.Printf("**************************************************************\n")
}

func main() {
	runtime.GOMAXPROCS(runtime.NumCPU() * 4)
	flag.Parse()

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
		fmt.Println("Usage:\ngo run miner.go -r remote -a account -c gpu\nexample:go run miner.go -r localhost:8009 -a 0xc3d7a1ef810983847510542edfd5bc5551a6321c")
	} else {
		fmt.Println(account, remote)
	}

	var param config.Param
	var cortex cortexminer.Cortex
	cm  := cortex.New(
		deviceInfos,
		param.New(remote, account, workername, uint(verboseLevel), 1, 1, false, true, false))

	cm.Mining()
}
