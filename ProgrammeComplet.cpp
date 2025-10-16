
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

// Fonction de hachage rapide (réutilisée partout)

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


// Exercice 1 : Arbre de Merkle

class Node {
public:
    string hash;
    Node* left;
    Node* right;

    Node(const string& h) : hash(h), left(nullptr), right(nullptr) {}
};

class MerkleTree {
private:
    vector<Node*> leaves;
    Node* root;

public:
    MerkleTree(const vector<string>& transactions) {
        buildTree(transactions);
    }

    void buildTree(vector<string> transactions) {
        if (transactions.empty()) {
            root = nullptr;
            return;
        }

        for (const auto& tx : transactions)
            leaves.push_back(new Node(fastSHA256(tx)));

        vector<Node*> currentLevel = leaves;

        while (currentLevel.size() > 1) {
            vector<Node*> newLevel;

            for (size_t i = 0; i < currentLevel.size(); i += 2) {
                if (i + 1 == currentLevel.size()) currentLevel.push_back(currentLevel[i]);

                string combined = currentLevel[i]->hash + currentLevel[i + 1]->hash;
                Node* parent = new Node(fastSHA256(combined));
                parent->left = currentLevel[i];
                parent->right = currentLevel[i + 1];
                newLevel.push_back(parent);
            }
            currentLevel = newLevel;
        }
        root = currentLevel[0];
    }

    string getRootHash() const {
        return root ? root->hash : "";
    }

    void printTreeHorizontal(Node* node, int space = 0, int levelSpace = 6) const {
        if (!node) return;
        space += levelSpace;
        printTreeHorizontal(node->right, space);
        cout << endl;
        for (int i = levelSpace; i < space; i++) cout << " ";
        cout << node->hash.substr(0,6) << endl;
        printTreeHorizontal(node->left, space);
    }

    void display() const {
        cout << "\n===== Structure de l’Arbre de Merkle =====\n";
        printTreeHorizontal(root);
    }
};

void runExercice1() {
    vector<string> transactions = {
        "Zineb-> Merieme : 10 BTC",
        "Hamza -> Sara : 5 BTC",
        "Mehdi -> Yassir : 2 BTC",
        "Sara -> Karim : 1 BTC",
        "Siham->Salma : 8 BTC",
        "Reda -> Nora : 12 BTC",
        "Khouloud -> Soumaia : 6BTC",
        "Aya -> Zineb: 3 BTC"
    };
    cout << "=== Transactions ===\n";
    for (size_t i=0;i<transactions.size();++i)
        cout << "Transaction " << i+1 << ": " << transactions[i] << endl;

    MerkleTree merkle(transactions);
    merkle.display();
    cout << "\nMerkle Root: " << merkle.getRootHash() << endl;
}


// Exercice 2 : PoW simple
string calculateMerkleRoot(vector<string> txs) {
    if (txs.empty()) return string(64,'0');
    while (txs.size() > 1) {
        vector<string> next;
        next.reserve((txs.size()+1)/2);
        for (size_t i=0;i<txs.size();i+=2) {
            string left = txs[i];
            string right = (i+1<txs.size()?txs[i+1]:left);
            next.push_back(fastSHA256(left+right));
        }
        txs.swap(next);
    }
    return txs.front();
}

class Block {
public:
    int id;
    time_t timestamp;
    string prevHash;
    string merkleRoot;
    uint64_t nonce;
    string hash;

    Block(int id_, const string& prevHash_, const string& merkleRoot_)
        : id(id_), timestamp(time(nullptr)), prevHash(prevHash_), merkleRoot(merkleRoot_), nonce(0) { calculateHash(); }

    void calculateHash() { hash = fastSHA256(to_string(id)+to_string(timestamp)+prevHash+merkleRoot+to_string(nonce)); }

    void mineBlock(int difficulty) {
        string target(difficulty,'0');
        do { ++nonce; calculateHash(); } while(hash.compare(0,difficulty,target)!=0);
    }
};

void printBlockInfo(const Block& b) {
    cout << "Bloc ID: " << b.id << "\n";
    cout << "  Timestamp  : " << b.timestamp << "\n";
    cout << "  PrevHash   : " << b.prevHash.substr(0,20) << "...\n";
    cout << "  MerkleRoot : " << b.merkleRoot.substr(0,20) << "...\n";
    cout << "  Nonce      : " << b.nonce << "\n";
    cout << "  Hash       : " << b.hash << "\n";
}

void runExercice2() {
   
    cout << "\n==============================\n";
    cout << "EXERCICE 2 : Proof of Work (PoW)\n";
    cout << "==============================\n";

    vector<vector<string>> allTransactions = {
        {"Alice->Bob:3", "Charlie->Dave:2", "Eve->Frank:1"},
        {"Meriem->Sara:4", "Youssef->Nadia:2"},
        {"Omar->Zineb:5", "Rania->Laila:3", "Ali->Hassan:1"}
    };

    string prevHash = string(64, '0'); // Hash du bloc Genesis
    int difficulty = 3;
    int id = 1;
    long long totalTime = 0;

    for (auto &txs : allTransactions) {
        string merkle = calculateMerkleRoot(txs);
        Block b(id, prevHash, merkle);

        cout << "\n--- Minage du bloc #" << id << " ---\n";
        auto start = chrono::high_resolution_clock::now();
        b.mineBlock(difficulty);
        auto end = chrono::high_resolution_clock::now();
        long long duration = chrono::duration_cast<chrono::milliseconds>(end - start).count();
        totalTime += duration;

        printBlockInfo(b);
        cout << "⏱ Temps minage : " << duration << " ms\n";

        prevHash = b.hash;
        id++;
    }

    cout << "\n==============================\n";
    cout << "Tous les blocs PoW minés avec succès !\n";
    cout << " Temps total : " << totalTime << " ms\n";
    cout << "Difficulté utilisée : " << difficulty << "\n";
    cout << "==============================\n";

    
}

// Classes communes pour ex 3 et 4

class Transaction {
public:
    int id;
    string sender;
    string receiver;
    double amount;
    Transaction(int i,string s,string r,double a):id(i),sender(s),receiver(r),amount(a){}
    string toString() const { stringstream ss; ss<<id<<":"<<sender<<"->"<<receiver<<":"<<amount; return ss.str(); }
};

class BlockTx {
public:
    int id;
    time_t timestamp;
    string prevHash;
    string merkleRoot;
    uint64_t nonce;
    string validator;
    string hash;
    vector<Transaction> transactions;

    BlockTx(int i,string prev,const vector<Transaction>& txs)
        : id(i), timestamp(time(nullptr)), prevHash(prev), transactions(txs), nonce(0), validator("") {
        vector<string> txsStr;
        for(auto &tx: transactions) txsStr.push_back(tx.toString());
        merkleRoot = calculateMerkleRoot(txsStr);
        calculateHash();
    }

    void calculateHash() {
        string data = to_string(id)+to_string(timestamp)+prevHash+merkleRoot+to_string(nonce)+validator;
        hash = fastSHA256(data);
    }

    void mineBlock(int difficulty) { string target(difficulty,'0'); do { ++nonce; calculateHash(); } while(hash.compare(0,difficulty,target)!=0); }
    void validatePoS(const string& validatorName){validator=validatorName;calculateHash();}
};

class Blockchain {
public:
    vector<BlockTx> chain;
    Blockchain(){ vector<Transaction> genesisTx = {Transaction(0,"Genesis","Network",0)}; chain.push_back(BlockTx(0,string(64,'0'),genesisTx)); }
    void addBlock(BlockTx& b){ chain.push_back(b); }
    bool isValid(){ for(size_t i=1;i<chain.size();++i){ if(chain[i].prevHash!=chain[i-1].hash) return false; BlockTx tmp=chain[i]; tmp.calculateHash(); if(tmp.hash!=chain[i].hash) return false; } return true; }
    void printBlock(const BlockTx& b){
        cout<<"Bloc ID: "<<b.id<<"\n  Timestamp: "<<b.timestamp<<"\n  PrevHash: "<<b.prevHash.substr(0,20)<<"...\n  MerkleRoot: "<<b.merkleRoot.substr(0,20)<<"...\n  Nonce: "<<b.nonce<<"\n  Validator: "<<(b.validator.empty()?"N/A":b.validator)<<"\n  Hash: "<<b.hash.substr(0,20)<<"...\n  Transactions:\n";
        for(auto& tx:b.transactions) cout<<"    "<<tx.toString()<<"\n";
        cout<<"--------------------------------------\n";
    }
};

class PoSSystem {
public:
    vector<pair<string,int>> validators;
    PoSSystem(){ validators={{"Alice",50},{"Bob",30},{"Charlie",20}}; }
    string chooseValidator(){ int totalStake=0; for(auto& v:validators) totalStake+=v.second; int rnd=rand()%totalStake; for(auto& v:validators){ if(rnd<v.second) return v.first; rnd-=v.second;} return validators[0].first;}
    long long simulatePoS(BlockTx& block){ auto start=high_resolution_clock::now(); block.validatePoS(chooseValidator()); auto end=high_resolution_clock::now(); return duration_cast<milliseconds>(end-start).count();}
};

long long simulatePoW(BlockTx& block,int difficulty){ auto start=high_resolution_clock::now(); block.mineBlock(difficulty); auto end=high_resolution_clock::now(); return duration_cast<milliseconds>(end-start).count(); }


// Exercice 3 : PoW + PoS simplifié
void runExercice3() {
    Blockchain myChain;
    PoSSystem posSystem;

    vector<Transaction> txs1 = {Transaction(1,"Zineb","Merieme",10), Transaction(2,"Hamza","Sara",5)};
    BlockTx block1(myChain.chain.back().id+1, myChain.chain.back().hash, txs1);

    cout << "\n--- Simulation PoW sur 1 bloc ---\n";
    long long tPow = simulatePoW(block1,3);
    myChain.addBlock(block1);
    myChain.printBlock(block1);
    cout << "Temps minage PoW: " << tPow << " ms\n";

    vector<Transaction> txs2 = {Transaction(3,"Ali","Laila",7)};
    BlockTx block2(myChain.chain.back().id+1, myChain.chain.back().hash, txs2);

    cout << "\n--- Simulation PoS sur 1 bloc ---\n";
    long long tPos = posSystem.simulatePoS(block2);
    myChain.addBlock(block2);
    myChain.printBlock(block2);
    cout << "Temps validation PoS: " << tPos << " ms\n";
    // Comparaison
    cout << "\n===== Comparaison des temps d'exécution =====\n";
    cout << "PoW: " << tPow << " ms, PoS: " << tPos << " ms\n";
    cout << "Le plus rapide: " << (tPos < tPow? "PoS" : "PoW") << endl;


    cout << "\nVérification blockchain exercice 3: " << (myChain.isValid()?"✔ Valide\n":"✖ Invalide\n");
}


// Exercice 4 : Blockchain complète + comparaison PoW/PoS
void runExercice4() {
    Blockchain myChain;
    PoSSystem posSystem;
    vector<vector<Transaction>> listTxs = {
        {Transaction(1,"Zineb","Merieme",10),Transaction(2,"Hamza","Sara",5)},
        {Transaction(3,"Yassine","Hajar",2),Transaction(4,"Mouad","Zineb",1)},
        {Transaction(5,"Ali","Laila",7),Transaction(6,"Sara","Hamza",3)},
        {Transaction(7,"Ahmed","Omar",8),Transaction(8,"Nora","Yassine",2)}
    };

    int difficulty=3;
    long long totalPoWTime=0,totalPoSTime=0;

    cout<<"\n===== Ajout blocs PoW =====\n";
    for(size_t i=0;i<listTxs.size();++i){
        BlockTx b(myChain.chain.back().id+1,myChain.chain.back().hash,listTxs[i]);
        long long t=simulatePoW(b,difficulty); totalPoWTime+=t;
        myChain.addBlock(b); cout<<"Bloc PoW ajouté:\n"; myChain.printBlock(b); cout<<"Temps minage PoW: "<<t<<" ms\n";
    }

    cout<<"\n===== Ajout blocs PoS =====\n";
    for(size_t i=0;i<listTxs.size();++i){
        BlockTx b(myChain.chain.back().id+1,myChain.chain.back().hash,listTxs[i]);
        long long t=posSystem.simulatePoS(b); totalPoSTime+=t;
        myChain.addBlock(b); cout<<"Bloc PoS ajouté:\n"; myChain.printBlock(b); cout<<"Temps validation PoS: "<<t<<" ms\n";
    }

    cout<<"\n===== Vérification Blockchain =====\n";
    cout<<(myChain.isValid()?"✔ Blockchain valide\n":"✖ Blockchain invalide\n");

    cout<<"\n===== Analyse Comparative =====\n";
    cout<<left<<setw(20)<<"Critère"<<setw(15)<<"PoW"<<setw(15)<<"PoS"<<endl;
    cout<<string(50,'-')<<endl;
    double avgPoW=(double)totalPoWTime/listTxs.size(), avgPoS=(double)totalPoSTime/listTxs.size();
    cout<<setw(20)<<"Temps moyen/bloc (ms)"<<setw(15)<<avgPoW<<setw(15)<<avgPoS<<endl;
    uint64_t totalPoWNonces=0; for(size_t i=1;i<=listTxs.size();++i) totalPoWNonces+=myChain.chain[i].nonce;
    cout<<setw(20)<<"Approx. ressources"<<setw(15)<<totalPoWNonces<<setw(15)<<"faible"<<endl;
    cout<<setw(20)<<"Facilité implémentation"<<setw(15)<<"Complexe"<<setw(15)<<"Simple"<<endl;
    cout<<"\nBloc le plus rapide: "<<(totalPoSTime<totalPoWTime?"PoS":"PoW")<<endl;
}


// Menu principal
int main() {
    srand(static_cast<unsigned>(time(nullptr)));
    int choice;
    do {
        cout<<"\n===== MENU PRINCIPAL =====\n";
        cout<<"1. Exercice 1 : Arbre de Merkle\n";
        cout<<"2. Exercice 2 : Proof-of-Work simple\n";
        cout<<"3. Exercice 3 : PoW + PoS simplifié\n";
        cout<<"4. Exercice 4 : Blockchain avec transactions et comparaison PoW/PoS\n";
        cout<<"0. Quitter\n";
        cout<<"Votre choix: "; cin>>choice;

        switch(choice) {
            case 1: runExercice1(); break;
            case 2: runExercice2(); break;
            case 3: runExercice3(); break;
            case 4: runExercice4(); break;
            case 0: cout<<"Au revoir!\n"; break;
            default: cout<<"Option invalide!\n";
        }
    } while(choice!=0);
    return 0;
}



