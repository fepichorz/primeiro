#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#define LARGURA 40
#define ALTURA 20
#define MAX_CORPO (LARGURA * ALTURA)

#define CIMA 0
#define BAIXO 1
#define ESQ 2
#define DIR 3

typedef struct {
    int x;
    int y;
} Ponto;

static struct termios term_orig;
static int stdin_flags_orig;
static int terminal_configurado = 0;

static void restaurar_terminal(void) {
    if (!terminal_configurado) {
        return;
    }
    tcsetattr(STDIN_FILENO, TCSANOW, &term_orig);
    fcntl(STDIN_FILENO, F_SETFL, stdin_flags_orig);
    printf("\x1b[0m\x1b[?25h\x1b[2J\x1b[H");
    fflush(stdout);
    terminal_configurado = 0;
}

static void configurar_terminal(void) {
    struct termios raw;

    if (tcgetattr(STDIN_FILENO, &term_orig) == -1) {
        perror("tcgetattr");
        exit(1);
    }
    stdin_flags_orig = fcntl(STDIN_FILENO, F_GETFL, 0);
    raw = term_orig;
    raw.c_lflag &= (tcflag_t) ~(ICANON | ECHO);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
    fcntl(STDIN_FILENO, F_SETFL, stdin_flags_orig | O_NONBLOCK);
    printf("\x1b[?25l");
    fflush(stdout);
    terminal_configurado = 1;
    atexit(restaurar_terminal);
}

static int tecla_disponivel(void) {
    int n = 0;
    if (ioctl(STDIN_FILENO, FIONREAD, &n) == -1) {
        return 0;
    }
    return n;
}

static int ler_tecla(void) {
    unsigned char c;
    if (read(STDIN_FILENO, &c, 1) != 1) {
        return -1;
    }
    if (c == 27) {
        unsigned char seq[2];
        usleep(8000);
        if (read(STDIN_FILENO, &seq[0], 1) != 1) {
            return 27;
        }
        if (seq[0] == '[' && read(STDIN_FILENO, &seq[1], 1) == 1) {
            if (seq[1] == 'A') return 'w';
            if (seq[1] == 'B') return 's';
            if (seq[1] == 'C') return 'd';
            if (seq[1] == 'D') return 'a';
        }
        return 27;
    }
    return (int)c;
}

static int colisao_corpo(Ponto *corpo, int tamanho, int x, int y, int ignorar_cauda) {
    int i;
    int limite = ignorar_cauda ? tamanho - 1 : tamanho;
    for (i = 1; i < limite; i++) {
        if (corpo[i].x == x && corpo[i].y == y) {
            return 1;
        }
    }
    return 0;
}

static void gerar_comida(Ponto *comida, Ponto *corpo, int tamanho) {
    do {
        comida->x = rand() % LARGURA;
        comida->y = rand() % ALTURA;
    } while (colisao_corpo(corpo, tamanho, comida->x, comida->y, 0));
}

static void desenhar(Ponto *corpo, int tamanho, Ponto comida, int pontuacao, int recorde) {
    int x, y, i;
    char mapa[ALTURA][LARGURA];

    memset(mapa, ' ', sizeof(mapa));
    mapa[comida.y][comida.x] = '*';
    for (i = 1; i < tamanho; i++) {
        mapa[corpo[i].y][corpo[i].x] = 'o';
    }
    mapa[corpo[0].y][corpo[0].x] = 'O';

    printf("\x1b[H");
    printf("\x1b[1;32m  COBRINHA\x1b[0m   Pontos: \x1b[1;33m%03d\x1b[0m   Recorde: \x1b[1;36m%03d\x1b[0m\n",
           pontuacao, recorde);
    printf("  WASD ou setas para mover  |  Q para sair\n");

    printf("  \x1b[32m+");
    for (x = 0; x < LARGURA; x++) {
        putchar('-');
    }
    printf("+\x1b[0m\n");

    for (y = 0; y < ALTURA; y++) {
        printf("  \x1b[32m|\x1b[0m");
        for (x = 0; x < LARGURA; x++) {
            if (mapa[y][x] == 'O') {
                printf("\x1b[1;32mO\x1b[0m");
            } else if (mapa[y][x] == 'o') {
                printf("\x1b[32mo\x1b[0m");
            } else if (mapa[y][x] == '*') {
                printf("\x1b[1;31m*\x1b[0m");
            } else {
                putchar(' ');
            }
        }
        printf("\x1b[32m|\x1b[0m\n");
    }

    printf("  \x1b[32m+");
    for (x = 0; x < LARGURA; x++) {
        putchar('-');
    }
    printf("+\x1b[0m\n");
    fflush(stdout);
}

static int jogar(int *recorde) {
    Ponto corpo[MAX_CORPO];
    Ponto comida;
    int tamanho = 3;
    int dir = DIR;
    int proxima_dir = DIR;
    int pontuacao = 0;
    int i;
    int tecla;
    int atraso_us = 120000;

    corpo[0].x = LARGURA / 2;
    corpo[0].y = ALTURA / 2;
    corpo[1].x = corpo[0].x - 1;
    corpo[1].y = corpo[0].y;
    corpo[2].x = corpo[0].x - 2;
    corpo[2].y = corpo[0].y;
    gerar_comida(&comida, corpo, tamanho);

    printf("\x1b[2J");
    desenhar(corpo, tamanho, comida, pontuacao, *recorde);

    while (1) {
        usleep((useconds_t)atraso_us);

        while (tecla_disponivel() > 0) {
            tecla = ler_tecla();
            if (tecla == 'q' || tecla == 'Q') {
                return -1;
            }
            if ((tecla == 'w' || tecla == 'W') && dir != BAIXO) {
                proxima_dir = CIMA;
            } else if ((tecla == 's' || tecla == 'S') && dir != CIMA) {
                proxima_dir = BAIXO;
            } else if ((tecla == 'a' || tecla == 'A') && dir != DIR) {
                proxima_dir = ESQ;
            } else if ((tecla == 'd' || tecla == 'D') && dir != ESQ) {
                proxima_dir = DIR;
            }
        }
        dir = proxima_dir;

        for (i = tamanho - 1; i > 0; i--) {
            corpo[i] = corpo[i - 1];
        }

        if (dir == CIMA) {
            corpo[0].y--;
        } else if (dir == BAIXO) {
            corpo[0].y++;
        } else if (dir == ESQ) {
            corpo[0].x--;
        } else {
            corpo[0].x++;
        }

        if (corpo[0].x < 0 || corpo[0].x >= LARGURA ||
            corpo[0].y < 0 || corpo[0].y >= ALTURA ||
            colisao_corpo(corpo, tamanho, corpo[0].x, corpo[0].y, 1)) {
            if (pontuacao > *recorde) {
                *recorde = pontuacao;
            }
            desenhar(corpo, tamanho, comida, pontuacao, *recorde);
            return pontuacao;
        }

        if (corpo[0].x == comida.x && corpo[0].y == comida.y) {
            if (tamanho < MAX_CORPO) {
                corpo[tamanho] = corpo[tamanho - 1];
                tamanho++;
            }
            pontuacao += 10;
            if (atraso_us > 50000) {
                atraso_us -= 2000;
            }
            gerar_comida(&comida, corpo, tamanho);
        }

        desenhar(corpo, tamanho, comida, pontuacao, *recorde);
    }
}

int main(void) {
    int recorde = 0;
    int pontos;
    int tecla;
    int jogar_de_novo = 1;

    srand((unsigned)time(NULL));
    configurar_terminal();

    while (jogar_de_novo) {
        pontos = jogar(&recorde);
        if (pontos < 0) {
            break;
        }

        printf("\n  \x1b[1;31mFim de jogo!\x1b[0m  Pontuacao: %d   Recorde: %d\n", pontos, recorde);
        printf("  Pressione R para jogar de novo ou Q para sair.\n");
        fflush(stdout);

        while (1) {
            tecla = ler_tecla();
            if (tecla == 'q' || tecla == 'Q') {
                jogar_de_novo = 0;
                break;
            }
            if (tecla == 'r' || tecla == 'R') {
                break;
            }
            usleep(50000);
        }
    }

    restaurar_terminal();
    printf("Obrigado por jogar Cobrinha!\n");
    return 0;
}
