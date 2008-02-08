#!/usr/bin/env tclsh
# Español translations for PureData
# $Id: espanol.tcl,v 1.1.2.3.2.1 2006-12-05 04:51:47 matju Exp $
# translated by Mario Mora (makro) & Ramiro Cosentino (rama)

### Menus

say file "Archivo"
  say new_file "Nuevo Archivo"
  say open_file "Abrir Archivo..."
  say pdrc_editor "Editor .pdrc"
  say send_message "Enviar Mensaje..."
  say paths "Rutas..."
  say close "Cerrar"
  say save "Guardar"
  say save_as "Guardar Como..."
  say print "Imprimir..."
  say quit "Salir"
  
  say canvasnew_file "Archivo Nuevo"
  say canvasopen_file "Abrir Archivo..."
  say canvassave "Guardar"
  say canvassave_as "Guardar como..."
  say clientpdrc_editor "Editor .pdrc "
  say clientddrc_editor "Editor .ddrc "
  say canvasclose "Cerrar"
  say canvasquit "Salir"

say edit "Edición"
  say undo "Deshacer"
  say redo "Rehacer"
  say cut "Cortar"
  say copy "Copiar"
  say paste "Pegar"
  say duplicate "Duplicar"
  say select_all "Seleccionar Todo"
  say text_editor "Editor de Texto..."
  say tidy_up "Ordenar"
  say edit_mode "Modo Edición"
  say editmodeswitch "Modo edicion/ejecucion"
  
  say canvascut "Cortar"
  say canvascopy "Copiar"
  say canvasundo "Deshacer"
  say canvasredo "Rehacer"
  say canvaspaste "Pegar"
  say canvasselect_all "Seleccionar todo"
  say canvaseditmodeswitch "Modo edicion/ejecucion"

say view "Ver"
  say reload "Recargar"
  say redraw "Refrescar"
  
  say canvasreload "Recargar"
  say canvasredraw "Refrescar"

say find "Buscar"
  say find_again "Buscar Nuevamente"
  say find_last_error "Buscar Ultimo Error"
  
say canvasfind "Buscar"
  say canvasfind_again "Buscar Nuevamente"

# contents of Put menu is Phase 5C
say put "Poner"
 say Object "Objeto"
  say Message "Mensaje"
  say Number "Numero"
  say Symbol "Simbolo"
  say Comment "Comentario"
  say Canvas "Canvas";#
  say Array "Deposito";#array as "desposito"?
  
  say canvasobject "Objeto"
  say canvasmessage "Mensaje"
  say canvasnumber "Numero"
  say canvassymbol "Simbolo"
  say canvascomment "Comentario"
  say canvasbang "Bang";#
  say canvastoggle "Interruptor";# toggle as "interruptor"?
  say canvasnumber2 "Numero2"
  say canvasvslider "Deslizador Vertical";#slider as "deslizador"??
  say canvashslider "Deslizador Horizontal"
  say canvasvradio "Rango Vertical";#radio  as "Rango"??
  say canvashradio "Rango Horizontal";#
  say canvascanvas "Canvas";#
  say canvasarray "Array";#

say media "Media"
  say audio_on "Audio ON"
  say audio_off "Audio OFF"
  say test_audio_and_midi "Probar Audio y MIDI"
  say load_meter "Medidor de Carga"
  
  say canvasaudio_on "Audio ON"
  say canvasaudio_off "Audio OFF"
  say clienttest_audio_and_midi "Probar Audio y MIDI"
  say canvasload_meter "Medidor de Carga"

say window "Ventana"

say help "Ayuda"
  say about "Acerca de..."
  say pure_documentation "Pura Documentacion..."
  say class_browser "Navegador de Clases..."
  say canvasabout "Acerca de..."

say properties "Propiedades"
say open "Abrir"

### for key binding editor
say general "General"
say audio_settings "Configuracion Audio "
say midi_settings "Configuracion Midi "
say latency_meter "Medidor de Latencia"
say Pdwindow "Ventana Pd "

say canvaspdwindow "Ventana Pd"
say canvaslatency_meter "Medidor de Latencia"
say clientaudio_settings "Configuracion Audio"
say clientmidi_settings "Configuracion Midi"

### for Properties Dialog (phase 5B)
say_category IEM
say w "ancho(px)"
say h "alto(px)"
say hold "tiempo de mantencion(ms)"
say break "tiempo de quiebre(ms)"
say min "valor minimo"
say max "valor maximo"
say is_log "modo"
say linear "linear"
say logarithmic "logaritmico"
say isa "inicio"
say n "numero de posibilidades"
say steady "regularidad"
say steady_no  "saltar en click";#
say steady_yes "estabilizar en click";#
say snd "enviar-simbolo"
say rcv "recibir-simbolo"
say lab "etiqueta"
say ldx "etiqueta compensacion x";#
say ldy "etiqueta compensacion y";#
say fstyle "Tipografia"
say fs "Tamaño de fuente"
say bcol "Color Fondo"
say fcol "color primer plano"
say lcol "color etiqueta"
say yes "si"
say no "no"
say courier "courier (typewriter)"
say helvetica "helvetica (sansserif)"
say times "times (serif)"
say coords "grafico en pariente" ;# parent?? as "pariente"??

say_category GAtomProperties
say width "ancho"
say lo "limite bajo"
say hi "limite alto"
say label "etiquetal"
say wherelabel "mostrar etiqueta on"
say symto "enviar simbolo"
say symfrom "recibir simbolo"

say_category GraphProperties
say x1   "desde x"
say x2   "hacia x"
say xpix "ancho pantalla"
say y2   "desde y"
say y1   "hacia y"
say ypix "altura pantalla"

say_category CanvasProperties
#say xscale "X units/px"
#say yscale "Y units/px"

say_category ArrayProperties
say name "nombre"
say n    "Tamaño"

### Main Window
say_category MainWindow
say in "in"
say out "out"
say audio "Audio"
say meters "Medidores"
say io_errors "Errores de E/S"

### phase 2

say_category Other
say_namespace summary {
  say_category IEMGUI
  say bng    "Bang Box";# Caja de bang?
# say Bang "Bang"
# say Toggle "Interruptor";# toggle as "interruptor"?
  say tgl    "Toggle Box";#Caja palanca? caja interruptora?
  say nbx    "Number Box (IEM)";#Caja de numeros?
# say Number2 "Numero2"
  say hsl    "deslizador (Horizontal)";#
  say vsl    "Deslizador (Vertical)";#
  say hradio "Choice Box (Horizontal)";# Caja seleccionadora?
  say vradio "Choice Box (Vertical)";#
# say Vradio "Rango Vertical";#radio as "rango"?
# say Hradio "Rango Horizontal";#
  say cnv    "Canvas (IEM)"
  say dropper "Drag-and-Drop Box";#Caja agarrar y soltar?
  say vu     "Vumeter";# medidor VU?

  say_category GLUE
  say bang "envia un mensaje de bang"
  say float "guarda y recuerda un numero"
  say symbol "guarda y recuerda un simbolo"
  say int "guarda y recuerda un entero"
  say send "envia un mensaje a un objeto nombrado"
  say receive "catch sent messages"
  say select "test para numeros o simbolos coincidentes"
  say route "rutea mensajes de acuerdo al primer elemento"
  say pack "genera mensajes compuestos"
  say unpack "obtiene elementos de mensajes compuestos"
  say trigger "ordena y convierte mensajes"
  say spigot "conexion mensajes interrumpible"
  say moses "divide un flujo de numeros"
  say until "mecanismo de loop"
  say print "imprime mensajes"
  say makefilename "formatea un simbolo con un campo variable";#
  say change "remover numeros repetidos de un flujo de datos";#
  say swap "intercambiar dos numeros";#
  say value "compartir valor numerico"

  say_category TIME
  say delay "envia un mensaje despues de un retraso de tiempo"
  say metro "envia un mensaje periodicamente"
  say line "envia una serie de numeros encaminados linearmente"
  say timer "medicion de intervalos de tiempo"
  say cputime "medicion tiempo CPU"
  say realtime "medicion tiempo real"
  say pipe "linea de retraso dinamicamente creciente para numeros"

  say_category MATH
  say + "sumar"
  say - "sustraer"
  say * "multiplicar"
  say {/ div} "dividir"
  say {% mod} "resto de la division"
  say pow "exponencial"
  say == "igual?"
  say != "no igual?"
  say > "mas que?"
  say < "menos que?"
  say >= "no menos que?"
  say <= "no mas que?"
  say &  "bitwise conjunction (and)";#bitwise? conjuncion (y)
  say |  "bitwise disjunction (or)";#bitwise? conjuncion (o)
  say && "conjuncion logica (y)"
  say || "separacion logica (o)"
  say mtof "MIDI a Hertz"
  say ftom "Hertz a MIDI"
  say powtodb "Watts a dB"
  say dbtopow "dB a Watts"
  say rmstodb "Volts a dB"
  say dbtorms "dB a Volts"
  say {sin cos tan atan atan2 sqrt} "trigonometria"
  say log "logaritmo Euler"
  say exp "exponencial Euler"
  say abs "valor absoluto"
  say random "aleatorio"
  say max "mayor de dos numeros"
  say min "menor de dos numeros"
  say clip "fuerza un numero en un rango"

  say_category MIDI
  say {notein ctlin pgmin bendin touchin polytouchin midiin sysexin} "entrada MIDI"
  say {noteout ctlout pgmout bendout touchout polytouchout midiout}  "salida MIDI"
  say makenote "programa un retrasado \"note off\" mensaje correspondiente a un note-on";#
  say stripnote "tira \"note off\" mensajes";#

  say_category TABLES
  say tabread "lee un numero desde una tabla"
  say tabread4 "lee un numero desde una tabla, con 4 puntos de interpolacion"
  say tabwrite "escribe un numero a una tabla"
  say soundfiler "lee y escribe tablas a archivos de audio"

  say_category MISC
  say loadbang "bang en carga"
  say serial "control recurso serial for NT only"
  say netsend "envia mensajes sobre internet"
  say netreceive "recibirlos"
  say qlist "secuencia mensajes"
  say textfile "convertidor de archivo a mensaje"
  say openpanel "\"Abrir\" dialogo"
  say savepanel "\"Guardar como\" dialogo"
  say bag "sistema de numeros"
  say poly "asignacion de voz polifonica"
  say {key keyup} "valores numericos de teclas del teclado"
  say keyname "simbolo de la tecla";#

  say_category "AUDIO MATH"
  foreach word {+ - * /} {say $word~ "[say $word] (for signals)"};#this has to be translated too?
  say max~ "supremo de señales"
  say min~  "infimo de señales"
  say clip~ "fuerza la señal a permanecer entre dos limites"
  say q8_rsqrt~ "raiz cuadrada reciproca economica (cuidado -- 8 bits!)";#
  say q8_sqrt~ "raiz cuadrada economica (cuidado -- 8 bits!)";#
  say wrap~ "abrigo alrededor (parte fraccional, tipo de)";#wrap? as abrigo?? around as "alrededor"??
  say fft~ "transformada fourier discreta compleja delantero";#
  say ifft~ "transformada fourier discreta compleja inversa";#
  say rfft~ "transformada fourier discreta real delantero";#
  say rifft~ "transformada fourier discreta real delantero";#
  say framp~ "arroja una rampa para cada bloque"
  foreach word {mtof ftom rmstodb dbtorms rmstopow powtorms} {
    say $word~ "[say $word] (para señales)";#
  }
}

### phase 3

say_namespace summary {
  say_category "UTILIDADES AUDIO"
  say dac~ "salida audio"
  say adc~ "entrada audio"
  say sig~ "convierte numeros a señales de audio"
  say line~ "genera rampas de audio"
  say vline~ "line~ delujo"
  say threshold~  "detectar umbrales de la señal"
  say snapshot~ "toma muestras de una señal (reconvierte a numeros)"
  say vsnapshot~ "snapshot~ delujo";#
  say bang~ "envia un mensaje bang despues de cada block DSP"
  say samplerate~ "obtener rango de muestreo"
  say send~ "conexion señal no local con fanout";# fanout??
  say receive~ "obtener señal desde send~"
  say throw~ "agrega a un bus sumador";#
  say catch~ "define y lee bus sumador";#
  say block~ "especifica tamaño del bloque y overlap"
  say switch~ "selecciona computacion DSP  on y off";#
  say readsf~ "reproduce archivo de audio desde disco"
  say writesf~ "graba sonido a disco"

  say_category "OSCILADORES DE AUDIO Y TABLAS"
  say phasor~ "oscilador diente de sierra"
  say {cos~ osc~} "oscilador coseno"
  say tabwrite~ "escribe a una tabla"
  say tabplay~ "reproduce desde una tabla (sin transponer)"
  say tabread~ "lee tabla sin interpolacion"
  say tabread4~ "lee tabla con 4 puntos de interpolacion"
  say tabosc4~ "oscilador de tabla de ondas"
  say tabsend~ "escribe un bloque continuamente a una tabla"
  say tabreceive~ "lee un bloque continuamente desde una tabla"

  say_category "FILTROS AUDIO"
  say vcf~ "filtro controlado por voltaje"
  say noise~ "generador ruido blanco"
  say env~ "lector envolvente"
  say hip~ "filtro pasa agudos"
  say lop~ "filtro pasa bajos"
  say bp~ "filtro pasa banda"
  say biquad~ "filtro crudo"
  say samphold~ "unidad muestra y mantener";# sample as "muestra"
  say print~ "imprimir uno o mas \"bloques\""
  say rpole~ "filtro crudo valor real de un polo"
  say rzero~ "fitro crudo valor real de un cero"
  say rzero_rev~ "[say rzero~] (tiempo-invertido)"
  say cpole~ "[say rpole~] (complejo-valorado)"
  say czero~ "[say rzero~] (complejo-valorado)"
  say czero_rev~ "[say rzero_rev~] (complejo-valorado)"

  say_category "AUDIO DELAY"
  say delwrite~ "escribir a una linea de retraso"
  say delread~ "leer desde una linea de retraso"
  say vd~ "leer desde una linea de retraso a un tiempo de retraso variable"

  say_category "SUBVENTANAS"
  say pd "define una subventana"
  say table "arsenal de numeros en una subventana"
  say inlet "agrega una entrada a pd"
  say outlet "agrega una salida a pd"
  say  inlet~  "[say entrada] (para señal)";#
  say outlet~ "[say salida] (para señal)";#

  say_category "PLANTILLAS DE DATOS"
  say struct "define una estructura de datos"
  say {drawcurve filledcurve} "dibuja una curva"
  say {drawpolygon filledpolygon} "dibuja un poligono"
  say plot "trace un campo del arsenal";#
  say drawnumber "imprime un valor numerico"

  say_category "ACCEDIENDO A DATOS"
  say pointer "señale a un objeto que pertenece a una plantilla"
  say get "obtener campos numericos"
  say set "cambiar campos numericos"
  say element "obtener un elemento del deposito";#array as "deposito" 
  say getsize "obtener el tamaño del deposito"
  say setsize "cambiar el tamaño del deposito"
  say append "agregar un elemento a una lista"
  say sublist "obtenga un indicador en una lista que sea un elemento de otro escalar";# scalar as escalar?? some maths expert should give his opinion
  say scalar "dibuje un escalar en pariente";#same
  
  say_category "OBSOLETE"
  say scope~ "(use tabwrite~ ahora)"
  say namecanvas "" ;# what was this anyway?
  say template "(use struct ahora)"
}

# phase 4 (pdrc & ddrc)

say section_audio "Audio"
  say -r "rango de muestreo"
  say -audioindev "dispositivos entrada de audio"
  say -audiooutdev "dispositivos salida de audio"
  say -inchannels "canales entrada audio (por dispositivo, como \"2\" or \"16,8\")"
  say -outchannels "numero de canales salida audio (igual)"
  say -audiobuf "especificar tamaño de almacenador de audio en mseg"
  say -blocksize "especificar tamaño block E/S audio  en muestras por cuadro"
  say -sleepgrain "especificar numero de milisegundos para suspension cuando este inactivo";#
  say -nodac "suprimir salida de audio"
  say -noadc "suprimir entrada de audio"
  say audio_api_choice "Audio API"
    say default "defecto"
    say -alsa "usar ALSA audio API"
    say -jack "usar JACK audio API"
    say -mmio "usar MMIO audio API (por defecto para Windows)"
    say -portaudio "usar ASIO audio driver (via Portaudio)"
    say -oss "usar OSS audio API"
  say -32bit "permitir 32 bit OSS audio (para RME Hammerfall)"
  say {} "defecto"

say section_midi "MIDI"
  say -nomidiin "suprime entrada MIDI"
  say -nomidiout "suprime salida MIDI"
  say -midiindev  "lista dispositivos midi in; e.g., \"1,3\" para primero y tercero"
  say -midioutdev "lista dispositivos midi out, mismo formato"

say section_externals "Externals"
  say -path     "ruta busqueda de archivo"
  say -helppath "ruta busqueda archivo de ayuda"
  say -lib      "cargar librerias de objeto"

say section_gui "Gooey";# what??
  say -nogui "suprime inicio de gui (precaucion)"
  say -guicmd "substituye por otro programa GUI (e.g., rsh)"
  say -console "lineas retroceso consola  (0 = desactivar consola)"
  say -look "iconos barra de botones"
  say -statusbar "activar barra de status"
  say -font "especificar tamaño de fuente por defecto en puntos"

say section_other "Otro"
  say -open "abrir archivo(s) en inicio"
  say -verbose "impresion extra en inicio y al buscar archivos";#
  say -d "nivel chequeo errores";#
  say -noloadbang "desactivar el efecto de \[loadbang\]"
  say -send "enviar un mensaje al inicio (despues patches estan cargados)"
  say -listdev "lista dispositivos audio y MIDI al inicio"
  say -realtime "usar prioridad tiempo-real  (necesita privilegios de administrador)"

say section_paths "Rutas"

# ddrc

say section_color "colores"
 say canvas_color "canvas";#
  say canvasbgedit "canvas fondo (modo edicion)"
  say canvasbgrun "canvas fondo (modo ejecucion)"
 say object_color "objeto"
  say viewframe1 "color objeto";# objetbox as "objeto"
  say viewframe2 "color objeto";#
  say viewframe3 "color objeto";#
  say viewframe4 "color resaltado  objeto";#
  say viewbg "objeto fondo"
  say viewfg "objeto primer plano"
  say commentbg "comentario fondo"
  say commentfg "comentario primer plano"
  say commentframe1 "comentario cuadro";#frame as "cuadro"
  say commentframe2 "comentario cuadro"
  say commentframe3 "comentario cuadro"
  say viewselectframe "caja resaltada";#
 say wire_color "cable";# wire as "cable"
  say wirefg "color cable"
  say wirefg2 "cable resaltado"
  say wiredash "cable rociado"
 say others_color "otros"
  say inletfg "color entrada"
  say outletfg "color salida"
  say selrect "caja de seleccion";# selection box
say keys "teclas"
say others "otros"
say canvashairstate "Activar malla";# crosshair como "malla"
say canvashairsnap "Malla ajustada a objeto"
say canvasstatusbar "Activar barra de estatus"
say canvasbuttonbar "Activar barra de botones"


# phase 5A

say cannot "no puedo"
say cancel "Cancelar"
say apply  "Aplicar"
say ok     "OK"
say popup_open "Abrir"
say popup_properties "Propiedades"
say popup_help "Ayuda"
say filter "Filtro: "
say how_many_object_classes "%d of %d object classes";# this has to be translated?
say do_what_i_mean "haga lo que quiero decir"
say save_changes? "Guardar cambios?"
say reset "Reiniciar"
say Courier "Courier (monospaced)"
say Helvetica "Helvetica (sansserif)"
say Times "Times (serif)"
say add "Agregar"
say up "Arriba"
say down "Abajo"
say remove "Remover"
say lib_add    "agrega el nombre tipeado en la lista"
say lib_up     "intercambiar orden con libreria previa"
say lib_down   "intercambiar orden con libreria proxima"
say lib_remove "remover libreria seleccionada en la lista"
say dir_add    "agregar una carpeta usando un cuadro de dialogo"
say dir_up     "intercambiar orden con carpeta previa"
say dir_down   "intercambiar orden con carpeta proxima"
say dir_remove "remover carpeta seleccionada en la lista"


### Other



