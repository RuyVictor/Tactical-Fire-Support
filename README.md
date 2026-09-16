# Tactical Fire Support

Implementacao inicial de apoio de artilharia para Arma Reforger.

## Fluxo no jogo

1. Abra o mapa.
2. Clique com o botao direito no ponto desejado para abrir o radial do mapa.
3. Selecione **Apoio de artilharia**.
4. O servidor valida a solicitacao e dispara uma salva de seis granadas de morteiro naquele ponto.

Cada jogador possui um intervalo de 60 segundos entre salvas e o alcance maximo e de 10 km.

## Arquitetura

- A entrada e adicionada ao radial do mapa (`SCR_MapRadialUI`) aberto com o botao direito.
- O comando tambem e registrado no sistema de comando, sem substituir `Commands.conf` ou `CommandingMenu.conf` do jogo.
- O ponto onde o radial foi aberto e usado como alvo no cliente solicitante.
- A validacao e o spawn da salva acontecem no servidor.
- Os projeteis usam o sistema nativo de `SCR_BarrageEffectsModule` e sao replicados para os clientes.

## Importacao no Workbench

O prefab ja foi indexado, o `resourceDatabase.rdb` foi atualizado e os scripts passaram pela validacao do Workbench. Se o projeto ja estava aberto antes destas alteracoes, use **Script Editor > Compile and Reload Scripts** uma vez para recarrega-los na sessao atual.

Para multiplayer, habilite o addon no servidor e nos clientes. O prefab e o script precisam estar presentes nos dois lados.
