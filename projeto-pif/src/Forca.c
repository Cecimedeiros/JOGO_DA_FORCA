#include "../include/Forca.h"
#include "../include/Normalizador.h"
#include "../include/Timer.h"
#include "../include/Screen.h"
#include "../include/Ranking.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_FRASES 100
#define MAX_ATTEMPTS 6

char** processamento_palavras(const char* filename, int* qtd_frases) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Erro ao abrir %s\n", filename);
        return NULL;
    }

    char** frases = malloc(MAX_FRASES * sizeof(char*));
    if (frases == NULL) {
        fclose(file);
        return NULL;
    }

    char buffer[256];
    *qtd_frases = 0;

    while (fgets(buffer, sizeof(buffer), file)) {
        buffer[strcspn(buffer, "\n")] = '\0';
        if (strlen(buffer) == 0) continue;

        frases[*qtd_frases] = malloc(strlen(buffer) + 1);
        strcpy(frases[*qtd_frases], buffer);
        (*qtd_frases)++;
    }

    fclose(file);
    return frases;
}

void desenhar_boneco(int tentativas, int x, int y) {
     
    screenGotoxy(x, y);     printf(" +----+");
    screenGotoxy(x, y + 1); printf(" |    |");
    screenGotoxy(x, y + 2); printf(" %c    |", (tentativas >= 1) ? 'O' : ' ');
    screenGotoxy(x, y + 3); printf("%c%c%c   |",
        (tentativas >= 3) ? '/' : ' ',
        (tentativas >= 2) ? '|' : ' ',
        (tentativas >= 4) ? '\\' : ' ');
    screenGotoxy(x, y + 4); printf("%c %c   |",
        (tentativas >= 5) ? '/' : ' ',
        (tentativas >= 6) ? '\\' : ' ');
    screenGotoxy(x, y + 5); printf("      |");
    screenGotoxy(x, y + 6); printf("=========\n");

}

void desenhar_jogo(const char* exibicao, int tentativas, const char* erradas, int vitorias) {
     
    int x = 10;
    int y = 6;

    screenGotoxy(x + 20, y);
    printf("=== JOGO DA FORCA ===");

    desenhar_boneco(tentativas, x + 25, y + 2);

    screenGotoxy(x, y + 10);
    printf("Pontos: %d", vitorias);

    screenGotoxy(x, y + 12);
    printf("Letras erradas: %s", erradas);

    screenGotoxy(x, y + 14);
    printf("Chances restantes: %d", MAX_ATTEMPTS - tentativas);

    screenGotoxy(x, y + 16);
    printf("Frase equivalente:");

    int len = strlen(exibicao);
    int centro = (MAXX + MINX - len) / 2;
    screenGotoxy(centro, y + 17);
    for (int i = 0; i < len && i < (MAXX - MINX - 2); i++) {
        putchar(exibicao[i]);
    }
    
}
int jogar_partida(const char* frase_equivalente, const char* frase_original, Jogo* jogo) {
    
    char* frase_limpa = strdup(frase_equivalente);
    frase_limpa[strcspn(frase_limpa, "\r\n")] = '\0';


    int tamanho = strlen(frase_limpa);
    char* exibicao = malloc(tamanho + 1);

    int total_para_acertar = 0;
    for (int i = 0; i < tamanho; i++) {
        if (frase_limpa[i] == ' ' || frase_limpa[i] == ',' || frase_limpa[i] == '.' || frase_limpa[i] == '-') {
            exibicao[i] = frase_limpa[i];
        } else {
            exibicao[i] = '_';
            total_para_acertar++;
        }
    }
    exibicao[tamanho] = '\0';

    jogo->tentativas = 0;
    jogo->acertos = 0;
    char erradas[50] = "";

    int tempo_inicial = (jogo->partidas_concluidas > 2) ? 60000 : 120000;
    timerInit(tempo_inicial);
    screenInit(1);

    while (jogo->tentativas < MAX_ATTEMPTS && jogo->acertos < total_para_acertar) {
        int largura_tela = 80;
        int centro = largura_tela / 2;

        screenGotoxy(centro - 15, 1);
        printf("🧠 ENFORCANDO A LÓGICA");

        screenGotoxy(centro - 28, 2);
        printf("Acerte a frase equivalente da original jogando forca!");

        screenGotoxy(centro - (strlen(frase_original) / 2), 3);
        printf("Frase original: %s", frase_original);

        int tempo_restante = tempo_inicial - getTimeDiff();
        screenGotoxy(centro - 15, 4);
        printf("⏱ Tempo restante: %d segundos", tempo_restante / 1000);

        desenhar_jogo(exibicao, jogo->tentativas, erradas, jogo->vitorias);

        if (timerTimeOver()) {
            screenInit(1);
            screenGotoxy(30, MAXY - 20);
            printf("⏰ O tempo acabou!");
            free(frase_limpa);
            free(exibicao);
            return -1;
        }

        char tentativa;
        screenGotoxy(10, MAXY - 3);
        printf("Digite uma letra (ou aperte '.' para sair): ");
        scanf(" %c", &tentativa);

        if (tentativa == '.') {
            screenDestroy();
            free(frase_limpa);
            free(exibicao);
            exit(0);
        }

        tentativa = tolower(tentativa);
        char tentativa_normalizada = remover_acento(tentativa);

        int ja_usou = 0;
        for (int i = 0; i < tamanho; i++) {
            if (tolower(remover_acento(exibicao[i])) == tentativa_normalizada) {
                ja_usou = 1;
                break;
            }
        }
        for (int i = 0; erradas[i] != '\0'; i++) {
            if (tolower(erradas[i]) == tentativa_normalizada) {
                ja_usou = 1;
                break;
            }
        }
        if (ja_usou) continue;

        int acertou = 0;
        for (int i = 0; i < tamanho; i++) {
            if (remover_acento(tolower(frase_limpa[i])) == tentativa_normalizada && exibicao[i] == '_') {
                exibicao[i] = frase_limpa[i];
                jogo->acertos++;
                acertou = 1;
            }
        }

        if (!acertou) {
            int len = strlen(erradas);
            erradas[len] = tentativa;
            erradas[len + 1] = '\0';
            jogo->tentativas++;
        }
    }

    screenInit(1);
    screenGotoxy(20, MAXY - 20);
    if (jogo->acertos == total_para_acertar) {
        printf("🎉 Parabéns! Você acertou a frase!");
        jogo->vitorias++; 
        salvar_pontuacao(jogo->nome, 1); 
    } else {
        const char* msg_derrota = "💀 Você perdeu!";
        int x_derrota = (MAXX - strlen(msg_derrota)) / 2;
        screenGotoxy(x_derrota, MAXY - 20);
        printf("%s", msg_derrota);

    }

    free(frase_limpa);
    free(exibicao);
    return 0;
}

