# This Makefile is meant to be used by people that do not usually work
# with Go source code. If you know what GOPATH is then you probably
# don't need to bother with make.

CBIN = $(shell pwd)/build/bin
GO ?= latest
LIB_CUCKOO_DIR = $(shell pwd)/miner/libcuckoo
PLUGINS_DIR = $(shell pwd)/plugins
CORTEXPATH = $(shell pwd)/../build/_workspace

all:
	build/env.sh go get -tags -v ./...
	make -C ${LIB_CUCKOO_DIR}
	build/env.sh go build -buildmode=plugin -o ${PLUGINS_DIR}/cuda_helper.so ./miner/libcuckoo/cuda_helper.go
	build/env.sh go build -o build/bin/cortex_miner ./cmd/miner


clean:
	rm miner/libcuckoo/*.o miner/libcuckoo/*.a
	rm -fr build/_workspace/pkg/ $(CBIN)/* $(PLUGINS_DIR)/*
	go clean -cache

