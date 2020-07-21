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
    result["TxVal"] = [];
    //result["txns"] = [];
    var txs_num = block.data.data.length;
    result["txs_num"] = [txs_num];

    let tx_filters = block.metadata.metadata[2]
    for (var index = 0; index < block.data.data.length; index++) {
        //var channel_header = block.data.data[index].payload.header.channel_header;
        result["TxVal"].push(tx_filters[index])
        //result["txns"].push(channel_header.tx_id)
    }
    
    console.log(result)
}).catch((err)=>{
    result["TxVal"] = "err";
    result["msg"] = err.message;
    console.log(result)
});