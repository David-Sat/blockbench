#include "db/fabric_db.h"

using namespace std;
using namespace BBUtils;

namespace ycsbc {

FabricDB::FabricDB(const string &endpoint, const string &wl_name)
    : endpoint_(endpoint) {
  if (wl_name == "ycsb") {
    std::cout << "Make sure to deploy ycsb chaincode via fabric-v1.4-node/deploy_kv.sh" << std::endl;
    sctype_ = BBUtils::SmartContractType::KVStore;
  } else {
    std::cout << "Make sure to deploy ycsb chaincode via fabric-v1.4-node/deploy_donothing.sh" << std::endl;
    sctype_ = BBUtils::SmartContractType::DoNothing;
  }
}

// ignore table
// ignore field
// read value indicated by a key
int FabricDB::Read(const string &table, const string &key,
                        const vector<string> *fields, vector<KVPair> &result) {
                          string orderer, peer, cmd;
  addresses(&orderer, &peer);
  if (sctype_ == BBUtils::SmartContractType::DoNothing) {
    cmd = "node fabric-v1.4-node/invoke_donothing.js "  + orderer + " " + peer;
  } else {
    cmd = "node fabric-v1.4-node/invoke_kv.js "  + orderer + " " + peer;
    cmd += " read " + key;
  }

  std::string result2 = exec(cmd.c_str());

  if (json_field(result2, "status") != "ok") {
    return DB::kErrorNoData;
  }
  string txn_hash = json_field(result2, "txID"); 
  txlock_->lock();
  (*pendingtx_)[txn_hash] = utils::time_now();
  txlock_->unlock();

  return DB::kOK;
}

// ignore table
// update value indicated by a key
int FabricDB::Update(const string &table, const string &key,
                          vector<KVPair> &values) {
  string orderer, peer, cmd;
  addresses(&orderer, &peer);
  if (sctype_ == BBUtils::SmartContractType::DoNothing) {
    cmd = "node fabric-v1.4-node/invoke_donothing.js "  + orderer + " " + peer;
  } else {
    string val = "\'";
    for (auto v : values) {
      val += v.first + "=" + v.second + "_";
    }
    for (auto &x : val) {
      if (x == '\"') x = '_';
    }
    val += "\'";
    cmd = "node fabric-v1.4-node/invoke_kv.js "  + orderer + " " + peer;
    cmd += " write " + key + " " + val;
  }
  //std::cout << "update key: " << key << std::endl;
  //uncomment
  std::string result = exec(cmd.c_str());
  //Place to add detected failures:
  if (json_field(result, "status") != "ok") {
    return DB::kErrorNoData;
  }
  string txn_hash = json_field(result, "txID"); 
  txlock_->lock();
  (*pendingtx_)[txn_hash] = utils::time_now();
  txlock_->unlock();
  return DB::kOK;
}

// ignore table
// ignore field
// concate values in KVPairs into one long value
int FabricDB::Insert(const string &table, const string &key,
                          vector<KVPair> &values) {
  return Update(table, key, values);
}

// ignore table
// delete value indicated by a key
int FabricDB::Delete(const string &table, const string &key) {
  vector<KVPair> empty_val;
  return Update(table, key, empty_val);
}

int FabricDB::GetTip() { 
  string orderer, peer;
  addresses(&orderer, &peer);
  string cmd = "node fabric-v1.4-node/query_blk_height.js " + orderer + " " + peer;
  string result = exec(cmd.c_str());
  string status = json_field(result, "status");
  //std::cout << "Get tip blk num Result: [" << result << "] status: " << "[" << status << "]" << std::endl;
  //uncomment
  if (status == "ok") {
    return stoi(json_field(result, "height"));
  } else {
    string msg = json_field(result, "msg");
    cout << "Get tip block number fails with err msg " << msg;
    return -1;
  }
}

// get all tx from the start_block until latest
vector<string> FabricDB::PollTxn(int block_number) {
  string orderer, peer;
  addresses(&orderer, &peer);
  string cmd = "node fabric-v1.4-node/poll_blk.js " + orderer + " " + peer + " " + std::to_string(block_number);
  string result = exec(cmd.c_str());
  
  // console output of transaction validation codes
  string cmd2 = "node fabric-v1.4-node/poll_metadata.js " + orderer + " " + peer + " " + std::to_string(block_number);
  string flag = exec(cmd2.c_str());

  auto valid = list_field(flag, "VALID");
  auto endorsement = list_field(flag, "ENDORSEMENT");
  auto mvcc = list_field(flag, "MVCC");
  auto phantom = list_field(flag, "PHANTOM");
  auto txs_sum = list_field(flag, "txs_sum");

  std::cout << "V: " << valid.size() << " E: " << endorsement.size() 
    << " M: " << mvcc.size() << " P: " << phantom.size() << std::endl;

    
  if (json_field(result, "status") == "ok") {
    auto r = list_field(result, "txns");
    return r; 
  } else {
    string msg = json_field(result, "msg");
    cout << "Polling txn fails with err msg " << msg;
    return vector<string>();
  }
}

vector<string> FabricDB::PollTxCodes(int block_number) {
  return vector<string>();
}

}  // ycsbc
