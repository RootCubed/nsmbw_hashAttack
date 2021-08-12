#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>   

#define u32 unsigned int

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

/*u32 reverseHashStep(u32 hash, char c) {
    u32 xored, carry, mask, result;
    xored = hash ^ c;

    carry = 0;
    result = xored & 0b11111;

    mask = 0b11111 << 5;
    result |= ((xored & mask) - ((result << 5) & mask) - carry) & (0b111111 << 5);
    carry = result & (1 << 10);
    result ^= carry;

    mask = 0b11111 << 10;
    result |= ((xored & mask) - ((result << 5) & mask) - carry) & (0b111111 << 10);
    carry = result & (1 << 15);
    result ^= carry;

    mask = 0b11111 << 15;
    result |= ((xored & mask) - ((result << 5) & mask) - carry) & (0b111111 << 15);
    carry = result & (1 << 20);
    result ^= carry;

    mask = 0b11111 << 20;
    result |= ((xored & mask) - ((result << 5) & mask) - carry) & (0b111111 << 20);
    carry = result & (1 << 25);
    result ^= carry;

    mask = 0b11111 << 25;
    result |= ((xored & mask) - ((result << 5) & mask) - carry) & (0b111111 << 25);
    carry = result & (1 << 30);
    result ^= carry;
    
    mask = 0b11 << 30;
    result |= (xored & mask) - ((result << 5) & mask) - carry;

    return result;
}*/

u32 reverseHashStep(u32 hash, char c) { // @Ninji how
    hash ^= c;
    u32 div = hash / 33;
    u32 rem = hash % 33;
    u32 adjust = (rem == 0) ? 0 : (8 + (((rem - 1) & 3) << 3) - ((rem - 1) >> 2));
    return div + (adjust * 130150524) + ((adjust + 7) >> 3);
}

const char possible[] = "ABCDEFGHIJKLMNOPQRSTUVXYZabcdefghijklmnopqrstuvwxyz_";
char buf[20];
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

    // for afterCreate
    u32 fBaseFuncHash = 0xF4BAA446; // for mangled
    u32 dBaseFuncHash = 0xF78D4984;
    //u32 fBaseFuncHash = 0x2C15C681; // for demangled
    //u32 dBaseFuncHash = 0x023630C3;
    
    // for afterDelete
    //u32 fBaseFuncHash = 0xC2AA80BB; // for mangled
    //u32 dBaseFuncHash = 0xA86EEAF9;
    //u32 fBaseFuncHash = 0x71066FFC; // for demangled
    //u32 dBaseFuncHash = 0x7EFF42BE;
    
    // for afterExecute
    //u32 fBaseFuncHash = 0xAF093B9D; // for mangled
    //u32 dBaseFuncHash = 0xD3DF545F;
    //u32 fBaseFuncHash = 0x834E007A; // for demangled
    //u32 dBaseFuncHash = 0xD15D72F8;
    
    // for afterDraw
    //u32 fBaseFuncHash = 0xC001F522; // for mangled
    //u32 dBaseFuncHash = 0xB11C1B60;
    //u32 fBaseFuncHash = 0xD3BF4625; // for demangled
    //u32 dBaseFuncHash = 0x218945E7;

    char preBrute_fBase[] = "fBase_cFQ27fBase_c12";
    char preBrute_dBase[] = "dBase_cFQ27fBase_c12";
    //char preBrute_fBase[] = "fBase_c::";
    //char preBrute_dBase[] = "dBase_c::";


    int isDemangledSymbol = 0;
    int knowArgumentLength = 1;
    
    char argGuessBeginning[] = "m";
    char argGuessEnd[] = "";

    int argLenGuessLower = 12;
    int argLenGuessUpper = 12;

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
                    if (!knowArgumentLength) {
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