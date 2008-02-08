#!/usr/bin/env tclsh
# Portuguese (Português) translations for PureData
# $Id: portugues.tcl,v 1.1.2.5 2006-10-13 16:00:56 matju Exp $
# by Nuno Godinho

# (waiting for a version that has 8859-1 accents)

### Menus

say file "Ficheiro"
  say new_file "Novo Ficheiro"
  say open_file "Abrir Ficheiro..."
  say pdrc_editor "Editor .pdrc"
  say send_message "Enviar Mensagem..."
  say paths "Caminhos..."
  say close "Fechar"
  say save "Gravar"
  say save_as "Gravar Como..."
  say print "Imprimir..."
  say quit "Sair"

say edit "Editar"
  say undo "Desfazer"
  say redo "Refazer"
  say cut "Cortar"
  say copy "Copiar"
  say paste "Colar"
  say duplicate "Duplicar"
  say select_all "Seleccionar Tudo"
  say text_editor "Editor de Texto..."
  say tidy_up "Arranjar"
  say edit_mode "Modo Editar"

say view "Vista"
  say reload "Recarregar"
  say redraw "Redesenhar"

say find "Procurar"
  say find_again "Procurar Novamente"
  say find_last_error "Encontrar Ultimo Erro"

say put "Colocar"

say media "Media"
  say audio_on "Audio ON"
  say audio_off "Audio OFF"
  say test_audio_and_midi "Testar Audio e MIDI"
  say load_meter "Medidor de Carga"

say window "Janela"

say help "Ajuda"
  say about "Acerca..."
  say pure_documentation "Documentação do Pure..."
  say class_browser "Listar Classes..."


### Main Window

say in "entrada"
say out "saida"
say audio "Audio"
say meters "Medidores"
say io_errors "Erros de IO"

### Other

say cannot "impossivel"

### phase 4

say section_audio "Áudio"
  say -r "frequência de amostragem"
  say -audioindev "dispositivos de entradaa áudio"
  say -audiooutdev "dispositivos de saída áudio"
  say -inchannels "canais de entrada áudio (por dispositivo, como \"2\" ou \"16,8\")"
  say -outchannels "número de canais de saída áudio (igual)"
  say -audiobuf "especificar tamanho do buffer de áudio em ms"
  say -blocksize "especificar tamanho do bloco I/O áudio em número de amostras"
  say -sleepgrain "especificar número de milisegundos que dorme quando inactivo"
  say -nodac "inibir saída de áudio"
  say -noadc "inibir entrada de áudio"
  say audio_api_choice "Áudio API"
  say default "defeito"
    say -alsa "usar ALSA audio API"
    say -jack "usar JACK audio API"
    say -mmio "usar MMIO audio API (por defeito para Windows)"
    say -portaudio "usar ASIO audio driver (via Portaudio)"
    say -oss "usar OSS audio API"
  say -32bit "permitir OSS áudio a 32 bit (para RME Hammerfall)"

say section_midi "MIDI"
  say -nomidiin "inibir entrada MIDI"
  say -nomidiout "inibir saída MIDI"
  say -midiindev  "lista de dispositivos de entrada midi; ex., \"1,3\" para primeiro e terceiro"
  say -midioutdev "lista de dispositivos de saída midi, mesmo formato"

say section_externals "Externals"
  say -path     "adicionar a caminho de pesquisa de ficheiros"
  say -helppath "adicionar a caminho de pesquisa de ficheiros de ajuda"
  say -lib      "carregar biblioteca(s) de objectos"

say section_gui "Gooey"
  say -nogui "inibir inicialização de GUI (cuidado)"
  say -guicmd "substituir programa de GUI (ex., rsh)"
  say -console "linhas armazenadas na consola (0 = inibir consola)"
  say -look "pasta contendo icons para barra de botões"
  say -statusbar "inibir barra de status"
  say -font "especificar tamanho da fonte por defeito em pontos"

say section_other "Outros"
  say -open "abrir ficheiro(s) na inicialização"
  say -verbose "impressões extra na inicialização e durante pesquisa de ficheiros"
  say -d "nível de depuração"
  say -noloadbang "inibir efeito de \[loadbang\]"
  say -send "enviar mensagem na inicialização (depois dos patches carregados)"
  say -listdev "listar dispositivos áudio e MIDI após inicialização"
  say -realtime "usar prioridade de tempo-real (necessários privilégios de root)"

