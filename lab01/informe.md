


# <center>Sistemas Operativos</center>

## Laboratorio 1 : MyBash

**Programando nuestro propio shell de Linux**







Desarrollado por:

+ Salas, Pedro
+ Hubmann, Tomás
+ Strasorier, Marcos
+ Cuevas, Ignacio









Profesores a cargo:

+ Facundo Bustos
+ Luigi Finetti




**Universidad Nacional de Córdoba, FaMAF**


Martes, 13 de septiembre de 2022
<br></br>
<br></br>

## Introducción

MyBash es un proyecto presentado por la cátedra con la finalidad de codificar una shell al estilo Bash.
Para desarrollarla tuvimos un período de tres semanas en las cuales tuvimos que investigar, planificar e implementar cada uno de los módulos necesarios para la funcionalidad de la misma.
<br></br>

## Modularización

## command.h

Para empezar este módulo, primero nos centramos en aprender a usar las funciones de GList (librería que elegimos para el manejo de listas), para luego empezar a completar el TAD. En realidad, intentamos usar las funciones de esta librería en la mayoría del cuerpo de este módulo, pero generaba ciertos problemas, más específicamente, en el momento de liberar la memoria. A medida que fuimos avanzando y realizando tests, en vez de usar las funciones de GList que supuestamente liberan la memoria de los elementos, descubrimos que no hacían bien su trabajo, y generaban memory leaks. Al final, decidimos que era más conveniente, usar free() y scommand_destroy, y liberar uno por uno los elementos del arreglo que se utilizaban en cada función del TAD.
Otra cosa necesaria para la implementación de las funciones del TAD (principalmente las funciones -_to_string), fue la función strmerge. Ésta estaba incluida en el archivo strextra dado por la cátedra, que generaba un nuevo string. Esto hizo que tengamos que tener cuidado con no liberar los strings que dejamos de usar.
Fuera de las problemáticas anteriormente mencionadas, no hubo más complicaciones en este módulo. 

 

## parser.c
 
Este módulo fue brindado por la cátedra con la finalidad de implementar el módulo parsing.c. Este se centraba en tomar lo ingresado por el usuario en la terminal y crear una instancia de parser, además de brindar funciones que nos permiten manipular los elementos de este tipo y usarlo como intermediario para generar elementos del tipo pipeline que luego serán empleados en el módulo de execute.


## parsing.c
 
La funcionalidad de este módulo es la de tomar una instancia del tipo parser y convertirla en algo del tipo pipeline, evitando los problemas generados por el exceso de espacios y detectar comandos inválidos (debido a la presencia de && o la entrada de comandos vacíos) en una etapa temprana y así evitar problemas con las funciones del módulo execute.
Para cumplir esto se usan dos funciones centrales el parse_scommand que después de verificar que el parse no está cerrado crea una instancia de scommand en la que se irán agregando los argumentos al campo correspondiente hasta encontrarse con una instancia de “\n”, “&” o “|” y luego volver a la función parse_pipeline. En esta función se encarga de llamar parse_scommand cuando es necesario para rellenar el pipe con los scommand generados por esta, además de destruir el pipe en caso de que se ingrese un comando inválido y siempre mientras hace todo esto consumir la totalidad del parser.
 

## strextra.c
 
Este es otro módulo que nos fue dado por la cátedra el cual posee únicamente la función strmerge que toma dos strings y crea uno nuevo que es la mezcla de los dos sin modificar los strings ingresados, esta función fue usada principalmente en el modulo command para implementar las funciones scommand_to_string y pipeline_to_string.
 

## builtin.c

En este módulo se implementaron tres funciones, “builtin_is_internal”, “builtin_alone” y “builtin_run”.
Las dos primeras funciones fueron casi triviales de implementar ya que solo tuvimos que hacer algunas comparaciones. En cambio, la última función fue más difícil de implementar. Para hacerlo, lo primero que hicimos fue ver qué comando se quería ejecutar comparando con la función strcmp().
Para implementar cd dividimos en otros tres casos: cuando tiene un argumento, cuando tiene más de uno (error) y cuando no tiene (HOME).
La implementación de help la hicimos solo con un print en pantalla de lo solicitado en el enunciado.
Por último, para implementar exit simplemente cerramos la entrada estándar (STDIN_FILENO).


## execute.c

Este módulo atacó el problema de lo que hay que hacer entre fork() y execvp(). Primero comenzamos haciendo casos simples, la ejecución de comandos internos y comandos simples.
Luego probamos hacer un caso más complejo probando con la ejecución de dos comandos simples separados con un pipe, pero se nos ocurrió que podíamos hacer multiples pipes utilizando recursión. Para ello, empezamos con el caso base, que es ejecutar un comando simple, y luego seguimos con el caso recursivo donde allí se crearía el pipe y se llamaría a si misma. La función pide como argumento un read, que es la punta de lectura del pipe anterior, por eso la llamada principal se pone STDIN_FILENO. En cada llamada se cierra la punta de escritura y en el caso base se cierra la finalmente la punta de lectura.
 

## mybash.c

La idea principal de éste módulo, fue básicamente la siguiente. El usuario ingresa una cadena de string por la entrada estándar. Una vez que es proporcionada, se ejecuta un ciclo infinito que muestra el “prompt” hasta que se cierre la entrada estándar con ctrl+d, o se escriba el comando exit. Mientras tanto el usuario puede escribir por stdin, una cadena de string que será convertido en un pipeline. Si la cadena ingresada es distinta de NULL, y es válida, se corre el comando correspondiente. En caso contrario, se emite un mensaje de error por stdout. 
Además decidimos modificar el show_prompt para poder mostrar el username y el hostname seguido del relative path, como lo hace la Bash de Linux. Para hacerlo existen ciertas funciones en unistd.h que nos permiten obtener esta información . Para el caso del path, como la función getcwd nos devuelve el absolute path, tuvimos que crear una función que devuelve el PATH relativo al HOME.El detalle de los colores se implementó declarando una constante que contenía el color correspondiente e ingresando dicho valor como primer argumento de los printf correspondientes. 

## Técnicas de programación


**Libraries**

La biblioteca principal del código fue GLib, en particular el TAD GList. Utilizamos el tipo lista para construir el TAD scommand y de pipeline. Aunque funciona muy bien, creemos que nos ha generado ciertos inconvenientes con el manejo de memoria.
Además de GLib utilizamos librerías comunes como, stdio.h, stdlib.h, assert.h, stdbool.h, string.h. Y otras menos comunes como:
* **unistd.h**: para importar las llamadas al sistema.
* **fcntl.h**: para importar funciones relacionadas al manejo de archivos.
* **sys/wait.h**: para importar la llamada al sistema wait.


**Recursión**

Para la implementación del multipipe tras probar distintos métodos terminamos usando la idea de crear una función recursiva que recibe un pipe sin comandos internos y un fd del cual proviene la entrada (STDIN_FILENO en caso de ser la primera llamada). Se considera como caso base el pipe de largo 1 y cualquier largo mayor como el caso recursivo.


**Estilo de código**

Para el código, intentamos mantener el mismo estilo para todos los módulos. Nombrando todas las variables/funciones en un mismo idioma (Inglés, ya que es lo más común). Además comentamos grandes partes del código para que se entienda qué y por qué se hace cada cosa.
En cuestiones de formato, tiene un formato muy estándar, indentaciones correctas y evitando espacios innecesarios.


**Manejo de memoria dinámica**

En el manejo de memoria dinámica, nos aseguramos de que cada memoria solicitada sea liberada.
La única memoria que no pudimos liberar se debe al uso de la librería GLib que nos ha generado varios problemas a lo largo del proyecto.


**Programación defensiva**

En cuanto a la programación defensiva, intentamos cubrir la mayor cantidad de casos bordes posibles y que el programa sea lo más robusto posible. Sobre todo en el módulo de execute.c ya que allí está la mayoría de las llamadas al sistema. 


**Algoritmos destacados**

En el módulo de command, hay dos algoritmos destacados (los más difíciles de implementar), scommand_to_string y pipeline_to_string. Principalmente la problemática de estos es la de evitar la pérdida de memoria.
El módulo execute presenta un algoritmo a destacar que es la función execute_pipeline_rec que es el punto central del funcionamiento de este módulo cuyo rol ha sido mencionado anteriormente.

**Herramientas de programación**

Para todo el proyecto fue vital el uso del Make para la agilidad de compilación.
También usamos herramientas como Valgrind y GDB para el debugging de cada módulo individualmente. Luego fuimos usando los tests que se proveyeron por parte de la cátedra.

**Desarrollo**

El desarrollo estuvo fragmentado en dos secciones grandes: 
(1) la implementación de TADs
(2) el desarrollo de los módulos que dependen de los TADs.

Durante el primer momento de desarrollo, el grupo tuvo bastantes problemas con GLib y su uso en el módulo command.c. Luego, unos tutoriales más tarde, el problema pasó a ser el manejo de memoria dinámica con GLib.

Luego llegamos a una etapa de paralelización en el que nos pudimos poner a completar el resto de módulos individualmente. El problema entonces pasó a ser la división de tareas y recursos y la integración de cada parte del desarrollo. Como fuimos rotando y avanzando de una forma pareja, pudimos llegar a integrar bien todas las partes.
<br></br>
<br></br>

## Conclusiones

A lo largo del desarrollo de este proyecto y tras un estudio del funcionamiento tanto de las diferentes syscalls y las diferentes librerías necesarias para implementar los distintos módulos mencionados con anterioridad. Hemos sido capaces de crear un shell que cumple  las condiciones solicitadas en el enunciado, además de incluir la capacidad de soportar pipes de un largo arbitrario y que se muestre la ubicación actual (junto al username y al hostname) al momento de ingresar un comando con los colores base de bash.
Algunos problemas que encontramos durante el desarrollo fueron los siguientes: 
+ Manejo de memoria con la librería de GLib, lo que nos dejó desconfiando de la fiabilidad de recursos implementados por terceros.
+ Problemas con la implementación de un multipipe, la prueba y * error hasta dar con la solución más limpia (recursión).
+ Bugs escurridizos, que se nos complicó encontrar incluso con el uso de gdb.


## Información general


**<h3>Laboratorio 1 : MyBash</h3>**

**Integrantes del grupo:**

* Salas, Pedro: pedrosalaspinero@mi.unc.edu.ar
* Hubmann, Tomás: tomas.hubmann@mi.unc.edu.ar
* Strasorier, Marcos: marcos.strasorier@mi.unc.edu.ar
* Cuevas, Ignacio: icuevas@mi.unc.edu.ar