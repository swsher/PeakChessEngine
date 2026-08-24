#ifndef BITSTRING_H
#define BITSTRING_H

#include <random>
#include <array>
#include <vector>
#include <algorithm>
#include <limits>
#include <bitset>
#include <string>

#include "moves.h"
#include "engine.h"

/*

bitboard identifier - 4 bits (0-15)
0 = white pawn bitboard
1 = white knight bitboard
2 = white bishop bitboard
3 = white rook bitboard
4 = white queen bitboard
5 = white king bitboard
6 = black pawn bitboard
7 = black knight bitboard
8 = black bishop bitboard
9 = black rook bitboard
10 = black queen bitboard
11 = black king bitboard
12 = white piece bitboard
13 = black piece bitboard
14 = all piece bitboard
15 = en passant bitboard

binary operator - 4 bits(0-15)
0-3 = +
4-6 = *
7-9 = |
10-12 = ^
13-15 = &

bitstrings are read in the form id-op-id-op...id
bitstrings must be of length = 8n+4 where n is an integer >= 0

*/

constexpr inline int BITSTRING_LENGTH = 84;
constexpr inline std::bitset<BITSTRING_LENGTH> BEST_BITSTRING("0000");

inline uint8_t readFromBitset(std::bitset<BITSTRING_LENGTH> bitstring, int end) {
    uint8_t value = 0;
    value += bitstring[end] << 3;
    value += bitstring[end-1] << 2;
    value += bitstring[end-2] << 1;
    value += bitstring[end-3];
    return value;
}

inline uint64_t applyBinaryAction(uint64_t lastBitboard, uint64_t nextBitboard, uint8_t action) {
    if(action <= 3) {
        return lastBitboard + nextBitboard;
    }
    else if(action <= 6) {
        return lastBitboard * nextBitboard;
    }
    else if(action <= 9) {
        return lastBitboard | nextBitboard;
    }
    else if(action <= 12) {
        return lastBitboard ^ nextBitboard;
    }
    return lastBitboard & nextBitboard;
}

inline uint64_t getBitboardFromIdentifier(const Game &game, uint8_t identifier) {
    switch(identifier) {
        case 0: return game.whitePawn;
        case 1: return game.whiteKnight;
        case 2: return game.whiteBishop;
        case 3: return game.whiteRook;
        case 4: return game.whiteQueen;
        case 5: return game.whiteKing;
        case 6: return game.blackPawn;
        case 7: return game.blackKnight;
        case 8: return game.blackBishop;
        case 9: return game.blackRook;
        case 10: return game.blackQueen;
        case 11: return game.blackKing;
        case 12: return game.whitePieces;
        case 13: return game.blackPieces;
        case 14: return game.allPieces;
        case 15: return game.enPassantBoard;
    }
    std::cout << "something went wrong in getbbfromidentifier" << std::endl;
    return 0;
}

int64_t bitstringEvaluate(std::bitset<BITSTRING_LENGTH> bitstring, const Game &game) {
    uint64_t lastBitboard = getBitboardFromIdentifier(game, readFromBitset(bitstring, 4));
    uint64_t currentBitboard;
    uint8_t action;
    uint8_t isIdentifier = 1;
    for(int i = 7; i < bitstring.size(); i += 4) {
        uint8_t value = readFromBitset(bitstring, i);
        if(isIdentifier) {
            currentBitboard = getBitboardFromIdentifier(game, value);
            lastBitboard = applyBinaryAction(lastBitboard, currentBitboard, action);
        }
        else {
            action = value;
        }
        isIdentifier = !isIdentifier;
    }
    // swap to int64_t
    int64_t eval = int64_t(lastBitboard) + 1 + std::numeric_limits<int64_t>::max();
    if(game.isWhiteTurn) {
        return eval;
    }
    else {
        return -1*eval;
    }
}

std::string getIdentifierString(uint8_t value) {
    switch(value) {
        case 0: return "whitepawn ";
        case 1: return "whiteknight ";
        case 2: return "whitebishop ";
        case 3: return "whiterook ";
        case 4: return "whitequeen ";
        case 5: return "whiteking ";
        case 6: return "blackpawn ";
        case 7: return "blackknight ";
        case 8: return "blackbishop ";
        case 9: return "blackrook ";
        case 10: return "blackqueen ";
        case 11: return "blackking ";
        case 12: return "whitepieces ";
        case 13: return "blackpieces ";
        case 14: return "allpieces ";
        case 15: return "enpassant ";
    }
    return "error";
}

std::string getBinaryActionString(uint8_t value) {
    if(value <= 3) {
        return "+ ";
    }
    else if(value <= 6) {
        return "* ";
    }
    else if(value <= 9) {
        return "| ";
    }
    else if(value <= 12) {
        return "^ ";
    }
    return "& ";
}

std::string getReadableString(std::bitset<BITSTRING_LENGTH> bitstring) {
    std::string returnStr = "";
    uint8_t isIdentifier = 1;
    for(int i = 3; i < bitstring.size(); i += 4) {
        uint8_t value = readFromBitset(bitstring, i);
        if(isIdentifier) {
            returnStr += getIdentifierString(value);
        }
        else {
            returnStr += getBinaryActionString(value);
        }
        isIdentifier = !isIdentifier;
    }
    return returnStr;
}

// constant seed for replicability
std::bitset<BITSTRING_LENGTH> generateRandomBitstring(uint32_t seed) {
    std::mt19937 generator {seed};
    std::uniform_int_distribution<int> bitDistribution{0, 1};
    std::bitset<BITSTRING_LENGTH> randomBitset;
    for(int i = 0; i < BITSTRING_LENGTH; ++i)  {
        randomBitset[i] = bitDistribution(generator);
    } 
    return randomBitset;
}

Move getBestMoveBitstring(std::bitset<BITSTRING_LENGTH> bitstring, const Game &game, int64_t &bestEval) {
    Move moves[200] = {0};
    getLegalMoves(game, moves);
    Move bestMove;
    bestEval = std::numeric_limits<int64_t>::min();
    int64_t eval;
    Game nextGame;
    
    for(Move move : moves) {
        if(move.piece) {
            doMove(game, nextGame, move);
            eval = bitstringEvaluate(bitstring, nextGame);
            if(eval >= bestEval) {
                bestMove = move;
                bestEval = eval;
            }
        }
    }
    //std::cout << "Evaluation: " << std::to_string(bestEval) << std::endl;
    return bestMove;
}

#endif
