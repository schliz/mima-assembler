//
// Dies ist ein einfacher MIMA-Bytecode Interpreter
//
// Autor: Thorsten Rapp, 01.06.2004
// Co-Autor: Christian Schliz, 16.11.2021
//

#include <stdio.h>
#include <string.h>
#include "../opcodes.h"

//
// Kompilieren: gcc -o runmima main.c
// Aufruf: ./runmima programm.mic
//
// Das Format der Datei .mic sieht wie folgt aus:
//
// 0-4. Byte: MIMA\0     Erkenner für MIMA Bytecode
// 5-7. Byte: 0x??????   IAR Anfangsinhalt
// 8. Byte: Anzahl von Eingabefelder
// 9. Byte: Anzahl von Ausgabefelder
// Liste von Eingabezellenadressen
// Liste von Ausgabezellenadressen
// Speicherinhalt ab Zelle 0 bis nach Bedarf
//

int mem[0x0FFFFF];  // A0-A19 = 2^20 Adressen zu je 3 Byte
int IAR;            // Instruktions-Adress-Register
int akku;           // Akkumulator
int IR;             // Instruktionsregister
int print[256];     // Ausgabefelder
int ask[256];       // Eingabefelder

int running = 1;

// Ausführungsphase:

void ins_LDC() {
    akku = IR & 0xFFFFF;
}

void ins_LDV() {
    akku = mem[IR & 0xFFFFF];
}

void ins_STV() {
    mem[IR & 0xFFFFF] = akku;
}

void ins_ADD() {
    akku += mem[IR & 0xFFFFF];
    akku %= 0x1000000;
}

void ins_AND() {
    akku &= mem[IR & 0xFFFFF];
}

void ins_OR() {
    akku |= mem[IR & 0xFFFFF];
}

void ins_XOR() {
    akku ^= mem[IR & 0xFFFFF];
}

void ins_EQL() {
    if (mem[IR & 0xFFFFF] == akku) {
        akku = -1;
    } else {
        akku = 0;
    }
}

void ins_JMP() {
    IAR = IR & 0xFFFFF;
}

void ins_JMN() {
    if ((akku & 0x800000) != 0) {
        IAR = IR & 0xFFFFF;
    }
}

void ins_LDIV() {
    akku = mem[mem[IR & 0xFFFFF]];
}

void ins_STIV() {
    mem[mem[IR & 0xFFFFF]] = akku;
}

void ins_JMS() {
    mem[IR & 0xFFFFF] = IAR;
    IAR = (IR & 0xFFFFF) + 1;
}

void ins_JIND() {
    IAR = mem[IR & 0xFFFFF];
}

void ins_HALT() {
    printf("\nAusgabe der ausgesuchten Speicherzellen:\n");

    int i;
    for (i = 0; print[i] != -1; i++) {
        printf("[0x%05X]: 0x%06X ; Dezimal:(%i)\n", print[i], mem[print[i]], mem[print[i]]);
    }

    printf("\n");
    running = 0;
}

void ins_NOT() {
    akku = ~akku;
    akku &= 0xFFFFFF;
}

void ins_RAR() {
    int overflow = (akku & 1);
    akku >>= 1;
    overflow <<= 23;
    akku |= overflow;
    akku %= 0x1000000;
}

void ins_ERROR() {
    printf("Fehler im Bytecode: Unbekannter OpCode!\n");
    running = 0;
}

int main(__attribute__((unused)) int argc, char *argv[]) {
    printf("MIMA - Bytecode Interpreter (c) Technische Informatik II\n\n");

    if (!(argv[1])) {
        printf("Aufruf: mima programm.mic\n");
        return 1;
    }

    // einlesen:
    // Der Dateieinleseteil ist zweckmäßig aber nicht schön programmiert.
    // Sorry, aber die Hauptkonzentration war den MIMA Programmen selbst
    // gewidmet. Dieser Interpreter ist nur ein Bonus.

    FILE *ptr;

    int c1;
    int c2;
    int c3;

    ptr = fopen(argv[1], "rb");
    int i;

    char mimacheck[10];
    fgets(mimacheck, 5, ptr);

    if ((strcmp(mimacheck, "MIMA") != 0) | (fgetc(ptr) != 0)) {
        printf("Kein MIMA Bytecode!\n");
        return 1;
    }

    IAR = fgetc(ptr) * 256 * 256 + fgetc(ptr) * 256 + fgetc(ptr);

    int inputlaenge = fgetc(ptr);
    int outputlaenge = fgetc(ptr);

    for (i = 0; i < inputlaenge; i++) {
        ask[i] = fgetc(ptr) * 256 * 256 + fgetc(ptr) * 256 + fgetc(ptr);
    }

    ask[i] = -1;

    for (i = 0; i < outputlaenge; i++) {
        print[i] = fgetc(ptr) * 256 * 256 + fgetc(ptr) * 256 + fgetc(ptr);
    }

    print[i] = -1;

    // mem[] herstellen
    i = 0;
    while ((c1 = fgetc(ptr)) != EOF) {
        c2 = fgetc(ptr);
        c3 = fgetc(ptr);
        mem[i] = c1 * 256 * 256 + c2 * 256 + c3;
        i++;
    }

    fclose(ptr);

    // ausführen:

    for (i = 0; ask[i] != -1; i++) {
        printf("Bitte Inhalt für Zelle [0x%05X] eingeben: ", ask[i]);
        scanf("%i", &mem[ask[i]]);
    }

    while (running) {
        // Fetch-Phase:
        IR = mem[IAR];
        IAR++;

        // Dekodier-Phase

        int OP1 = IR & 0xF00000; // first opcode bit
        OP1 >>= 20;

        int OP2 = IR & 0x0F0000; // second opcode bit (used if applies)
        OP2 >>= 16;

        switch (OP1) {
            case (LDC):
                ins_LDC();
                break;
            case (LDV):
                ins_LDV();
                break;
            case (STV):
                ins_STV();
                break;
            case (ADD):
                ins_ADD();
                break;
            case (AND):
                ins_AND();
                break;
            case (OR):
                ins_OR();
                break;
            case (XOR):
                ins_XOR();
                break;
            case (EQL):
                ins_EQL();
                break;
            case (JMP):
                ins_JMP();
                break;
            case (JMN):
                ins_JMN();
                break;
            case (LDIV):
                ins_LDIV();
                break;
            case (STIV):
                ins_STIV();
                break;
            case (JMS):
                ins_JMS();
                break;
            case (JIND):
                ins_JIND();
                break;
            case (0xF):
                switch (OP2) {
                    case (HALT & 0x0F):
                        ins_HALT();
                        break;
                    case (NOT & 0x0F):
                        ins_NOT();
                        break;
                    case (RAR & 0x0F):
                        ins_RAR();
                        break;
                    default:
                        ins_ERROR();
                        break;
                } // switch second opcode bit
                break;
            default:
                ins_ERROR();
                break;
        } // switch first opcode bit
    } // execution loop

    return 0;
}
