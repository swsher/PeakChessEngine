#include <iostream>
#include <cstdint>
#include <string>
#include <print>
#include <bit>
#include <chrono>
#include <bitset>
#include <vector>

#include "constants.h"
#include "precompute.h"
#include "display.h"
#include "moves.h"
#include "engine.h"
#include "uci.h"
#include "bitstring.h"

// len 32
// 10111000110000110111000110001110
int main(int argc, char *argv[]) {
    initMasterLUT();
    // std::vector<std::bitset<BITSTRING_LENGTH>> currentGeneration = initGeneration(128);
    // for(int i = 0; i < 500; i++) {
    //     currentGeneration = runGeneration(currentGeneration, 0.1f, i);
    //     std::cout << "Generation: " << std::to_string(i) << std::endl;
    // }
    // for(std::bitset bitstring : currentGeneration) {
    //     std::cout << bitstring << std::endl;
    // }
    //run();
    Game game = gameFromFEN("k7/8/8/6r1/7r/1K6/8/8 b - - 0 1");
    std::cout << bruteForceOptimalBitstring(game, 3) << std::endl;
	return 0;
}
