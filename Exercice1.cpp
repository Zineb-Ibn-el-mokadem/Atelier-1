

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <iomanip>
#include <functional>  // pour std::hash
#include <memory>
#include <sstream>
#include <cstdint>


using namespace std;


// Classe Node (nœud de l’arbre)
class Node {
public:
    string hash;
    Node* left;
    Node* right;

    Node(const string& h) : hash(h), left(nullptr), right(nullptr) {}
};



// Fonction de hachage légère inspirée de FNV-1a et MurmurHash
string fastSHA256(const string& data) {
    const uint64_t FNV_OFFSET = 1469598103934665603ULL;
    const uint64_t FNV_PRIME  = 1099511628211ULL;

    uint64_t hash1 = FNV_OFFSET;
    uint64_t hash2 = FNV_OFFSET ^ 0xDEADBEEFCAFEBABEULL;

    // Mélange double pour meilleure distribution
    for (unsigned char c : data) {
        hash1 ^= c;
        hash1 *= FNV_PRIME;
        hash1 ^= (hash1 >> 13);

        hash2 ^= (c + 0x9e3779b97f4a7c15ULL + (hash2 << 6) + (hash2 >> 2));
    }

    // Combine les deux en pseudo-SHA256 (64 caractères)
    stringstream ss;
    ss << hex << setw(16) << setfill('0') << (hash1 ^ (hash2 >> 7))
       << setw(16) << setfill('0') << (hash2 ^ (hash1 << 3))
       << setw(16) << setfill('0') << (~hash1)
       << setw(16) << setfill('0') << (~hash2);

    string result = ss.str();
    return result.substr(0, 64);
}

//  Classe MerkleTree
class MerkleTree {
private:
    vector<Node*> leaves;
    Node* root;

public:
    MerkleTree(const vector<string>& transactions) {
        buildTree(transactions);
    }

    // Construction de l’arbre
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
                if (i + 1 == currentLevel.size()) {
                    currentLevel.push_back(currentLevel[i]);
                }

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

    // Fonction pour obtenir la racine
    string getRootHash() const {
        if (!root) return "";
        return root->hash;
    }

  
    // Affichage de l’arbre de manière horizontale 
    void printTreeHorizontal(Node* node, int space = 0, int levelSpace = 6) const {
        if (!node) return;

        // Décalage vers la droite (impression en ordre inversé)
        space += levelSpace;

        // Afficher le sous-arbre droit d’abord
        printTreeHorizontal(node->right, space);

        // Afficher le nœud actuel
        cout << endl;
        for (int i = levelSpace; i < space; i++)
            cout << " ";
        cout << node->hash.substr(0, 6) << endl; // afficher juste 6 caractères du hash

        // Afficher le sous-arbre gauche
        printTreeHorizontal(node->left, space);
    }

    void display() const {
        cout << "\n===== Structure de l’Arbre de Merkle =====\n";
        printTreeHorizontal(root);
    }
};

int main() {
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

    cout << "=== Transactions ===" << endl;
    for (int i = 0; i < transactions.size(); i++) {
        cout << "Transaction " << i + 1 << ": " << transactions[i] << endl;
    }

    MerkleTree merkle(transactions);

    merkle.display();
    cout << "\nMerkle Root: " << merkle.getRootHash() << endl;

    return 0;
}










