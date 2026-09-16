# Tactical Fire Support

Implementacao inicial de apoio de artilharia para Arma Reforger.

## Fluxo no jogo

1. Abra o mapa.
2. Clique com o botao direito no ponto desejado para abrir o radial do mapa.
3. Selecione **Apoio de artilharia**.
4. O servidor valida a solicitacao e dispara a salva configurada no prefab naquele ponto.

Cada jogador possui por padrao um intervalo de 60 segundos entre salvas e o alcance maximo e de 10 km.

## Configuracao

Abra `Configs/TFS_ArtillerySettings.conf` no Config Editor e altere `m_iCooldownSeconds`:

- `60`: intervalo padrao de um minuto por jogador.
- `10`: dez segundos entre solicitacoes.
- `0`: desabilita completamente o intervalo.

A configuracao e lida pelo servidor a cada solicitacao. Em multiplayer, vale o arquivo empacotado no addon carregado pelo servidor.

O mesmo arquivo controla a sequencia de fogo:

- `m_fBarrageDelaySeconds`: tempo entre aceitar a solicitacao e iniciar a salva.
- `m_fSplashLeadSeconds`: antecedencia do aviso **SPLASH**.
- `m_bEnableRadioCues`: habilita mensagens **FIRE MISSION**, **SHOT** e **SPLASH**.
- `m_bPlayRadioCueSound`: toca um aviso sonoro discreto junto das mensagens.

O cue atual usa um som de interface do jogo. Voz, estatica de radio e amostras personalizadas exigem um projeto `.acp` e arquivos de audio licenciados.

## Flyby e near miss

O TFS mantem o prefab balistico do barrage vanilla, portanto nao substitui os eventos nativos de flyby. Para `SOUND_FLYBY` e sonic crack funcionarem, o mundo precisa conter o modulo vanilla `Prefabs/Sounds/Weapons/ProjectileSoundsModule.et`. Os cenarios oficiais normalmente ja fornecem esse sistema; confirme a presenca dele em mundos personalizados.

## Arquitetura

- A entrada e adicionada ao radial do mapa (`SCR_MapRadialUI`) aberto com o botao direito.
- O comando tambem e registrado no sistema de comando, sem substituir `Commands.conf` ou `CommandingMenu.conf` do jogo.
- O ponto onde o radial foi aberto e usado como alvo no cliente solicitante.
- A validacao e o spawn da salva acontecem no servidor.
- Os projeteis usam o sistema nativo de `SCR_BarrageEffectsModule` e sao replicados para os clientes.

## Importacao no Workbench

O prefab ja foi indexado, o `resourceDatabase.rdb` foi atualizado e os scripts passaram pela validacao do Workbench. Se o projeto ja estava aberto antes destas alteracoes, use **Script Editor > Compile and Reload Scripts** uma vez para recarrega-los na sessao atual.

Para multiplayer, habilite o addon no servidor e nos clientes. O prefab e o script precisam estar presentes nos dois lados.
