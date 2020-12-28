# Fruity Crush
> Uma cópia do jogo Candy Crush desenvolvido como projeto final da disciplina de Microcontroladores (EMB5642) na UFSC - Campus Joinville em 2019.2

O jogo foi desenvolvido em C para o microcontrolador Tiva-C TM4C123G acoplado a uma [PCB](/PCB) e a um display LCD Nokia 5110. Ele foi desenvolvido no Code Composer Studio, e todos os sprites utilizados são de autoria própria, tendo sido transformados em bitmaps neste [site](https://sparks.gogo.co.nz/pcd8554-bmp.html).

## Como jogar

O jogo utiliza a matriz de botões 4x4 da PCB para ser jogado, cada botão correspondendo a uma casa do tabuleiro. Para mudar as frutas de lugar deve-se clicar nas duas frutas desejadas: a troca será feita se elas estiveram uma ao lado da outra. Caso a troca não leve nenhuma combinação de frutas a serem estouradas, as frutas retornarão a seus lugares iniciais.

## Regras

As regras do jogo se inspiram nas regras do Candy Crush, havendo modificações devido as restrições da tela. Elas podem ser acessadas na seção 'Como jogar' na tela de opções do jogo ou serem vistas [aqui](/Sprites/Instrucoes).


