#ifndef UCI_H
#define UCI_H

#include <string>
#include <list>
#include <sstream>
#include <thread>
#include <functional>
#include <iostream>
#include <bitset>

#include "moves.h"
#include "engine.h"
#include "display.h"

Game game;
std::jthread searchThread;
uint8_t isActive;
uint8_t BITSTRING_MODE = 1;

std::list<std::string> inputToTokens(std::string input) {
    std::list<std::string> tokens;
    std::istringstream stringStream(input);
    std::string token;
    
    while(stringStream >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

std::string handleUci() {
    initMasterLUT();
    std::string returnStr = "id name AmbiorixV1\n";
    returnStr += "id author Ultraeon\n";
    returnStr += "uciok";
    return returnStr;
}

// unimplemented
void handleDebug(std::list<std::string> tokens) {
    return;
}

std::string handleIsReady() {
    std::string returnStr = "readyok";
    return returnStr;
}

// unimplemented
void handleSetOption(std::list<std::string> tokens) {
    return;
} 

// unimplemented
void handleRegister(std::list<std::string> tokens) {
    return;
}

void handleUciNewGame() {
    game = initGame();
    return;
}

void handlePosition(std::list<std::string> tokens) {
    std::string initType = tokens.front();
    tokens.pop_front();
    
    if(initType == "startpos") {
        game = initGame();
    }
    else if(initType == "fen") {
        std::string fenString;
        
        std::string token = (tokens.empty()) ? "" : tokens.front();
        while(!(token.empty()) && (token != "moves")) {
            tokens.pop_front();
            
            fenString += " " + token;
            
            token = (tokens.empty()) ? "" : tokens.front();;
        }
        game = gameFromFEN(fenString);
    }
    
    if(!tokens.empty()) {
        tokens.pop_front();
    }
    
    for(std::string token : tokens) {
        uint8_t startIndex = token[0]-97;
        uint8_t endIndex = token[2]-97;
        uint8_t special = 0;
        startIndex += 8*(8-(token[1]-48));
        endIndex += 8*(8-(token[3]-48));
        if(token.length() == 5) {
            switch(token[4]) {
                case 'q': special = PROMOTE_QUEEN; break; 
                case 'r': special = PROMOTE_ROOK; break;
                case 'b': special = PROMOTE_BISHOP; break;
                case 'n': special = PROMOTE_KNIGHT; break;
            }
        }
        
        Move moves[200] = {0}; 
        getLegalMoves(game, moves);
        Game nextGame;
        if(special) {
            for(Move move : moves) {
                if(move.piece) {
                    if(move.startTile == startIndex && move.endTile == endIndex && move.special == special) {
                        doMove(game, nextGame, move);
                        break;
                    }
                }
            }
        }
        else {
            for(Move move : moves) {
                if(move.piece) {
                    if(move.startTile == startIndex && move.endTile == endIndex) {
                        doMove(game, nextGame, move);
                        break;
                    }
                }
            }
        }
        
        game = nextGame;
    }
}

void handleGo(std::list<std::string> tokens) {
    std::string token = tokens.front();
    tokens.pop_front();
    if(BITSTRING_MODE) {
        int64_t eval = 0;
        Move bestMove = getBestMoveBitstring(BEST_BITSTRING, game, eval);
        std::string infoStr = "info depth 1 time 1 pv ";
        std::string returnStr = "bestmove ";
        returnStr += getTileString(bestMove.startTile);
        returnStr += getTileString(bestMove.endTile);
        infoStr += getTileString(bestMove.startTile);
        infoStr += getTileString(bestMove.endTile);
        switch(bestMove.special) {
            case PROMOTE_QUEEN: returnStr += "q"; infoStr += "q"; break;
            case PROMOTE_ROOK: returnStr += "r"; infoStr += "r"; break;
            case PROMOTE_BISHOP: returnStr += "b"; infoStr += "b"; break;
            case PROMOTE_KNIGHT: returnStr += "n"; infoStr += "n"; break;
        }
        infoStr += " score cp ";
        infoStr += std::to_string(eval/0xFFFFFFFFFFFFFF);
        std::cout << infoStr << std::endl;
        std::cout << returnStr << std::endl;
        return;
    }
    
    if(token == "infinite") {
        searchThread = std::jthread(timedSearch, 0x7FFFFFFFFFFFFFFFLL, std::cref(game));
    }
    else if(token == "movetime") {
        std::string timeMS = tokens.front();
        searchThread = std::jthread(timedSearch, std::stoi(timeMS), std::cref(game));
    }
    else if(token == "wtime") {
        uint64_t whiteTimeMS = std::stoi(tokens.front());
        tokens.pop_front();
        tokens.pop_front();
        uint64_t blackTimeMS = std::stoi(tokens.front());
        uint64_t timeRemainingMS = (game.isWhiteTurn) ? whiteTimeMS : blackTimeMS;
        searchThread = std::jthread(timedSearch, timeRemainingMS/20, std::cref(game));
    }
}

// unimplemented
void handlePonderHit() {
    return;
}

void handleStop() {
    if(searchThread.joinable()) {
        searchThread.request_stop();
    }
}

void handleQuit() {
    isActive = 0;
}

std::string handleLine(std::string input) {
    std::list<std::string> tokens = inputToTokens(input);
    if(tokens.empty()) {
        return "";
    }
    std::string command = tokens.front();
    tokens.pop_front();
    
    std::string returnStr = "";
    if(command == "uci") {
        returnStr = handleUci();
    }
    else if(command == "debug") {
        handleDebug(tokens);
    }
    else if(command == "isready") {
        returnStr = handleIsReady();
    }
    else if(command == "setoption") {
        handleSetOption(tokens);
    }
    else if(command == "register") {
        handleRegister(tokens);
    }
    else if(command == "ucinewgame") {
        handleUciNewGame();
    }
    else if(command == "position") {
        handlePosition(tokens);    
    }
    else if(command == "go") {
        handleGo(tokens);   
    }
    else if(command == "ponderhit") {
        handlePonderHit();
    }
    else if(command == "stop") {
        handleStop();
    }
    else if(command == "quit") {
        handleQuit();    
    }
    return returnStr;
}

void run() {
    std::string input;
    isActive = 1;
    while(isActive) {
        std::getline(std::cin, input);
        std::string response = handleLine(input);
        if(!response.empty()) {
            std::cout << response << std::endl;
        }
    }
}

#endif
