
#define u32 unsigned int

#define  _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

u32 hash(char *string) {
	u32 hash = 0x1505; // Initial hash value.

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

const char possible[] = "ABCDEFGHIJKLMNOPQRSTUVXYZabcdefghijklmnopqrstuvwxyz";
char buf[32];
char *genStr(int len, long index) {
    for (int i = 0; i < len; i++) {
        buf[i] = possible[index % sizeof(possible)];
        index /= sizeof(possible);
    }
    buf[len] = 0;
    return buf;
}

long powLong(int base, int exp) {
    long res = 1;
    for (int i = 0; i < exp; i++) {
        res *= base;
    }
    return res;
}

int main() {
    // vuln demo
    char buf[0x100];
    char secret[] = "Ipwbfwt";
    char one[] = "One";
    char two[] = "Two";
    while (1) {
        fgets(buf, 0x100, stdin);
        buf[strcspn(buf, "\r\n")] = '\0';
        printf("%08x\n", hash(buf));
        u32 hashBuf = hash(buf);
        for (int i = strlen(secret) - 1; i >= 0; i--) {
            hashBuf = reverseHashStep(hashBuf, secret[i]);
        }
        printf("secret: %08x\n", hashBuf);
        u32 tmp = hashBuf;
        for (int i = strlen(one) - 1; i >= 0; i--) {
            tmp = reverseHashStep(tmp, one[i]);
        }
        printf("one: %08x\n", tmp);
        tmp = hashBuf;
        for (int i = strlen(two) - 1; i >= 0; i--) {
            tmp = reverseHashStep(tmp, two[i]);
        }
        printf("two: %08x\n", tmp);
    }
    
    // test hash collisions
    /*u32 goal = 0x0d462e07;
    char buf1[] = "";
    char buf2[100];
    for (int len = 1; len < 6; len++) {
        long end = powLong(sizeof(possible), len);
        printf("%d\n", end);
        for (long index = 0; index < end; index++) {
            char *b = genStr(len, index);
            strcpy(buf2, buf1);
            strcat(buf2, b);
            if (hash(buf2) == goal) {
                printf(buf2);
                exit(0);
            }
        }
        //printf("len %d done.\n", len);
    }*/
}