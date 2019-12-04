#ifndef FRUITY_CRUSH_H
#define FRUITY_CRUSH_H

void destrava_pino(uint32_t portal, uint8_t pino);
void habilita_matriz_botoes2(void);
void renovar_tabuleiro(void);
int ha_troca(void);
int ha_combinacao(void);
void imprimir_tabuleiro(void);
void qual_botao2(void);
int checagem(int tabu[4][4]);
int ha_comb_possiveis();
void setup_nivel(void);
void partida(void);
void jogo(void);

#endif
