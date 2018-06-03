#pragma once;
#include <vector>

namespace standards {
const std::vector<std::vector<std::vector<unsigned char>>> STANDARDS = {
    {{0x00}}, // Zeros  (NIST SP-800-88 Rev. 1)

    {{}}, // Random Data  (British HMG Infosec Standard 5, Baseline Standard)

    {{0x00}, {0xFF}, {}}, // Zeros, Ones, Random Data (British HMG Infosec
                          // Standard 5, Enhanced Standard)

    {{0xFF}, {0x00}, {}, {}, {}, {}, {}}, // Bruce Schneier's Algirithm (7-Pass)

    {{}, // Peter Gutmann's Algorithm (35-Pass)
     {},
     {},
     {},
     {0x55, 0x55, 0x55},
     {0xAA, 0xAA, 0xAA},
     {0x92, 0x49, 0x24},
     {0x49, 0x24, 0x94},
     {0x24, 0x92, 0x49},
     {0x00, 0x00, 0x00},
     {0x11, 0x11, 0x11},
     {0x22, 0x22, 0x22},
     {0x33, 0x33, 0x33},
     {0x44, 0x44, 0x44},
     {0x55, 0x55, 0x55},
     {0x66, 0x66, 0x66},
     {0x77, 0x77, 0x77},
     {0x88, 0x88, 0x88},
     {0x99, 0x99, 0x99},
     {0xAA, 0xAA, 0xAA},
     {0xBB, 0xBB, 0xBB},
     {0xCC, 0xCC, 0xCC},
     {0xDD, 0xDD, 0xDD},
     {0xEE, 0xEE, 0xEE},
     {0xFF, 0xFF, 0xFF},
     {0x92, 0x49, 0x24},
     {0x49, 0x24, 0x94},
     {0x24, 0x92, 0x49},
     {0x6D, 0xB6, 0xDB},
     {0xB6, 0xDB, 0x6D},
     {0xDB, 0x6D, 0xB6},
     {},
     {},
     {},
     {}}};

const std::vector<std::string> NAMES = {
    "NIST SP-800-88 Rev. 1 (All Zeros)", "British HMG Infosec Standard 5, Baseline Standard (Random Bits)",
    "British HMG Infosec Standard 5, Enhanced Standard (Zeros, Ones, Random Bits)",
    "Bruce Schneier's Algorithm (Ones, Zeros, 5 Random Passes)", "Peter Gutmann's Algorithm (35-Pass)"};
const std::vector<std::string> NAMES_SHORT  = {"Zeros", "Random", "Enhanced", "Schneier", "Gutmann"};
const std::vector<std::string> DESCRIPTIONS = {
    "This standard by the National Institution of Standards and Techonology overwrites data once with 0s. This should be "
    "enough to wipe data from any modern disk, however floppy disks' data can still be recovered afterwards using forensic "
    "analysis. This is the fastest method.",
    "This standard overwrites with random data. This can be useful for hiding any presence of overwriting. This is slightly "
    "slower than NIST SP-800-88 Rev. 1 as random bits must be generated.",
    "This standard is slightly more secure and overwrites data 3 times: once with 0s, once with 1s and finally again with "
    "random bits.",
    "This method overwrites data with 0s, 1s and 5 rounds of random bits. In total it overwrites data 7 times.",
    "This method is specifically designed to combat different magnetic encoding schemes. It overwrites data 35 times with "
    "different values each time. It is the slowest but most secure out of all of these options."};
enum standard { ZEROS, RANDOM, HMG, SCHNEIER, GUTMANN };
} // namespace standards