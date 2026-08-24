#ifdef GENETICBITSTRING_H
#define GENETICBITSTRING_H

std::vector<uint32_t> findAggressiveSeeds(uint32_t lowerBound, uint32_t upperBound) {
    initMasterLUT();
    std::vector<uint32_t> aggroSeeds;
    for(uint32_t i = lowerBound; i < upperBound; ++i) {
        if((i+1) % 10000 == 0) {
            std::cout << "Completed: " << std::to_string(i+1) << std::endl;
        }
        std::bitset<BITSTRING_LENGTH> bitstring = generateRandomBitstring(i);
        Game game = initGame();
        Game nextGame;
        for(uint8_t moves = 0; moves < 10; ++moves) {
            int64_t eval = 0;
            Move bestMove = getBestMoveBitstring(bitstring, game, eval);
            doMove(game, nextGame, bestMove);
            game = nextGame;
            game.isWhiteTurn = 1;
        }
        // if(evaluateGame(game) > 6) {
        //     aggroSeeds.push_back(i);
        //     continue;
        // }
        game = initGame();
        for(uint8_t moves = 0; moves < 10; ++moves) {
            int64_t eval = 0;
            game.isWhiteTurn = 0;
            Move bestMove = getBestMoveBitstring(bitstring, game, eval);
            doMove(game, nextGame, bestMove);
            game = nextGame;
        }
        // if(evaluateGame(game) < -6) {
        //     aggroSeeds.push_back(i);
        // }
    }
    return aggroSeeds;
}

std::bitset<BITSTRING_LENGTH> mutate(std::bitset<BITSTRING_LENGTH> bitstring, float flipChance, uint32_t seed) {
    std::mt19937 generator {seed};
    std::uniform_real_distribution<float> distribution(0.0f, 1.0f); 
    for(int i = 0; i < BITSTRING_LENGTH; i++) {
        if(distribution(generator) < flipChance) {
            bitstring[i] = ~bitstring[i];
        }
    }
    return bitstring;
}

// 1 means white won
// 2 means black won
uint8_t runMatch(std::bitset<BITSTRING_LENGTH> bitstringW, std::bitset<BITSTRING_LENGTH> bitstringB, int maxMoves) {
    Game game = initGame();
    Game nextGame;
    for(int i = 0; i < maxMoves; i++) {
        int64_t eval = 0;
        doMove(game, nextGame, getBestMoveBitstring(bitstringW, game, eval));
        game = nextGame;
        doMove(game, nextGame, getBestMoveBitstring(bitstringB, game, eval));
        game = nextGame;
    }
    // if(evaluateGame(game) > 0) {
    //     return 1;
    // }
    // else if(evaluateGame(game) < 0) {
    //     return 2;
    // }
    // else {
    //     return 2;
    // }
    std::cout << "something went wrong in runmatch" << std::endl;
    return 0;
}

std::vector<std::bitset<BITSTRING_LENGTH>> initGeneration(int size) {
    std::vector<std::bitset<BITSTRING_LENGTH>> generation;
    for(int i = 0; i < size; ++i) {
        generation.push_back(generateRandomBitstring(i));
    }
    return generation;
}

std::vector<std::bitset<BITSTRING_LENGTH>> runGeneration(std::vector<std::bitset<BITSTRING_LENGTH>> generation, float mutationRate, uint32_t seed) {
    std::mt19937 generator {seed};
    std::shuffle(generation.begin(), generation.end(), generator);
    std::vector<std::bitset<BITSTRING_LENGTH>> nextGeneration;
    for(int i = 1; i < generation.size(); i += 2) {
        uint8_t matchResult =  runMatch(generation[i-1], generation[i], 50);
        if(matchResult == 1) {
            nextGeneration.push_back(generation[i-1]);
            nextGeneration.push_back(mutate(generation[i-1], mutationRate, i+seed));
        }
        else {
            nextGeneration.push_back(generation[i]);
            nextGeneration.push_back(mutate(generation[i], mutationRate, i+seed));
        }
    }
    return nextGeneration;
}

#endif
