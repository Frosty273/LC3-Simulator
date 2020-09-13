#include <stdio.h>

void output(int registerArray[], int PC, int IR, int CC);
int main(int argc, char*argv[]) {
        FILE *fp;
        char buf[2];
        fp = fopen(argv[1], "rb");
        unsigned int size = 0, index = 0;
        //Determine size of the instruction array
        while (fread(buf, 1, 2, fp) == 2) {
                size++;
        }
        fclose(fp);
        int instructionArray[size];
        //Set each index in the array to the full 2 bytes combined;
        fp = fopen(argv[1], "rb");
        while (fread(buf, 1, 2, fp) == 2) {
                unsigned char byte1 = buf[0];
                unsigned char byte2 = buf[1];
                instructionArray[index] =  byte2+(256*byte1);
                index++;
        }
        fclose(fp);
        //Initialise
        unsigned int initial = 0;
        int registerArray[8];
        unsigned int R0  = initial, R1  = initial, R2  = initial, R3  = initial, R4  = initial, R5 = initial, R6 = initial, R7 = initial, IR = initial;
        registerArray[0] = R0, registerArray[1] = R1, registerArray[2] = R2, registerArray[3] = R3;
        registerArray[4] = R4, registerArray[5] = R5, registerArray[6] = R6, registerArray[7] = R7;
        unsigned int CC = 0x5A, BRn = 0, BRz = 0, BRp = 0;
        int PC = instructionArray[0], startingPC = instructionArray[0];
        //Execution
        int binary = 0, opcode = 0, dr = 0, offset = 0, newIndex = 0, temp = 0, modeType = 0;
        signed short signedData = 0, ldiOffset = 0, data = 0, data2 = 0, signedOffset, changingI, sr1 = 0, sr2 = 0, shortCC = 0;
        unsigned short unsignedData = 0;
         for(int i = 1; i < size; i++) {
                binary = instructionArray[i];
                if (binary == 0xF025) {
                        break;
                }
                opcode = binary & 0xF000;
                PC = PC + 1;
                IR = instructionArray[i];
                switch(opcode) {
                        case 0x2000://LD
                                dr = (instructionArray[i] & 0xE00) >> 9;
                                offset = instructionArray[i] & 0x1FF;
                                unsignedData = instructionArray[i+1+offset];
                                registerArray[dr] = unsignedData;
                                signedData = unsignedData;
                                if (signedData > 0) {
                                        CC = 0x50;
                                } else if (signedData == 0) {
                                        CC = 0x5A;
                                } else {
                                        CC = 0x4E;
                                }
                                break;
                        case 0xE000://LEA
                                dr = (instructionArray[i] & 0xE00) >> 9;
                                offset = instructionArray[i] & 0x1FF;
                                signedData = PC + offset;
                                registerArray[dr] = PC + offset;
                                if (signedData > 0) {
                                        CC = 0x50;
                                } else if (signedData  == 0) {
                                        CC = 0x5A;
                                } else {
                                        CC = 0x4E;
                                }
                                break;
                        case 0xA000://LDI
                                dr = (instructionArray[i] & 0xE00) >> 9;
                                ldiOffset = instructionArray[i] & 0x1FF;
                                ldiOffset = ldiOffset << 7;
                                ldiOffset = ldiOffset >> 7;
                                newIndex = instructionArray[i+1+ldiOffset];
                                unsignedData = instructionArray[newIndex - startingPC + 1];
                                registerArray[dr] = unsignedData;
                                signedData = unsignedData;
                                if (signedData > 0) {
                                        CC = 0x50;
                                } else if (signedData == 0) {
                                        CC = 0x5A;
                                } else {
                                        CC = 0x4E;
                                }
                                break;
                        case 0x5000://AND
                                dr = (instructionArray[i] & 0xE00) >> 9;
                                sr1 = (instructionArray[i] & 0x1C0) >> 6;
                                data = registerArray[sr1];
                                modeType = (instructionArray[i] & 0x20) >> 5;
                                if (modeType == 0) {//register mode
                                        sr2 = instructionArray[i] & 0x7;
                                        data2 = registerArray[sr2];
                                        registerArray[dr] = data & data2;
                                }
                                else {//immediate mode
                                        data2 = instructionArray[i] & 0x1F;
                                        registerArray[dr] = data & data2;
                                }
                                signedData = data & data2;
                                if (signedData > 0) {
                                        CC = 0x50;
                                } else if (signedData == 0) {
                                        CC = 0x5A;
                                } else {
                                        CC = 0x4E;
                                }
                                break;
                        case 0x9000://NOT
                                dr = (instructionArray[i] & 0xE00) >> 9;
                                sr1 = (instructionArray[i] & 0x1C0) >> 6;
                                data = registerArray[sr1];
                                unsignedData = ~data;
                                signedData = ~data;
                                registerArray[dr] = unsignedData;
                                if (signedData > 0) {
                                        CC = 0x50;
                                } else if (signedData == 0) {
                                        CC = 0x5A;
                                } else {
                                        CC = 0x4E;
                                }
                                break;
                        case 0x1000://ADD
                                dr = (instructionArray[i] & 0xE00) >> 9;
                                sr1 = (instructionArray[i] & 0x1C0) >> 6;
                                data = registerArray[sr1];
                                modeType = (instructionArray[i] & 0x20) >> 5;
                                if (modeType == 0) {//register mode
                                        sr2 = instructionArray[i] & 0x7;
                                        data2 = registerArray[sr2];
                                }
                                else {//immediate mode
                                        data2 = instructionArray[i] & 0x1F;
                                        data2 = data2 << 11;
                                        data2 = data2 >> 11;
                                }
                                signedData = data + data2;
                                unsignedData = data + data2;
                                registerArray[dr] = unsignedData;
                                if (signedData > 0) {
                                        CC = 0x50;
                                } else if (signedData == 0) {
                                        CC = 0x5A;
                                } else {
                                        CC = 0x4E;
                                }
                                break;
                        case 0x0000://BR
                                BRn = (instructionArray[i] & 0x800) >> 11;
                                BRz = (instructionArray[i] & 0x400) >> 10;
                                BRp = (instructionArray[i] & 0x200) >> 9;
                                signedOffset = instructionArray[i] & 0x1FF;
                                changingI = signedOffset;
                                changingI = changingI << 7;
                                changingI = changingI >> 7;
                                if (BRn == 1 && CC == 0x4E) {
                                        PC = PC + changingI;
                                        i = i + changingI;
                                }
                                else if (BRz == 1 && CC == 0x5A) {
                                        PC = PC + changingI;
                                        i = i + changingI;
                                }
                                else if (BRp == 1 && CC == 0x50) {
                                        PC = PC + changingI;
                                        i = i + changingI;
                                }
                               printf("after executing instruction\t0x%04x\n", IR);
                               output(registerArray, PC, IR, CC);
                                break;
                }
        }
}
void output(int registerArray[], int PC, int IR, int CC) {
        printf("R0\t0x%04x\n", registerArray[0]);
        printf("R1\t0x%04x\n", registerArray[1]);
        printf("R2\t0x%04x\n", registerArray[2]);
        printf("R3\t0x%04x\n", registerArray[3]);
        printf("R4\t0x%04x\n", registerArray[4]);
        printf("R5\t0x%04x\n", registerArray[5]);
        printf("R6\t0x%04x\n", registerArray[6]);
        printf("R7\t0x%04x\n", registerArray[7]);
        printf("PC\t0x%04x\n", PC);
        printf("IR\t0x%04x\n", IR);
        printf("CC\t%c\n", CC);
        for(int i = 0; i < 18; i++) {
                printf("=");
        }
        printf("\n");
}