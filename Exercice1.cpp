#include <iostream>
#include <vector>
#include <string>
#include <functional>  // pour std::hash

using namespace std;

// Fonction pour simuler un hash (ici on utilise std::hash pour simplifier)
// string simpleHash(const string &data) {
//     hash<string> hasher;
//     size_t hashValue = hasher(data);
//     return to_string(hashValue);
// }

string simpleHash(const string &data) {
    unsigned long long hash = 0;

    // Algorithme de hachage simple inspiré de djb2
    for (char c : data) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }

    // Convertir le nombre obtenu en chaîne hexadécimale
    string hex = "";
    const char *hexChars = "0123456789abcdef";
    while (hash > 0) {
        hex = hexChars[hash % 16] + hex;
        hash /= 16;
    }

    if (hex == "") hex = "0"; // sécurité pour éviter une chaîne vide
    return hex;
}

// Fonction pour construire un arbre de Merkle et retourner la racine
string buildMerkleRoot(vector<string> transactions) {
    if (transactions.empty()) return "";

    // Étape 1 : convertir chaque transaction en hash
    vector<string> level;
    for (const auto &tx : transactions) {
        level.push_back(simpleHash(tx));
    }

    // Étape 2 : construire l'arbre jusqu'à la racine
    while (level.size() > 1) {
        vector<string> newLevel;

        for (size_t i = 0; i < level.size(); i += 2) {
            // si nombre impair, on duplique le dernier hash
            if (i + 1 == level.size()) {
                level.push_back(level[i]);
            }

            // concaténer deux enfants et re-hasher
            string combined = level[i] + level[i + 1];
            newLevel.push_back(simpleHash(combined));
        }

        level = newLevel;  // passer au niveau supérieur
    }

    // Étape 3 : la racine est le seul élément restant
    return level[0];
}

int main() {
    // Exemple de transactions
    vector<string> transactions = {
        "Alice -> Bob : 10 BTC",
        "Bob -> Charlie : 5 BTC",
        "Charlie -> Dave : 2 BTC",
        "Dave -> Eve : 1 BTC"
    };

    cout << "=== Arbre de Merkle ===" << endl;
    for (int i = 0; i < transactions.size(); i++) {
        cout << "Transaction " << i+1 << ": " << transactions[i] << endl;
    }

    string merkleRoot = buildMerkleRoot(transactions);

    cout << "\nMerkle Root: " << merkleRoot << endl;

    return 0;
}
