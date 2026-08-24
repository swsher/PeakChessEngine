#ifndef BRUTEFORCEBITSTRING_H
#define BRUTEFORCEBITSTRING_H

#include "bitstring.h"

float getBitstringScore(std::bitset<BITSTRING_LENGTH> bitstring, const Game &game, int depth) {
    Move moves[200] = {0};
    uint8_t offset = getLegalMoves(game, moves);
    uint8_t canMove = 0;
    for(int i = 0; i < offset; ++i) {
        if(moves[i].piece) {
            canMove = 1;
        }
    }
    if(!canMove) {
        uint8_t result = checkGameEnding(game);
        if(result == LOSS) {
            return -100.0f;
        }
        else {
            return 0.0f;
        }
    }
    if(depth <= 0) {
        return 0.0f;
    }
    
    Game nextGame;
    Game nextNextGame;
    int64_t eval = 0;
    doMove(game, nextGame, getBestMoveBitstring(bitstring, game, eval));
    float score = 0.0f;
    int16_t numMoves = 0;
    offset = getLegalMoves(nextGame, moves);
    canMove = 0;
    for(int i = 0; i < offset; ++i) {
        if(moves[i].piece) {
            ++numMoves;
            canMove = 1;
            doMove(nextGame, nextNextGame, moves[i]);
            score += getBitstringScore(bitstring, nextNextGame, depth-1);
        }
    }
    if(!canMove) {
        uint8_t result = checkGameEnding(nextGame);
        if(result == LOSS) {
            return 100.0f;
        }
        else {
            return 0.0f;
        }
    }
    else {
        return score / float(numMoves);
    }
}

inline void setBitset(std::bitset<BITSTRING_LENGTH> &bitstring, uint16_t start, uint8_t value) {
    bitstring[start] = (value & 0b1);
    bitstring[start+1] = (value & 0b10);
    bitstring[start+2] = (value & 0b100);
    bitstring[start+3] = (value & 0b1000);
}

// entirely based on checkmates
// explores the full tree(with white playing bitstring moves) up to the cap
// the highest scoring bitstring is returned at the end.
// cap is measured in full moves
std::bitset<BITSTRING_LENGTH> bruteforceBitstring(const Game &game, int hardLengthCap) {
    uint8_t opList[] = {0, 4, 7, 10, 13};
    uint8_t idList[] = {4, 5, 11};
    uint8_t bsArrayRep[BITSTRING_LENGTH/4] = {0};
    uint8_t bsArrayIndex = 0;
    
    std::bitset<BITSTRING_LENGTH> currentBitstring;
    for(int i = 0; i < std::size(bsArrayRep); ++i) {
        if(i % 2 == 0) {
            setBitset(currentBitstring, i*4, idList[bsArrayRep[i]]);
        }
        else {
            setBitset(currentBitstring, i*4, opList[bsArrayRep[i]]);
        }
    }
    
    uint64_t total = 1;
    uint64_t numTokens = ((BITSTRING_LENGTH/4)-1)/2;
    total *= std::size(idList);
    for(int i = 0; i < numTokens; ++i) {
        total *= std::size(opList);
        total *= std::size(idList);
    }
    uint8_t finished = 0;
    float bestScore = std::numeric_limits<float>::lowest();
    std::bitset<BITSTRING_LENGTH> bestBitstring;
    uint64_t current = 0;
    while(!finished) {
        if(current % ((total/100)+1) == 0) {
            std::cout << std::to_string(current) << "/" << std::to_string(total) << " completed" << std::endl;
        }
        float currentScore = getBitstringScore(currentBitstring, game, hardLengthCap);
        if(currentScore > bestScore) {
            std::cout << "Score: " << std::to_string(currentScore) << std::endl;
            std::cout << "String: " << currentBitstring << "\n" << std::endl;
            bestScore = currentScore;
            bestBitstring = currentBitstring;
        }
        
        bsArrayIndex = 0;
        while(bsArrayIndex < std::size(bsArrayRep)) {
            if(bsArrayIndex % 2 == 0) {
                if(bsArrayRep[bsArrayIndex] == (std::size(idList)-1)) {
                    bsArrayRep[bsArrayIndex] = 0;
                    setBitset(currentBitstring, 4*bsArrayIndex, idList[bsArrayRep[bsArrayIndex]]);
                }
                else {
                    ++bsArrayRep[bsArrayIndex];
                    setBitset(currentBitstring, 4*bsArrayIndex, idList[bsArrayRep[bsArrayIndex]]);
                    break;
                }
            }
            else {
                if(bsArrayRep[bsArrayIndex] == (std::size(opList)-1)) {
                    bsArrayRep[bsArrayIndex] = 0;
                    setBitset(currentBitstring, 4*bsArrayIndex, opList[bsArrayRep[bsArrayIndex]]);
                }
                else {
                    ++bsArrayRep[bsArrayIndex];
                    setBitset(currentBitstring, 4*bsArrayIndex, opList[bsArrayRep[bsArrayIndex]]);
                    break;
                }
            }
            ++bsArrayIndex;
        }
        
        finished = (bsArrayIndex == std::size(bsArrayRep));
        ++current;
    }
    return bestBitstring;
}

std::bitset<BITSTRING_LENGTH> randBruteforceBitstring(const Game &game, int lengthCap, uint64_t numToTest, uint32_t seed) {
    uint8_t opList[] = {0, 4, 7, 10, 13};
    uint8_t idList[] = {4, 5, 11};
    std::mt19937 generator {seed};
    std::uniform_int_distribution<int> opDist{0, std::size(opList)-1};
    std::uniform_int_distribution<int> idDist{0, std::size(idList)-1};
    std::bitset<BITSTRING_LENGTH> currentBitstring;
    
    uint8_t finished = 0;
    float bestScore = std::numeric_limits<float>::lowest();
    std::bitset<BITSTRING_LENGTH> bestBitstring;
    uint64_t current = 0;
    while(current < numToTest) {
        for(int i = 0; i < BITSTRING_LENGTH/4; ++i)  {
            if(i % 2 == 0) {
                setBitset(currentBitstring, i*4, idList[idDist(generator)]);
            }
            else {
                setBitset(currentBitstring, i*4, opList[opDist(generator)]);
            }
        } 
        
        if(current % ((numToTest/100)+1) == 0) {
            std::cout << std::to_string(current) << "/" << std::to_string(numToTest) << " completed" << std::endl;
        }
        float currentScore = getBitstringScore(currentBitstring, game, lengthCap);
        if(currentScore > bestScore) {
            std::cout << "Score: " << std::to_string(currentScore) << std::endl;
            std::cout << "String: " << currentBitstring << "\n" << std::endl;
            bestScore = currentScore;
            bestBitstring = currentBitstring;
        }
        ++current;
    }
    return bestBitstring;
}

#endif
