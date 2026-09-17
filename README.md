# Tactical Fire Support

Implementacao inicial de apoio de artilharia para Arma Reforger.

## Fluxo no jogo

1. Abra o mapa.
2. Clique com o botao direito no ponto desejado para abrir o radial do mapa.
3. Selecione **Artilharia 240 mm**.
4. O servidor valida a solicitacao, marca o ponto de impacto para a faccao aliada e dispara um unico projetil pesado de 240 mm equivalente naquele ponto.

Cada jogador possui por padrao um intervalo de 60 segundos entre disparos e o alcance maximo e de 10 km.

## Municao de 240 mm

O jogo base desta versao oferece projeteis de morteiro de 81/82 mm, mas nao um recurso nativo de 240 mm. O TFS cria uma variante independente, sem dependencia de outro mod, com:

- um unico projetil por solicitacao;
- massa de 130,84 kg, diametro balistico de 240 mm e comprimento de 1.565 mm;
- carga real de 31,93 kg de TNT e 98,91 kg de massa nao explosiva usada como aproximacao do corpo para fragmentacao;
- constante de Gurney de 2.380 m/s e 9.900 fragmentos simulados;
- efeito central **TNT Large** do jogo base, mais 12 plumas visuais distribuidas em dois aneis ate 18 m do impacto e decal ampliado para escala 28;
- incoming, whistle e near miss espacializados no proprio projetil;
- impacto customizado em camadas: ataque, shockwave, low boom, cauda e detritos;
- disparo distante de uma bateria virtual, replicado para os clientes.

O prefab em voo herda do projetil de 81 mm do jogo base e grava diretamente `scale 2.963` na transformacao raiz (`240 / 81`). Nao ha mais um componente de script apenas para escala. Isso deixa o volume visivel coerente com o calibre e conserva separadamente a configuracao balistica de 240 mm, 130,84 kg e 1.565 mm. A silhueta ainda e a da municao de 81 mm; somente uma malha 3D propria permitiria reproduzir com exatidao ogiva, cinta de forcamento e empenagem de um projetil real de 240 mm.

Os valores fisicos usam como referencia o projetil sovietico F-864 empregado pelo morteiro M-240/2S4: 130,84 kg de massa total e 31,93 kg de TNT. A massa restante e uma aproximacao de simulacao para `CaseWeight`, pois o valor real inclui espoleta e outros componentes alem do corpo fragmentavel. Os 9.900 fragmentos seguem a densidade aproximada de 100 fragmentos simulados por kg de corpo observada em explosivos vanilla; nao sao uma contagem historica declarada.

## Configuracao

Abra `Configs/TFS_ArtillerySettings.conf` no Config Editor. O arquivo ja contem explicitamente todos os valores recomendados, inclusive os tempos, para que nao seja necessario depender dos defaults escondidos no script.

Para `m_iCooldownSeconds`:

- `60`: intervalo padrao de um minuto por jogador.
- `10`: dez segundos entre solicitacoes.
- `0`: desabilita completamente o intervalo.

A configuracao e lida pelo servidor a cada solicitacao. Em multiplayer, vale o arquivo empacotado no addon carregado pelo servidor.

O mesmo arquivo controla a sequencia de fogo:

- `m_fInitialMissionDelaySeconds`: espera silenciosa depois da selecao do alvo e antes de confirmar a missao; padrao de 2 s e limite de 30 s. Esse tempo tambem posterga o disparo real para manter a sequencia sincronizada.
- `m_fBarrageDelaySeconds`: tempo entre a confirmacao da missao e iniciar o ataque; padrao de 20 s.
- `m_fShotCueDelaySeconds`: atraso depois da confirmacao para executar juntos o aviso **SHOT** e o som da bateria; padrao de 3,75 s e nunca maior que `m_fBarrageDelaySeconds`.
- `m_fSplashLeadSeconds`: antecedencia do aviso **ROUNDS INCOMING** e do alerta aos aliados proximos; padrao de 12 s, deixando a ordem de evacuacao terminar antes do ataque. O valor efetivo nunca ultrapassa `m_fBarrageDelaySeconds`, evitando que o alerta seja enviado antes da confirmacao da missao.
- `m_fRadioInterCueDelaySeconds`: silencio geral entre transmissoes enfileiradas; padrao de 1,25 s e limite de 5 s.
- `m_fAlertToneToDangerCloseDelaySeconds`: pausa especifica depois do beep e antes de **DANGER CLOSE**; padrao recomendado de 1,25 s e limite de 5 s.
- `m_fDangerCloseToClearAreaDelaySeconds`: pausa entre **DANGER CLOSE** e **CLEAR AREA**; padrao recomendado de 1,25 s e limite de 5 s.
- `m_fMarkerPostImpactLifetimeSeconds`: tempo durante o qual o marcador permanece no mapa depois da detonacao real; o padrao recomendado e `0`, removendo-o imediatamente, e o limite e 60 s.
- `m_bEnableRadioCues`: habilita mensagens **FIRE MISSION**, **SHOT**, **ROUNDS INCOMING** e **DANGER CLOSE**.
- `m_bPlayRadioCueSound`: toca as vozes de radio customizadas junto das mensagens.
- `m_bShowImpactMarker`: mostra no mapa um alvo vermelho sincronizado, visivel apenas para a faccao solicitante enquanto a missao estiver ativa.
- `m_fDangerCloseRadiusMeters`: raio, de 0 a 2.000 m, dentro do qual aliados recebem **DANGER CLOSE** com o beep de alerta; o padrao recomendado e 300 m.
- `m_bEnableDistantFireSound`: habilita o estampido distante da bateria de 240 mm.
- `m_fDistantFireOffsetMeters`: distancia virtual do som da bateria em relacao ao solicitante; aceita de 0 a 10.000 m e usa 2.500 m por padrao.
- `m_iAudioRangeProfile`: perfil conjunto do disparo, incoming e impacto customizado:
  - `0`: padrao, com ganho mais contido;
  - `1`: ampliado, configuracao recomendada para uso normal;
  - `2`: extremo, configuracao atual de teste, para tornar as camadas muito evidentes mesmo a longa distancia.

O perfil unico evita controles redundantes para cada camada. A aproximacao e o flyby nao usam um atraso em segundos: acompanham dinamicamente a posicao, velocidade e passagem do projetil.

Com os valores padrao, a linha do tempo fica: alvo selecionado em 0 s, **Fire mission received** em 2 s, **Shot** e estampido da bateria em aproximadamente 5,75 s, aviso final em 10 s e inicio do ataque em 22 s. O alerta proximo ocupa aproximadamente 10,3 s: beep, 1,25 s de silencio, **Danger close**, outra pausa de 1,25 s e **Clear the target area immediately**. Assim a ultima fala termina antes do ataque e ainda resta a queda real do projetil ate a explosao. Como regra segura, mantenha `m_fSplashLeadSeconds` maior que `7,8 + m_fAlertToneToDangerCloseDelaySeconds + m_fDangerCloseToClearAreaDelaySeconds`.

Os WAV de artilharia adicionados ao projeto ja estao registrados e distribuidos em tres projetos de audio:

- `Sounds/Artillery/TFS_ArtilleryFire.acp`: disparo distante e cauda da bateria;
- `Sounds/Artillery/TFS_ArtilleryIncoming.acp`: air rush, whistle e crack/near miss;
- `Sounds/Artillery/TFS_ArtilleryImpact.acp`: camadas customizadas do impacto.

As vozes ficam em `Sounds/Radio/TFS_ArtilleryRadio.acp`. O radio e reproduzido localmente para cada destinatario, sem atenuacao por distancia, como uma comunicacao recebida no equipamento do jogador. Uma fila impede sobreposicao entre as falas e prioriza o aviso **DANGER CLOSE**.

O impacto usa somente o projeto de audio customizado. A particula **TNT Large** continua sendo usada no VFX, mas seu `SOUND_EXPLOSION` vanilla foi removido para nao duplicar o impacto customizado.

### Uso das amostras de audio

Todos os 22 WAV de artilharia e os 11 WAV de radio estao conectados aos projetos de audio. Eles nao tocam todos simultaneamente: os bancos combinam camadas fixas com escolhas aleatorias para evitar que cada disparo seja identico.

- bateria, 4 WAV: uma das duas variacoes de disparo e uma das duas caudas sao escolhidas, sem empilhar os dois disparos;
- incoming, 8 WAV: tres air rush, tres whistles e dois cracks alimentam as camadas de aproximacao, passagem e near miss;
- impacto, 10 WAV: ataque inicial, shockwave, low boom, cauda e detritos; cada banco com varias amostras escolhe uma variacao.
- radio, 11 WAV: duas variacoes para **Fire mission received**, **Shot**, **Rounds incoming**, **Danger close** e **Clear area**, mais o beep; o beep agora e um evento separado, seguido de uma pausa antes da fala e da ordem para evacuar.

Os 11 WAV de radio foram normalizados para aproximadamente -18 LUFS sem comprimir sua dinamica. Todas as falas usam -2 dB no banco e chegam a aproximadamente -20 LUFS na reproducao. O beep usa -4 dB e chega a aproximadamente -22 LUFS, dois decibeis abaixo das vozes porque seu tom concentrado e percebido como mais alto. Isso mantem as variacoes e os diferentes comandos audiveis e no mesmo nivel percebido.

Ao longo de varios disparos, todos podem ser ouvidos. Em um unico impacto, toca uma variacao de cada banco, evitando empilhar varias versoes equivalentes do mesmo som.

O disparo da bateria e enviado uma unica vez por jogador. O incoming e o impacto tambem possuem uma trava local de reproducao unica; camadas como crack, shockwave, low boom, cauda e detritos continuam combinadas intencionalmente, mas nenhum evento vanilla equivalente toca em paralelo.

Os limites maximos dos grafos sao 12 km para a bateria, 10 km para as camadas principais do impacto, 4 km para a aproximacao/incoming e 1,6 km para o flyby proximo. Os detritos possuem uma curva separada: volume base de -22 dB, queda forte depois de 35 m e corte em 300 m. Esses sao limites de processamento; atenuacao, ambiente e o perfil escolhido ainda determinam o volume percebido.

### Marcador e avisos da missao

Ao aceitar uma solicitacao, o servidor cria um marcador vermelho **IMPACTO ARTILHARIA 240 MM** exatamente nas coordenadas escolhidas. O marcador:

- e sincronizado pelo `SCR_MapMarkerManagerComponent` e filtrado pela faccao do solicitante;
- nao consome o limite de marcadores pessoais e nao pode ser removido por um jogador;
- permanece durante a missao e e retirado depois do tempo configurado em `m_fMarkerPostImpactLifetimeSeconds`;
- possui uma expiracao de seguranca entre 120 e 600 segundos caso o projetil ou o efeito de impacto falhe.

A remocao principal e confirmada no servidor pelo ciclo de vida do projetil filho da barragem, sem depender da criacao do VFX no cliente. Quando o projétil desaparece após a colisão, o marcador associado ao alvo e removido; a notificacao do prefab visual permanece apenas como caminho redundante.

Na etapa configurada por `m_fSplashLeadSeconds`, o solicitante recebe **Rounds incoming**. Jogadores aliados que estiverem dentro de `m_fDangerCloseRadiusMeters` do alvo recebem primeiro o beep, depois **Danger close. Incoming artillery** e, por ultimo, **Clear the target area immediately**.

Se o proprio solicitante estiver dentro do raio de perigo, o aviso urgente **Danger close** substitui **Rounds incoming** para evitar duas transmissoes concorrentes e uma fila de radio artificialmente longa.

O codigo de radio ja separa cue e idioma (`ENGLISH`/`RUSSIAN`) e usa nomes de eventos com sufixo de idioma. Enquanto nao houver bancos russos, qualquer idioma ainda recai deliberadamente nos eventos ingleses. Quando as amostras russas forem adicionadas, sera necessario criar os cinco eventos `_RUSSIAN` no projeto de audio e habilitar a selecao correspondente; nao e preciso alterar o fluxo da missao.

## VFX de impacto

Aumentar somente a escala da entidade de uma particula nao amplia de forma confiavel o tamanho interno de todos os emissores da `.ptc`. Por isso o impacto usa uma composicao visual no proprio prefab da ogiva: uma TNT Large central e 12 instancias adicionais em dois aneis. Isso cria uma pegada visual de aproximadamente 50 m sem multiplicar o dano, que continua sendo calculado uma unica vez pela ogiva.

Uma `.ptc` Extra Large autorada manualmente no Particle Editor ainda seria a melhor opcao para uma producao final com menor custo de renderizacao e uma coluna totalmente continua. Nesse caso, seria necessario duplicar a TNT Large e ajustar o `Size Multiplier` de cada emissor individualmente; o sistema atual nao exige esse trabalho manual.

## Incoming, flyby e near miss

O prefab de 240 mm modifica o mesmo `SCR_ShellSoundComponent` herdado do projetil de 81 mm e substitui sua lista `Filenames` por apenas `TFS_ArtilleryIncoming.acp`. Portanto, o componente e a atualizacao de sinais sao herdados, mas nenhum projeto de audio vanilla de 81 mm permanece conectado. `TFS_ArtilleryAudioProfileComponent` dispara explicitamente um dos eventos `TFS_INCOMING_STANDARD`, `TFS_INCOMING_EXTENDED` ou `TFS_INCOMING_EXTREME` desse ACP customizado.

Uma camada longa de `Whistle` nasce junto com o projetil e o acompanha continuamente. `AirRush` possui bancos separados para aproximacao, afastamento e flyby distante; `Crack` fica reservado ao gate de passagem proxima. Os sinais vanilla de velocidade, direcao e distancia continuam sendo usados para abrir e fechar essas camadas no momento adequado, conforme o funcionamento documentado do `SCR_ShellSoundComponent`.

A modulacao vanilla agressiva de pitch foi removida dos tres eventos, de todos os bancos `AirRush`, dos dois bancos de `Whistle` e do banco de `Crack`. Todos agora usam `Pitch 0`, apenas com variacao aleatoria discreta entre 0,1 e 0,2. Os fades foram alongados para a aproximacao entrar gradualmente: 1,2 s na camada longa, 0,8 s no AirRush de aproximacao e 0,6 s no Whistle proximo. Assim os WAV preservam sua velocidade e desenho originais; distancia e direcao alteram principalmente a presenca e o volume, nao a velocidade de reproducao.

O `ProjectileSoundsModule` global e um caminho separado do `SCR_ShellSoundComponent`: ele pode tocar `SOUND_FLYBY` e `SOUND_SONIC_CRACK` a partir dos recursos definidos no `ShellMoveComponent`. Para impedir que os recursos herdados do projetil vanilla de 81 mm sejam sobrepostos ao desenho customizado, o prefab de 240 mm limpa explicitamente `SonicCracks` e `SubsonicFlybys`. Nesta versao, `AirRush` e `Crack` pertencem exclusivamente ao `TFS_ArtilleryIncoming.acp`. Se futuramente forem desejados eventos globais com propagacao fisica, sera necessario criar eventos `SOUND_*` customizados e registrar o respectivo ACP no modulo do mundo.

## Arquitetura

- A entrada e adicionada ao radial do mapa (`SCR_MapRadialUI`) aberto com o botao direito.
- O comando tambem e registrado no sistema de comando, sem substituir `Commands.conf` ou `CommandingMenu.conf` do jogo.
- O ponto onde o radial foi aberto e usado como alvo no cliente solicitante.
- A validacao e o spawn do disparo acontecem no servidor.
- O marcador de impacto e criado no servidor, replicado somente para aliados e removido pelo efeito da detonacao real.
- O projetil usa o sistema nativo de `SCR_BarrageEffectsModule` e e replicado para os clientes.

## Importacao no Workbench

Os prefabs, WAV e projetos `.acp` ja foram indexados, o `resourceDatabase.rdb` foi atualizado e os scripts passaram pela validacao do Workbench. Nao e necessario montar manualmente os grafos no Audio Editor.

Se o projeto ja estava aberto antes destas alteracoes, use **Script Editor > Compile and Reload Scripts** ou feche e reabra o Workbench para recarregar os recursos na sessao atual. Depois, faca um teste em jogo para confirmar a cadencia e o balanco com o restante da mixagem.

Para multiplayer, habilite o addon no servidor e nos clientes. O prefab e o script precisam estar presentes nos dois lados.
