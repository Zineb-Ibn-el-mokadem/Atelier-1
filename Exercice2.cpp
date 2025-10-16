// main.cpp
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <cstdint>

using namespace std;
using namespace std::chrono;


string fastSHA256(const string& data) {
    const uint64_t FNV_OFFSET = 1469598103934665603ULL;
    const uint64_t FNV_PRIME  = 1099511628211ULL;

    uint64_t h1 = FNV_OFFSET;
    uint64_t h2 = FNV_OFFSET ^ 0xCAFEBABEDEADBEEFULL;

    for (unsigned char c : data) {
        h1 ^= c;
        h1 *= FNV_PRIME;
        h1 ^= (h1 >> 12);

        h2 ^= (uint64_t)c + 0x9e3779b97f4a7c15ULL + (h2 << 6) + (h2 >> 2);
    }

    // Concatène 4 morceaux de 16 hex pour obtenir ~64 hex digits
    std::ostringstream ss;
    ss << std::hex << std::nouppercase << std::setfill('0')
       << std::setw(16) << (h1 ^ (h2 >> 7))
       << std::setw(16) << (h2 ^ (h1 << 3))
       << std::setw(16) << (~h1)
       << std::setw(16) << (~h2);

    string out = ss.str();
    if (out.size() < 64) out.append(64 - out.size(), '0');
    return out.substr(0, 64);
}


string calculateMerkleRoot(vector<string> txs) {
    if (txs.empty()) return string(64, '0'); // valeur neutre si aucune tx

    while (txs.size() > 1) {
        vector<string> next;
        next.reserve((txs.size() + 1) / 2);
        for (size_t i = 0; i < txs.size(); i += 2) {
            string left = txs[i];
            string right = (i + 1 < txs.size()) ? txs[i + 1] : left; // dupliquer si impair
            next.push_back(fastSHA256(left + right));
        }
        txs.swap(next);
    }
    return txs.front();
}


//  Classe Block
   
class Block {
public:
    int id;
    time_t timestamp;
    string prevHash;
    string merkleRoot;
    uint64_t nonce;
    string hash;

    Block(int id_, const string& prevHash_, const string& merkleRoot_)
        : id(id_), timestamp(time(nullptr)), prevHash(prevHash_), merkleRoot(merkleRoot_), nonce(0) {
        calculateHash();
    }

    void calculateHash() {
        // Construction de la donnée à hacher
        string data = to_string(id) + to_string(timestamp) + prevHash + merkleRoot + to_string(nonce);
        hash = fastSHA256(data);
    }

    // Miner le bloc : trouver un hash commençant par `difficulty` zéros hex
    void mineBlock(int difficulty) {
        string target(difficulty, '0');
        // optimisation : utiliser compare (évite création de sous-chaîne dans if)
        do {
            ++nonce;
            calculateHash();
        } while (hash.compare(0, difficulty, target) != 0);
    }
};


//Fonction d'affichage d'un bloc
   
void printBlockInfo(const Block& b) {
    cout << "Bloc ID: " << b.id << "\n";
    cout << "  Timestamp  : " << b.timestamp << "\n";
    cout << "  PrevHash   : " << b.prevHash.substr(0, 20) << "...\n";
    cout << "  MerkleRoot : " << b.merkleRoot.substr(0, 20) << "...\n";
    cout << "  Nonce      : " << b.nonce << "\n";
    cout << "  Hash       : " << b.hash << "\n";
}


// Fonction pour simuler PoW et retourner le temps total en ms
long long simulatePoW(int numBlocks, int difficulty) {
    vector<Block> blockchain;
    vector<string> genesisTx = {"Genesis: Alice->Bob:10", "Genesis: Bob->Charlie:5"};
    string genesisMerkle = calculateMerkleRoot(genesisTx);
    Block genesis(0, string(64,'0'), genesisMerkle);
    blockchain.push_back(genesis);

    auto t_start = high_resolution_clock::now();

    for (int i=1; i<=numBlocks; i++) {
        vector<string> txs = {"Alice->Bob:3", "Charlie->Dave:2", "Eve->Frank:1"};
        string merkle = calculateMerkleRoot(txs);
        Block newBlock(i, blockchain.back().hash, merkle);

        newBlock.mineBlock(difficulty);
        blockchain.push_back(newBlock);
    }

    auto t_end = high_resolution_clock::now();
    return duration_cast<milliseconds>(t_end - t_start).count();
}


// Main : simulation PoW
  
int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cout << "===== Simulation Proof-of-Work (PoW) =====\n\n";

    // 1) Créer la blockchain et le bloc genesis
    vector<Block> blockchain;
    vector<string> genesisTx = { "Genesis: Zineb->Merieme:10 BTC", "Genesis: Hamza->Sara:5 BTC" };
    string genesisMerkle = calculateMerkleRoot(genesisTx);
    Block genesis(0, string(64, '0'), genesisMerkle);
    blockchain.push_back(genesis);

    cout << "Bloc genesis créé.\n";
    printBlockInfo(blockchain[0]);
    cout << "-------------------------------------------\n";

    // 2) Niveaux de difficulté à tester 
    vector<int> difficulties = {2, 3, 4};

    for (int diff : difficulties) {
        cout << "\n--> Minage d'un nouveau bloc avec difficulty = " << diff << "\n";

        // 2.a) Créer quelques transactions 
        vector<string> txs = {
            "Hamza->Sara:5 BTC",
            "Yassine->Hajar:2 BTC",
            "Mouad->Zineb:1 BTC"
        };
        string merkle = calculateMerkleRoot(txs);

        // 2.b) Créer le bloc avec prevHash = hash du dernier bloc
        Block newBlock(static_cast<int>(blockchain.size()), blockchain.back().hash, merkle);

        // 2.c) Miner et mesurer le temps
        auto t0 = chrono::high_resolution_clock::now();
        newBlock.mineBlock(diff);
        auto t1 = chrono::high_resolution_clock::now();
        long long ms = chrono::duration_cast<chrono::milliseconds>(t1 - t0).count();

        // 2.d) Afficher résultats
        cout << "Bloc miné en " << ms << " ms\n";
        cout << "Nonce trouvé: " << newBlock.nonce << "\n";
        cout << "Hash (prefix): " << newBlock.hash.substr(0, diff + 6) << "...\n";

        blockchain.push_back(newBlock);
    }

    // 3) Vérifier la validité de la blockchain
    bool valid = true;
    for (size_t i = 1; i < blockchain.size(); ++i) {
        if (blockchain[i].prevHash != blockchain[i - 1].hash) {
            valid = false;
            break;
        }
        // vérifier que le hash du bloc est bien recalculable (optionnel)
        Block tmp = blockchain[i];
        tmp.calculateHash();
        if (tmp.hash != blockchain[i].hash) {
            valid = false;
            break;
        }
    }

    cout << "\n===== Vérification de la blockchain =====\n";
    cout << (valid ? "✔ Blockchain valide\n" : "✖ Blockchain invalide\n");

    cout << "\nAffichage final des blocs :\n";
    for (const auto& b : blockchain) {
        cout << "-------------------------------------------\n";
        printBlockInfo(b);
    }

    return 0;
}
