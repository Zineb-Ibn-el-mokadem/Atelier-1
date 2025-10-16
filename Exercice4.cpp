
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <cstdint>
#include <random>

using namespace std;
using namespace chrono;


// Classe Transaction

class Transaction {
public:
    int id;
    string sender;
    string receiver;
    double amount;

    Transaction(int i, string s, string r, double a)
        : id(i), sender(s), receiver(r), amount(a) {}

    string toString() const {
        stringstream ss;
        ss << id << ":" << sender << "->" << receiver << ":" << amount;
        return ss.str();
    }
};


// Hash simple 

string fastSHA256(const string& data) {
    const uint64_t FNV_OFFSET = 1469598103934665603ULL;
    const uint64_t FNV_PRIME = 1099511628211ULL;
    uint64_t h1 = FNV_OFFSET, h2 = FNV_OFFSET ^ 0xCAFEBABEDEADBEEFULL;
    for (unsigned char c : data) {
        h1 ^= c; h1 *= FNV_PRIME; h1 ^= (h1 >> 12);
        h2 ^= (uint64_t)c + 0x9e3779b97f4a7c15ULL + (h2 << 6) + (h2 >> 2);
    }
    stringstream ss;
    ss << hex << nouppercase << setfill('0')
       << setw(16) << (h1 ^ (h2 >> 7))
       << setw(16) << (h2 ^ (h1 << 3))
       << setw(16) << (~h1)
       << setw(16) << (~h2);
    string out = ss.str();
    if (out.size() < 64) out.append(64 - out.size(), '0');
    return out.substr(0, 64);
}


// Merkle Tree

string calculateMerkleRoot(const vector<Transaction>& txs) {
    if (txs.empty()) return string(64, '0');

    vector<string> hashes;
    for (auto& tx : txs) hashes.push_back(fastSHA256(tx.toString()));

    while (hashes.size() > 1) {
        vector<string> next;
        for (size_t i = 0; i < hashes.size(); i += 2) {
            string left = hashes[i];
            string right = (i + 1 < hashes.size()) ? hashes[i + 1] : left;
            next.push_back(fastSHA256(left + right));
        }
        hashes.swap(next);
    }
    return hashes.front();
}


// Classe Block

class Block {
public:
    int id;
    time_t timestamp;
    string prevHash;
    string merkleRoot;
    uint64_t nonce;
    string validator; // pour PoS
    string hash;
    vector<Transaction> transactions;

    Block(int i, const string& prev, const vector<Transaction>& txs)
        : id(i), timestamp(time(nullptr)), prevHash(prev), transactions(txs), nonce(0), validator("") {
        merkleRoot = calculateMerkleRoot(transactions);
        calculateHash();
    }

    void calculateHash() {
        string data = to_string(id) + to_string(timestamp) + prevHash + merkleRoot + to_string(nonce) + validator;
        hash = fastSHA256(data);
    }

    void mineBlock(int difficulty) {
        string target(difficulty, '0');
        do { ++nonce; calculateHash(); } while (hash.compare(0, difficulty, target) != 0);
    }

    void validatePoS(const string& validatorName) {
        validator = validatorName;
        calculateHash();
    }
};


// Classe Blockchain

class Blockchain {
public:
    vector<Block> chain;

    Blockchain() { 
        vector<Transaction> genesisTxs = { Transaction(0,"Genesis","Network",0) };
        chain.push_back(Block(0,string(64,'0'),genesisTxs));
    }

    void addBlock(Block& b) { chain.push_back(b); }

    bool isValid() {
        for (size_t i = 1; i < chain.size(); ++i) {
            if (chain[i].prevHash != chain[i-1].hash) return false;
            Block tmp = chain[i];
            tmp.calculateHash();
            if (tmp.hash != chain[i].hash) return false;
        }
        return true;
    }

    void printBlock(const Block& b) {
        cout << "Bloc ID: " << b.id << "\n";
        cout << "  Timestamp : " << b.timestamp << "\n";
        cout << "  PrevHash  : " << b.prevHash.substr(0,20) << "...\n";
        cout << "  MerkleRoot: " << b.merkleRoot.substr(0,20) << "...\n";
        cout << "  Nonce     : " << b.nonce << "\n";
        cout << "  Validator : " << (b.validator.empty()?"N/A":b.validator) << "\n";
        cout << "  Hash      : " << b.hash.substr(0,20) << "...\n";
        cout << "  Transactions: \n";
        for (auto& tx: b.transactions) cout << "    " << tx.toString() << "\n";
        cout << "--------------------------------------\n";
    }
};


// PoS System

class PoSSystem {
public:
    vector<pair<string,int>> validators; // nom, stake
    PoSSystem() { validators = {{"Alice",50},{"Bob",30},{"Charlie",20}}; }

    string chooseValidator() {
        int totalStake = 0;
        for (auto& v : validators) totalStake += v.second;
        int rnd = rand() % totalStake;
        for (auto& v : validators) {
            if (rnd < v.second) return v.first;
            rnd -= v.second;
        }
        return validators[0].first;
    }

    long long simulatePoS(Block& block) {
        auto start = high_resolution_clock::now();
        block.validatePoS(chooseValidator());
        auto end = high_resolution_clock::now();
        return duration_cast<milliseconds>(end-start).count();
    }
};


// Simulation PoW

long long simulatePoW(Block& block, int difficulty) {
    auto start = high_resolution_clock::now();
    block.mineBlock(difficulty);
    auto end = high_resolution_clock::now();
    return duration_cast<milliseconds>(end-start).count();
}


// Fonction principale

int main() {
    ios::sync_with_stdio(false); cin.tie(nullptr);
    srand(time(nullptr));

    Blockchain myChain;
    PoSSystem posSystem;

    // Transactions exemples pour plusieurs blocs
    vector<vector<Transaction>> listTxs = {
        { Transaction(1,"Zineb","Merieme",10), Transaction(2,"Hamza","Sara",5) },
        { Transaction(3,"Yassine","Hajar",2), Transaction(4,"Mouad","Zineb",1) },
        { Transaction(5,"Ali","Laila",7), Transaction(6,"Sara","Hamza",3) },
        { Transaction(7,"Ahmed","Omar",8), Transaction(8,"Nora","Yassine",2) }
    };

    
    // Partie PoW : ajouter plusieurs blocs
    
    cout << "\n===== Ajout blocs PoW =====\n";
    int difficulty = 3;
    long long totalPoWTime = 0;
    for (size_t i=0; i<listTxs.size(); ++i) {
        Block b(myChain.chain.back().id + 1, myChain.chain.back().hash, listTxs[i]);
        long long t = simulatePoW(b,difficulty);
        totalPoWTime += t;
        myChain.addBlock(b);
        cout << "Bloc PoW ajouté :\n";
        myChain.printBlock(b);
        cout << "Temps minage PoW: " << t << " ms\n";
    }

    
    // Partie PoS : ajouter plusieurs blocs
   
    cout << "\n===== Ajout blocs PoS =====\n";
    long long totalPoSTime = 0;
    for (size_t i=0; i<listTxs.size(); ++i) {
        Block b(myChain.chain.back().id + 1, myChain.chain.back().hash, listTxs[i]);
        long long t = posSystem.simulatePoS(b);
        totalPoSTime += t;
        myChain.addBlock(b);
        cout << "Bloc PoS ajouté :\n";
        myChain.printBlock(b);
        cout << "Temps validation PoS: " << t << " ms\n";
    }

    
    // Vérification intégrité
    
    cout << "\n===== Vérification Blockchain =====\n";
    cout << (myChain.isValid() ? "✔ Blockchain valide\n" : "✖ Blockchain invalide\n");

    
        // Comparaison détaillée PoW vs PoS
       
        cout << "\n===== Analyse Comparative =====\n";
        cout << left << setw(20) << "Critère"
            << setw(15) << "PoW"
            << setw(15) << "PoS" << endl;
        cout << string(50,'-') << endl;

        // Rapidité d'ajout des blocs
        double avgPoW = (double)totalPoWTime / listTxs.size();
        double avgPoS = (double)totalPoSTime / listTxs.size();
        cout << setw(20) << "Temps moyen/bloc (ms)"
            << setw(15) << avgPoW
            << setw(15) << avgPoS << endl;

        // Consommation ressources (approx)
        uint64_t totalPoWNonces = 0;
        for (size_t i=1;i<=listTxs.size();++i) totalPoWNonces += myChain.chain[i].nonce;
        cout << setw(20) << "Approx. ressources"
            << setw(15) << totalPoWNonces
            << setw(15) << "faible" << endl;

        // Facilité de mise en œuvre (qualitative)
        cout << setw(20) << "Facilité implémentation"
            << setw(15) << "Complexe"
            << setw(15) << "Simple" << endl;

        // Résumé rapide
        cout << "\nBloc le plus rapide: " << (totalPoSTime < totalPoWTime ? "PoS" : "PoW") << endl;


    return 0;
}
