
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
using namespace std::chrono;

// Implémentation de fastSHA256 
string fastSHA256(const string& data) {
    const uint64_t FNV_OFFSET = 1469598103934665603ULL;
    const uint64_t FNV_PRIME = 1099511628211ULL;

    uint64_t h1 = FNV_OFFSET;
    uint64_t h2 = FNV_OFFSET ^ 0xCAFEBABEDEADBEEFULL;

    for (unsigned char c : data) {
        h1 ^= c;
        h1 *= FNV_PRIME;
        h1 ^= (h1 >> 12);

        h2 ^= (uint64_t)c + 0x9e3779b97f4a7c15ULL + (h2 << 6) + (h2 >> 2);
    }

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

// Calcul de la racine Merkle 
string calculateMerkleRoot(vector<string> txs) {
    if (txs.empty()) return string(64, '0');

    while (txs.size() > 1) {
        vector<string> next;
        next.reserve((txs.size() + 1) / 2);
        for (size_t i = 0; i < txs.size(); i += 2) {
            string left = txs[i];
            string right = (i + 1 < txs.size()) ? txs[i + 1] : left;
            next.push_back(fastSHA256(left + right));
        }
        txs.swap(next);
    }
    return txs.front();
}

// Classe Block 
class Block {
public:
    int id;
    time_t timestamp;
    string prevHash;
    string merkleRoot;
    uint64_t nonce;
    string validator; 
    string hash;

    Block(int id_, const string& prevHash_, const string& merkleRoot_)
        : id(id_), timestamp(time(nullptr)), prevHash(prevHash_), merkleRoot(merkleRoot_), nonce(0), validator("") {
        calculateHash();
    }

    void calculateHash() {
        string data = to_string(id) + to_string(timestamp) + prevHash + merkleRoot + to_string(nonce) + validator;
        hash = fastSHA256(data);
    }

    // Méthode pour miner avec PoW
    void mineBlock(int difficulty) {
        string target(difficulty, '0');
        do {
            ++nonce;
            calculateHash();
        } while (hash.compare(0, difficulty, target) != 0);
    }

    // Méthode pour valider avec PoS
    void validateWithPoS(const string& validatorName) {
        validator = validatorName;
        calculateHash(); // Pas de minage, juste mise à jour du hash avec le validateur
    }
};

// Classe Validator pour PoS
class Validator {
public:
    string name;
    int stake;

    Validator(string n, int s) : name(n), stake(s) {}
};

// Classe PoS pour sélectionner un validateur
class PoS {
private:
    vector<Validator> validators;

public:
    PoS() {
        validators = {{"Val1", 50}, {"Val2", 30}, {"Val3", 20}}; // Stakes proportionnels
    }

    string selectValidator() {
        int totalStake = 0;
        for (const auto& v : validators) totalStake += v.stake;
        int random = rand() % totalStake;
        for (const auto& v : validators) {
            if (random < v.stake) return v.name;
            random -= v.stake;
        }
        return validators[0].name;
    }
};

// Fonction d'affichage (réutilisée depuis votre code)
void printBlockInfo(const Block& b) {
    cout << "Bloc ID: " << b.id << "\n";
    cout << "  Timestamp  : " << b.timestamp << "\n";
    cout << "  PrevHash   : " << b.prevHash.substr(0, 20) << "...\n";
    cout << "  MerkleRoot : " << b.merkleRoot.substr(0, 20) << "...\n";
    cout << "  Nonce      : " << b.nonce << "\n";
    cout << "  Validator  : " << (b.validator.empty() ? "N/A" : b.validator) << "\n";
    cout << "  Hash       : " << b.hash.substr(0, 20) << "...\n";
}

// Fonction pour simuler et mesurer le temps de validation
long long simulateValidation(Block& block, int difficulty, bool usePoS, PoS& pos) {
    auto t_start = high_resolution_clock::now();
    if (usePoS) {
        block.validateWithPoS(pos.selectValidator());
    } else {
        block.mineBlock(difficulty);
    }
    auto t_end = high_resolution_clock::now();
    return duration_cast<milliseconds>(t_end - t_start).count();
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    srand(static_cast<unsigned>(time(nullptr))); // Initialisation pour PoS

    cout << "===== Simulation Proof-of-Stake (PoS) et Proof-of-Work (PoW) =====\n\n";

    // 1) Créer la blockchain avec un bloc genesis
    vector<Block> blockchain;
    vector<string> genesisTx = {"Genesis: Zineb->Merieme:10 BTC", "Genesis: Hamza->Sara:5 BTC"};
    string genesisMerkle = calculateMerkleRoot(genesisTx);
    Block genesis(0, string(64, '0'), genesisMerkle);
    blockchain.push_back(genesis);

    cout << "Bloc genesis créé.\n";
    printBlockInfo(blockchain[0]);
    cout << "-------------------------------------------\n";

    // 2) Simuler un nouveau bloc avec PoW et PoS
    PoS posSystem;
    vector<string> txs = {"Hamza->Sara:5 BTC", "Yassine->Hajar:2 BTC", "Mouad->Zineb:1 BTC"};
    string merkle = calculateMerkleRoot(txs);
    Block powBlock(1, blockchain.back().hash, merkle);
    Block posBlock(2, blockchain.back().hash, merkle);

    // Mesurer PoW
    long long powTime = simulateValidation(powBlock, 2, false, posSystem);
    blockchain.push_back(powBlock);
    cout << "\n--> Validation avec PoW (difficulté 2):\n";
    printBlockInfo(powBlock);
    cout << "Temps d'exécution PoW: " << powTime << " ms\n";

    // Mesurer PoS
    long long posTime = simulateValidation(posBlock, 2, true, posSystem);
    blockchain.push_back(posBlock);
    cout << "\n--> Validation avec PoS:\n";
    printBlockInfo(posBlock);
    cout << "Temps d'exécution PoS: " << posTime << " ms\n";

    // Comparaison
    cout << "\n===== Comparaison des temps d'exécution =====\n";
    cout << "PoW: " << powTime << " ms, PoS: " << posTime << " ms\n";
    cout << "Le plus rapide: " << (posTime < powTime ? "PoS" : "PoW") << endl;

    // Vérification de la validité
    bool valid = true;
    for (size_t i = 1; i < blockchain.size(); ++i) {
        if (blockchain[i].prevHash != blockchain[i - 1].hash) {
            valid = false;
            break;
        }
        Block tmp = blockchain[i];
        tmp.calculateHash();
        if (tmp.hash != blockchain[i].hash) {
            valid = false;
            break;
        }
    }
    cout << "\n===== Vérification de la blockchain =====\n";
    cout << (valid ? "✔ Blockchain valide\n" : "✖ Blockchain invalide\n");

    return 0;
}