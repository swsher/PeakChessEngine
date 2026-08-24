#ifndef ENGINE_H
#define ENGINE_H

#include <chrono>
#include <thread>
#include <limits>

#include "constants.h"
#include "moves.h"
#include "bitstring.h"

uint64_t nodeCount(const Game &game, uint8_t depth) {
    if(depth == 0) {
        return 1;
    }
    uint64_t totalNodes = 0;
    Game nextGame;
    Move moves[200] = {0};
    getLegalMoves(game, moves);
    for(Move move : moves) {
        if(move.piece) {
            doMove(game, nextGame, move);
            totalNodes += nodeCount(nextGame, depth-1);
        }
    }
    return totalNodes;
}


void perft(std::string fenString, uint8_t depth) {
    auto start_t = std::chrono::steady_clock::now();
    Game game = gameFromFEN(fenString);
    Game nextGame;
    Move moves[200] = {0};
    getLegalMoves(game, moves);
    uint64_t totalNodes = 0;
    for(Move move : moves) {
        if(move.piece) {
            doMove(game, nextGame, move);
            uint64_t currentNodes = nodeCount(nextGame, depth-1);
            totalNodes += currentNodes;
            std::cout << getTileString(move.startTile) << getTileString(move.endTile) << " " << std::to_string(currentNodes) << std::endl;
        }
    }
    auto end_t = std::chrono::steady_clock::now();
    std::cout << "\n" << std::to_string(totalNodes) << std::endl;
    
    auto duration = end_t - start_t;
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    std::cout << "Time Taken: " << milliseconds << "ms" << std::endl;
    auto nps = (milliseconds) ? totalNodes / (milliseconds) : totalNodes;
    std::cout << "Speed: " << nps << " kN/s" << std::endl;
}

inline uint8_t moveEquals(const Move &move1, const Move &move2) {
    return move1.piece == move2.piece && move1.startTile == move2.startTile
    && move1.endTile == move2.endTile && move1.special == move2.special;
}

int64_t evaluateGame(const Game &game) {
    int64_t eval = 0;
    eval += std::popcount(game.whitePawn) * 1;
    eval += std::popcount(game.whiteKnight) * 3;
    eval += std::popcount(game.whiteBishop) * 3;
    eval += std::popcount(game.whiteRook) * 5;
    eval += std::popcount(game.whiteQueen) * 9;
    eval -= std::popcount(game.blackPawn) * 1;
    eval -= std::popcount(game.blackKnight) * 3;
    eval -= std::popcount(game.blackBishop) * 3;
    eval -= std::popcount(game.blackRook) * 5;
    eval -= std::popcount(game.blackQueen) * 9;
    uint8_t kingIndex = (game.isWhiteTurn) ? std::countr_zero(game.whiteKing) : std::countr_zero(game.blackKing);
    if(isSquareAttacked(game, kingIndex)) {
        --eval;
    }
    if(game.isWhiteTurn) {
        return eval;
    }
    else {
        return -1*eval;
    }
}

int64_t quiescence(std::stop_token sToken, auto startTime, int64_t timeMS, uint32_t &lastStopCheck, const Game &game, int64_t alpha, int64_t beta, uint8_t quiDepth, uint64_t &totalNodes) {
    ++totalNodes;
    ++lastStopCheck;
    if(lastStopCheck > 1000) {
        if(sToken.stop_requested()) {
            return 0;
        }
        
        auto endTime = std::chrono::steady_clock::now();
        int64_t timeElapsedMS = std::chrono::duration_cast<std::chrono::milliseconds>(endTime-startTime).count();
        if(timeElapsedMS > timeMS) {
            return 0;
        }
        
        lastStopCheck = 0;
    }
    
    if(quiDepth >= 3) {
        return evaluateGame(game);
    }
    
    Move moveTable[200] = {0};
    uint8_t offset = getLegalMoves(game, moveTable);
    Game nextGame;
    int64_t positionEval = evaluateGame(game);
    
    uint8_t moveFlag = 1;
    for(uint8_t index = 0; index < offset; index++) {
        if(moveTable[index].piece) {
            moveFlag = 0;
            doMove(game, nextGame, moveTable[index]);
            int64_t currentEval = evaluateGame(nextGame);
            if(-1*positionEval != currentEval) {
                currentEval = -1*quiescence(sToken, startTime, timeMS, lastStopCheck, nextGame, -1*beta, -1*alpha, quiDepth+1, totalNodes);
            }
            if(currentEval >= beta) {
                return beta;
            }
            if(currentEval > alpha) {
                alpha = currentEval;
            }
        }
    }
    if(moveFlag) {
        uint8_t result = checkGameEnding(game);
        if(result == LOSS) {
            return std::numeric_limits<int64_t>::min();
        }
        else {
            return 0;
        }
    }
    return alpha;
}

// alpha-beta negamax
int64_t negamax(std::stop_token sToken, auto startTime, int64_t timeMS, uint32_t &lastStopCheck, const Game &game, int64_t alpha, int64_t beta, int8_t depth, uint64_t &totalNodes) {
    ++totalNodes;
    ++lastStopCheck;
    if(lastStopCheck > 1000) {
        if(sToken.stop_requested()) {
            return 0;
        }
        
        auto endTime = std::chrono::steady_clock::now();
        int64_t timeElapsedMS = std::chrono::duration_cast<std::chrono::milliseconds>(endTime-startTime).count();
        if(timeElapsedMS > timeMS) {
            return 0;
        }
        
        lastStopCheck = 0;
    }

    if(depth <= 0) {
        return evaluateGame(game);
    }
    
    Move moveTable[200] = {0};
    uint8_t offset = getLegalMoves(game, moveTable);
    Game nextGame;
    
    uint8_t moveFlag = 1;
    for(uint8_t index = 0; index < offset; index++) {
        if(moveTable[index].piece) {
            moveFlag = 0;
            doMove(game, nextGame, moveTable[index]);
            int64_t currentEval = -1*negamax(sToken, startTime, timeMS, lastStopCheck, nextGame, -1*beta, -1*alpha, depth-1, totalNodes);
            if(currentEval >= beta) {
                return beta;
            }
            if(currentEval > alpha) {
                alpha = currentEval;
            }
        }
    }
    if(moveFlag) {
        uint8_t result = checkGameEnding(game);
        if(result == LOSS) {
            return std::numeric_limits<int64_t>::min();
        }
        else {
            return 0;
        }
    }
    return alpha;
}

Move getBestMove(std::stop_token sToken, auto startTime, int64_t timeMS, const Game &game, int8_t depth, int64_t &eval, uint64_t &totalNodes) {
    ++totalNodes;
    Move moveTable[200] = {0};
    uint8_t offset = getLegalMoves(game, moveTable);
    int64_t alpha = std::numeric_limits<int64_t>::min();
    int64_t beta = std::numeric_limits<int64_t>::max();
    Game nextGame;
    uint32_t lastStopCheck = 0;
    Move bestMove;
    
    for(uint8_t index = 0; index < offset; index++) {
        if(moveTable[index].piece) {
            doMove(game, nextGame, moveTable[index]);
            int64_t currentEval = -1*negamax(sToken, startTime, timeMS, lastStopCheck, nextGame, -1*beta, -1*alpha, depth-1, totalNodes);
            if(currentEval > alpha) {
                alpha = currentEval;
                bestMove = moveTable[index];
            }
        }
        if(sToken.stop_requested()) {
            return bestMove;
        }
        
        auto endTime = std::chrono::steady_clock::now();
        int64_t timeElapsedMS = std::chrono::duration_cast<std::chrono::milliseconds>(endTime-startTime).count();
        if(timeElapsedMS > timeMS) {
            return bestMove;
        }
    }
    eval = alpha;
    return bestMove;
}

void timedSearch(std::stop_token sToken, int64_t timeMS, const Game &game) {
    Move bestMove = {0};
    auto startTime = std::chrono::steady_clock::now();
    int64_t eval;
    int8_t currentDepth = 1;
    uint64_t totalNodes = 0;
    while(currentDepth < 100) {
        Move currentMove = getBestMove(sToken, startTime, timeMS, game, currentDepth, eval, totalNodes);
        auto endTime = std::chrono::steady_clock::now();
        int64_t timeElapsedMS = std::chrono::duration_cast<std::chrono::milliseconds>(endTime-startTime).count();
        if(sToken.stop_requested() || timeElapsedMS > timeMS) {
            std::string returnStr = "bestmove ";
            returnStr += getTileString(bestMove.startTile);
            returnStr += getTileString(bestMove.endTile);
            switch(bestMove.special) {
                case PROMOTE_QUEEN: returnStr += "q"; break;
                case PROMOTE_ROOK: returnStr += "r"; break;
                case PROMOTE_BISHOP: returnStr += "b"; break;
                case PROMOTE_KNIGHT: returnStr += "n"; break;
            }
            std::cout << returnStr << std::endl;
            return;
        }
        
        if(currentMove.piece) {
            bestMove = currentMove;
            std::string infoString = "info depth ";
            infoString += std::to_string(currentDepth);
            infoString += " time ";
            infoString += std::to_string(timeElapsedMS);
            infoString += " nodes ";
            infoString += std::to_string(totalNodes);
            infoString += " pv ";
            infoString += getTileString(bestMove.startTile);
            infoString += getTileString(bestMove.endTile);
            switch(bestMove.special) {
                case PROMOTE_QUEEN: infoString += "q"; break;
                case PROMOTE_ROOK: infoString += "r"; break;
                case PROMOTE_BISHOP: infoString += "b"; break;
                case PROMOTE_KNIGHT: infoString += "n"; break;
            }
            infoString += " score cp ";
            infoString += std::to_string(eval*200);
            infoString += " nps ";
            if(timeElapsedMS) {
                infoString += std::to_string((totalNodes*1000)/timeElapsedMS);
            }
            else {
                infoString += std::to_string(totalNodes*1000);
            }
            std::cout << infoString << std::endl;
            if(eval == std::numeric_limits<int64_t>::max()) {
                std::string returnStr = "bestmove ";
                returnStr += getTileString(bestMove.startTile);
                returnStr += getTileString(bestMove.endTile);
                switch(bestMove.special) {
                    case PROMOTE_QUEEN: returnStr += "q"; break;
                    case PROMOTE_ROOK: returnStr += "r"; break;
                    case PROMOTE_BISHOP: returnStr += "b"; break;
                    case PROMOTE_KNIGHT: returnStr += "n"; break;
                }
                std::cout << returnStr << std::endl;
                return;
            }
        }
        else {
            currentDepth--;
        }
        
        currentDepth++;
    }
}

#endif
