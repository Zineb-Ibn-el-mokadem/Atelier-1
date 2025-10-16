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

    cout << "\nVérification blockchain exercice 3: " << (myChain.isValid()?"✔ Valide\n":"✖ Invalide\n");