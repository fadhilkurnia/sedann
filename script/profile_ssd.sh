#!/bin/bash
# This script measure the disk random read throughput by varying
# the number of threads and also the read block size.
# The result is storead in ssd_profile.csv

result_file=ssd_profile.csv
thread_nums=( 1 2 4 8 16 )
block_sizes=( '4k' '8k' '16k' '32k' '64k' '128k' '256k' '512k' '1m' )
target_disk=/dev/nvme0n1

echo "rw,#threads,reqsize(kb),ioengine,iodepth,throughput(bytes/s),iops,bw_mean" > $result_file

for sz in ${block_sizes[@]}; do
    for tn in ${thread_nums[@]}; do
        echo "meassuring with IO size: $sz kb, #threads: $tn"
        fio --name=test --filename=$target_disk --filesize=4G \
            --time_based --ramp_time=2s --runtime=60s \
            --ioengine=libaio --direct=1 --verify=0 --randrepeat=0 \
            --blocksize=${sz}k --iodepth=64 --rw=randread \
            --numjobs=$tn --group_reporting --output-format=json > /tmp/fio_result.json
        bw=$(jq '.jobs[0].read.bw_bytes' /tmp/fio_result.json)
        bw_mean=$(jq '.jobs[0].read.bw_mean' /tmp/fio_result.json)
        iops=$(jq '.jobs[0].read.iops' /tmp/fio_result.json)
        echo "randread,$tn,$sz,libaio,64,$bw,$iops,$bw_mean" >> $result_file
    done
done
