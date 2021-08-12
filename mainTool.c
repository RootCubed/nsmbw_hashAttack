#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>   

#define u32 unsigned int

// https://github.com/simontime/iQiPack/blob/master/crypto.cpp#L5
u32 hash(char *string) {
	u32 hash = 0x1505; // Initial hash value.

	for (u32 i = 0; string[i] != 0; i++) {
		hash *= 33;
		hash ^= string[i];
	}

	return hash;
}

u32 hashContinue(u32 hash, char *string) {
	for (u32 i = 0; string[i] != 0; i++) {
		hash *= 33;
		hash ^= string[i];
	}

	return hash;
}

// @Ninji how tf
u32 reverseHashStep(u32 hash, char c) {
    hash ^= c;
    u32 div = hash / 33;
    u32 rem = hash % 33;
    u32 adjust = (rem == 0) ? 0 : (8 + (((rem - 1) & 3) << 3) - ((rem - 1) >> 2));
    return div + (adjust * 130150524) + ((adjust + 7) >> 3);
}

const char possible[] = "ABCDEFGHIJKLMNOPQRSTUVXYZabcdefghijklmnopqrstuvwxyz_";
char buf[32];
char *genStr(int len, long index) {
    for (int i = 0; i < len; i++) {
        buf[i] = possible[index % sizeof(possible)];
        index /= sizeof(possible);
    }
    buf[len] = 0;
    return buf;
}

int main() {
    // ------------------------------

    // here are all the inputs
    
    // for afterDelete
    u32 fBaseFuncHash = 0xC2AA80BB; // for mangled
    u32 dBaseFuncHash = 0xA86EEAF9;
    //u32 fBaseFuncHash = 0x71066FFC; // for demangled
    //u32 dBaseFuncHash = 0x7EFF42BE;

    char preBrute_fBase[] = "7fBase_cF18";
    char preBrute_dBase[] = "7dBase_cF18";
    //char preBrute_fBase[] = "fBase_c::";
    //char preBrute_dBase[] = "dBase_c::";


    int isDemangledSymbol = 0;
    int knowArgumentLength = 1;
    
    char argGuessBeginning[] = "";
    char argGuessEnd[] = "_e";

    int argLenGuessLower = 18;
    int argLenGuessUpper = 18;

    int funcNameGuess = 11; // todo: automate searching for this

    // ------------------------------

    srand((unsigned) time(NULL));
    u32 startHash_fBase = fBaseFuncHash;
    u32 startHash_dBase = dBaseFuncHash;

    if (isDemangledSymbol) {
        startHash_fBase = reverseHashStep(startHash_fBase, ')');
        startHash_dBase = reverseHashStep(startHash_dBase, ')');
        startHash_fBase = reverseHashStep(startHash_fBase, ' ');
        startHash_dBase = reverseHashStep(startHash_dBase, ' ');
    }

    for (int i = strlen(argGuessEnd) - 1; i >= 0; i--) {
        startHash_fBase = reverseHashStep(startHash_fBase, argGuessEnd[i]);
        startHash_dBase = reverseHashStep(startHash_dBase, argGuessEnd[i]);
    }

    printf("Starting hash cracking (common function name):\n");


    int lenGuessLower = argLenGuessLower - strlen(argGuessBeginning) - strlen(argGuessEnd);
    int lenGuessUpper = argLenGuessUpper - strlen(argGuessBeginning) - strlen(argGuessEnd);
    
    int found = 0;

    int argLength = 0;
    int funcNameLength = 0;
    
    char argName[32];
    char funcName[32];

    while (found < 4) {
        for (int len = lenGuessLower; len <= lenGuessUpper; len++) {
            u32 curr_fBase = startHash_fBase;
            u32 curr_dBase = startHash_dBase;
            
            for (int i = 0; i < len; i++) {
                char c = possible[rand() % (sizeof(possible) - 1)];
                argName[len - i - 1] = c;
                curr_fBase = reverseHashStep(curr_fBase, c);
                curr_dBase = reverseHashStep(curr_dBase, c);
            }
            argName[len] = '\0';

            for (int i = strlen(argGuessBeginning) - 1; i >= 0; i--) {
                curr_fBase = reverseHashStep(curr_fBase, argGuessBeginning[i]);
                curr_dBase = reverseHashStep(curr_dBase, argGuessBeginning[i]);
            }

            // number for type size
            if (!knowArgumentLength && !isDemangledSymbol) {
                curr_fBase = reverseHashStep(curr_fBase, (len % 10) + '0');
                curr_dBase = reverseHashStep(curr_dBase, (len % 10) + '0');
                
                if (len >= 10) {
                    curr_fBase = reverseHashStep(curr_fBase, (len / 10) + '0');
                    curr_dBase = reverseHashStep(curr_dBase, (len / 10) + '0');
                }
            }

            if (isDemangledSymbol) {
                curr_fBase = reverseHashStep(curr_fBase, ' ');
                curr_dBase = reverseHashStep(curr_dBase, ' ');

                curr_fBase = reverseHashStep(curr_fBase, '(');
                curr_dBase = reverseHashStep(curr_dBase, '(');

                for (int i = 0; i < funcNameGuess; i++) {
                    char c = possible[rand() % (sizeof(possible) - 1)];
                    funcName[funcNameGuess - i - 1] = c;
                    curr_fBase = reverseHashStep(curr_fBase, c);
                    curr_dBase = reverseHashStep(curr_dBase, c);
                }
                funcName[funcNameGuess] = '\0';
            }

            for (int i = strlen(preBrute_fBase) - 1; i >= 0; i--) {
                curr_fBase = reverseHashStep(curr_fBase, preBrute_fBase[i]);
            }
            for (int i = strlen(preBrute_dBase) - 1; i >= 0; i--) {
                curr_dBase = reverseHashStep(curr_dBase, preBrute_dBase[i]);
            }
            
            if (curr_fBase == curr_dBase) {
                char buf[60] = "";
                if (isDemangledSymbol) {
                    strcat(buf, preBrute_fBase);
                    strcat(buf, funcName);
                    strcat(buf, "( ");
                    strcat(buf, argGuessBeginning);
                    strcat(buf, argName);
                    strcat(buf, argGuessEnd);
                    strcat(buf, " )");
                } else {
                    strcat(buf, "(funcname)__");
                    strcat(buf, preBrute_fBase);
                    if (knowArgumentLength) {
                        char numBuf[5];
                        itoa(len, numBuf, 10);
                        strcat(buf, numBuf);
                    }
                    strcat(buf, argGuessBeginning);
                    strcat(buf, argName);
                    strcat(buf, argGuessEnd);
                }
                printf("Found hash collision: %s\n", buf);
                argLength = len;
                funcNameLength = funcNameGuess;
                found++;
            }
        }
    }
    printf("Function name length = %d, argument name length = %d", funcNameLength, argLength);
}