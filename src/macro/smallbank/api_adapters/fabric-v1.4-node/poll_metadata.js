'use strict';

const ccUtil = require("./ccutil.js")

if (process.argv.length < 3) {
    console.log("Invalid parameter...");
    console.log("Should be 'node query_blk_height.js <ordererAddr> <peerAddr> <blk number>'");
    process.exit(1);
}

var ordererAddr = "grpc://" + process.argv[2];
var peerAddr = "grpc://" + process.argv[3];
var blockNum =  parseInt(process.argv[4]);

var channel;
var result = new Object;

Promise.resolve().then(()=>{
    return ccUtil.createChannelAndClient(peerAddr, ordererAddr);
}).then((result)=>{
    channel = result.channel;
    return channel.queryBlock(blockNum);
}).then((block)=>{
    result["VALID"] = [];
    result["ENDORSEMENT"] = [];
    result["MVCC"] = [];
    result["PHANTOM"] = [];
    result["txs_sum"] = [];
    var txs_sum = block.data.data.length;
    //result["txs_sum"] = txs_sum;
    result["txs_sum"].push(txs_sum)

    let tx_filters = block.metadata.metadata[2]
    for (var index = 0; index < block.data.data.length; index++) {
        var channel_header = block.data.data[index].payload.header.channel_header;
        switch(tx_filters[index]) {
            case 0:
                result["VALID"].push(channel_header.tx_id)
                break;
            case 10:
                result["ENDORSEMENT"].push(channel_header.tx_id)
                break;
            case 11:
                result["MVCC"].push(channel_header.tx_id)
                break;
            case 12:
                result["PHANTOM"].push(channel_header.tx_id)
                break;
        }
    }
    
    console.log(result)
}).catch((err)=>{
    result["TxVal"] = "err";
    result["msg"] = err.message;
    console.log(result)
});