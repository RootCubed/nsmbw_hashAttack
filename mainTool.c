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

u32 reverseHashStep(u32 hash, char c) { // @Ninji how
    hash ^= c;
    u32 div = hash / 33;
    u32 rem = hash % 33;
    u32 adjust = (rem == 0) ? 0 : (8 + (((rem - 1) & 3) << 3) - ((rem - 1) >> 2));
    return div + (adjust * 130150524) + ((adjust + 7) >> 3);
}

const char possible[] = "ABCDEFGHIJKLMNOPQRSTUVXYZabcdefghijklmnopqrstuvwxyz_0123456789";
char buf[32];
char *genStr(int len, long index) {
    for (int i = 0; i < len; i++) {
        buf[i] = possible[index % sizeof(possible)];
        index /= sizeof(possible);
    }
    buf[len] = 0;
    return buf;
}

int main(int argc, char **argv) {
    u32 fBaseFuncHash, dBaseFuncHash;
    char preBrute_fBase[128];
    char preBrute_dBase[128];

    int isDemangledSymbol, useArgumentLength;
    
    char argGuessBeginning[64];
    char argGuessEnd[64];

    int argLenGuessLower, argLenGuessUpper;

    int funcNameGuessLen; // todo: automate searching for this

    if (argc > 1) {
        FILE *f = fopen(argv[1], "r");
        
        fscanf(f, "0x%08x\n", &fBaseFuncHash);
        fscanf(f, "0x%08x\n", &dBaseFuncHash);

        char inBuf[128];
        
        fgets(inBuf, 128, f);
        inBuf[strcspn(inBuf, "\r\n")] = '\0';
        sscanf(inBuf, "\"%s", preBrute_fBase);
        preBrute_fBase[strcspn(preBrute_fBase, "\"")] = '\0';
        
        fgets(inBuf, 128, f);
        inBuf[strcspn(inBuf, "\r\n")] = '\0';
        sscanf(inBuf, "\"%s", preBrute_dBase);
        preBrute_dBase[strcspn(preBrute_dBase, "\"")] = '\0';

        fscanf(f, "%d %d\n", &isDemangledSymbol, &useArgumentLength);
        
        fgets(inBuf, 64, f);
        inBuf[strcspn(inBuf, "\r\n")] = '\0';
        sscanf(inBuf, "\"%s", argGuessBeginning);
        argGuessBeginning[strcspn(argGuessBeginning, "\"")] = '\0';
        
        fgets(inBuf, 64, f);
        inBuf[strcspn(inBuf, "\r\n")] = '\0';
        sscanf(inBuf, "\"%s", argGuessEnd);
        argGuessEnd[strcspn(argGuessEnd, "\"")] = '\0';

        fscanf(f, "%d-%d\n", &argLenGuessLower, &argLenGuessUpper);
        fscanf(f, "%d\n", &funcNameGuessLen);

        fclose(f);
    } else {
        printf("Please supply a collider file to start searching.");
    }

    if (argc <= 2 || strcmp(argv[2], "-s") != 0) {
        printf("Beginning collision searching with parameters:\n");
        printf("Hashes              0x%08x | 0x%08x\n", fBaseFuncHash, dBaseFuncHash);
        printf("Static part         %10s | %10s\n", preBrute_fBase, preBrute_dBase);
        printf("Demangled symbol    %s\n", (isDemangledSymbol) ? "[x]" : "[ ]");
        printf("Use argument length %s\n", (useArgumentLength) ? "[x]" : "[ ]");
        printf("Arg start/end guess \"%s\", \"%s\"\n", argGuessBeginning, argGuessEnd);
        printf("Arg len guess range %d-%d\n", argLenGuessLower, argLenGuessUpper);
        printf("Function name len   %d\n", funcNameGuessLen);
    }

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

    //printf("Starting hash cracking (common function name):\n");


    int lenGuessLower = argLenGuessLower - strlen(argGuessBeginning) - strlen(argGuessEnd);
    int lenGuessUpper = argLenGuessUpper - strlen(argGuessBeginning) - strlen(argGuessEnd);
    
    int found = 0;

    int argLength = 0;
    int funcNameLength = 0;
    
    char argName[64];
    char funcName[64];

    while (found < 100000) {
        u32 save_1_fBase = startHash_fBase;
        u32 save_1_dBase = startHash_dBase;

        for (int i = 0; i < lenGuessUpper; i++) {
            char c = possible[rand() % (sizeof(possible) - 1)];
            argName[lenGuessUpper - i - 1] = c;
            if (i < lenGuessLower - 1) {
                save_1_fBase = reverseHashStep(save_1_fBase, c);
                save_1_dBase = reverseHashStep(save_1_dBase, c);
            }
        }
        argName[lenGuessUpper] = '\0';

        for (int len = lenGuessLower; len <= lenGuessUpper; len++) {
            
            u32 curr_fBase = reverseHashStep(save_1_fBase, argName[lenGuessUpper - len]);
            u32 curr_dBase = reverseHashStep(save_1_dBase, argName[lenGuessUpper - len]);

            save_1_fBase = curr_fBase;
            save_1_dBase = curr_dBase;

            if ((lenGuessLower - len) % 2 == 1) continue;

            for (int i = strlen(argGuessBeginning) - 1; i >= 0; i--) {
                curr_fBase = reverseHashStep(curr_fBase, argGuessBeginning[i]);
                curr_dBase = reverseHashStep(curr_dBase, argGuessBeginning[i]);
            }

            // number for type size
            if (useArgumentLength && !isDemangledSymbol) {
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

                for (int i = 0; i < funcNameGuessLen; i++) {
                    char c = possible[rand() % (sizeof(possible) - 1)];
                    funcName[funcNameGuessLen - i - 1] = c;
                    curr_fBase = reverseHashStep(curr_fBase, c);
                    curr_dBase = reverseHashStep(curr_dBase, c);
                }
                funcName[funcNameGuessLen] = '\0';
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
                    strcat(buf, argName + (lenGuessUpper - len));
                    strcat(buf, argGuessEnd);
                    strcat(buf, " )");
                } else {
                    strcat(buf, "(commonStart)");
                    strcat(buf, preBrute_fBase);
                    if (useArgumentLength) {
                        char numBuf[5];
                        itoa(len, numBuf, 10);
                        strcat(buf, numBuf);
                    }
                    strcat(buf, argGuessBeginning);
                    strcat(buf, argName + (lenGuessUpper - len));
                    strcat(buf, argGuessEnd);
                }
                printf("%s\n", buf);
                fflush(stdout);
                argLength = len;
                funcNameLength = funcNameGuessLen;
                found++;
            }
        }
    }
    //printf("Function name length = %d, argument name length = %d", funcNameLength, argLength);
}