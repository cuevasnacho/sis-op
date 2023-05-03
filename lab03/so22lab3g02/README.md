# **Sistemas Operativos**

## **Laboratorio 3 : Planificacion**

Desarrollado por:

+ Salas, Pedro
+ Hubmann, Tomas
+ Strasorier, Marcos
+ Cuevas, Ignacio

Profesores a cargo:
+ Facundo Bustos
+ Luigi Finetti

**Universidad Nacional de Cordoba, FaMAF**

---
\
# **Parte 1: análisis general del codigo**

Después de analizar el código de xv6 sacamos las siguientes conclusiones:

### **1. ¿Qué política de planificación utiliza xv6 para elegir el próximo proceso a ejecutarse?**

xv6 utiliza Round Robin como política de planificación. Esto lo deducimos por dos motivos principales:
El context switch se realizaba en dos ocasiones:
+ El primer motivo fue que el proceso produjo una llamada a sleep o se completó el proceso.
+ El segundo motivo fue que el context switch fue realizado por un timer interrupt que sucede cuando se consume el *quantum* que es igual para todos los procesos asignado a ese proceso.

Otro motivo porque lo consideramos Round Robin es que el criterio de selección del scheduler toma directamente el primer proceso en estado *RUNNABLE* y en ese momento inicia el context switch para comenzar a correr el proceso.

\

### **2. Analizar el código que interrumpe a un proceso al final de su quantum y responda:**

+ **¿Cuánto dura un quantum en xv6?**

El tiempo de un quantum es alrededor de una décima de segundo en el formato xv6 base.
\
+ **¿El context switch consume tiempo de un quantum? ¿Cuánto dura un cambio de contexto en xv6?** 

El context switch consume tiempo del quantum ya que antes de siquiera hacerse una timer interrupt se debe llamar a **timervec** que chequea el tiempo transcurrido y setea el momento de la próxima timer interrupt, una vez hecho esto llama a **kerneltrap** que debe revisar el motivo de la interrupción y si es el motivo fue el timer se llamará a **yield** que se encarga de volver al **scheduler** donde se seleccionará el siguiente proceso a correr y recién en ese momento se realizará el guardado de los registros del proceso actual por los del siguiente para finalmente iniciar su ejecución y debido al gran número de operaciones que necesita se puede observar que consume una parte del quantum que en el caso de ser muy pequeño será muy relevante en el tiempo que tarda un proceso hasta completarse. 
Tras hacer las mediciones hicimos un aproximacion de su duracion que es alrededor de **1/900000 segundos si lo redondeamos.**
\
+ **¿Hay alguna forma de que a un proceso se le asigne menos tiempo?**

No hay una manera directa de como modificarlo sin cambiar varias partes de xv6 para que tenga múltiples valores de quantum y en función de que proceso esta corriendo modificar el momento de la próxima interrupción en consecuencia. 
Una cosa que se podría hacer sin cambiar tanto es contar el número de quantums que estuvo corriendo un proceso haciendo que se vuelva a correr hasta que llegue hasta cierto número, pero esto no decrementa el quantum sino que lo hace más largo para ciertos procesos aunque generaría el mismo resultado si achicamos el quantum.  

##### *Aclaraciones*

+ El quantum como está definido en el libro, lo encontramos en el archivo kernel/start.c en la función timerinit() como intervalo.
+ RUNNABLE es el estado de un proceso, equivalente al estado READY del libro.


---
\
# **Parte 2: análisis de medidas en Round Robin** 

### Escenarios

+ Escenario 0: quantum por defecto
+ Escenario 1: quantum 10 veces más corto
+ Escenario 2: quantum 100 veces más corto
+ Escenario 3: quantum 1000 veces más corto

### Casos

+ Caso 0: 1 iobench solo
+ Caso 1: 1 cpubench solo
+ Caso 2: 1 iobench con 1 cpubench
+ Caso 3: 1 iobench con 1 iobench
+ Caso 4: 1 cpubench con 1 cpubench

---

A lo largo del siguiente texto se usará la siguiente notacion:

E0 = escenario 0
E1 = escenario 1
E2 = escenario 2
E3 = escenario 3

C0 = Caso 0
C1 = Caso 1
C2 = Caso 2
C3 = Caso 3
C4 = Caso 4

---

### Mediciones con Round Robin

Apariciones por pantalla con Round Robin

| RR            | E0    | E1    | E2    | E3    |
|----           |----   |----   |----   |----   |
| C0            | 89    | 880   | 8334  | 2771  |
| C1            | 74    | 293   | 250   | 43    |
| C2 (cpu)      | 46    | 466   | 4236  | 1645  |
| C2 (i/o)      | 74    | 280   | 132   | 21    |
| C3 (promedio) | 87.5  | 850.1 | 4999  | 2895.5|
| C4 (promedio) | 81.5  | 356   | 324.5 | 61    |

**Hip1:** debido al tipo del scheduler antes de generar los resultados     ya suponemos que el número de operaciones realizadas por el ionbench caerá de manera notable al realizar un cpubench a la vez

**Hip2:** Debido a que el context switch consume parte del quantum el número de operaciones en una cantidad x de tiempo serán menores en el tiempo más corto.

En cuanto a la hipótesis 2 el número de operaciones si se vio comprometido ya que a pesar de contar el número de operaciones en un tiempo fijo (10 segundos en nuestro caso) en vez de guiarnos por el número de operaciones que mostraban cada impresión se puede observar que hay una caída entre caso y caso, como se muestra en los gráficos siguientes que ilustran el número de operaciones que se realizó en ese tiempo para cada escenario que se planteado con anterioridad.
Los ejemplos más ilustrativos de esto son el gráfico de C0 y C1. En ambos casos se puede observar que el número de operaciones va decreciendo sin una caída tan abrupta hasta que llegamos al E3 que solo hace un poco más de la mitad de las operaciones que hace en el mismo caso en el E0 en el caso de C0 y en el caso de C1 en el E3 solo hace una séptima parte de las operaciones que  hace en el mismo caso en el E0.
El C3 nos muestra cosas bastante interesantes ya que al mezclar tanto un ionbench como un cpubench se nota bastante las consecuencias que tiene el tener un quantum chico o grande al momento de dar preferencia a un tipo de proceso u otro:
En el E0 se nota una caída abrupta del número de operaciones I/O del ionbench mientras que el número de operaciones realizadas no varió en gran medida, pero a medida que se reduce el quantum el número de operaciones del ionbench se incrementa más que proporcionalmente al número de operaciones que realiza el cpubench como se observa en el caso E1 y E2.
En el caso E3 el quantum es tan pequeño que el número de operaciones cae en ambos procesos por lo que en una prueba práctica nos permitirá asumir que ya es demasiado pequeño.
Este comportamiento puede ser explicado si pensamos en cómo se comportan esos procesos ya que el cpubench consume por completo el quantum al hacer una gran cantidad de operaciones mientras que ionbench al realizar operaciones del tipo input output suele pasar al estado SLEEPING y por lo tanto deberá esperar que el cpubench consuma todo su quantum en cada ocasión.
En el caso de los C3 no hay nada que mencionar más allá de que el número de operaciones entre los 2 ionbench es un poco menor al C0 que solo ejecuta 1 a la vez.
Un caso atípico y que nos sorprendió es el de C4 ya que a pesar de reaccionar de manera similar a su contraparte C2 al modificar el quantum, a pesar de que el cpu se debía dividir entre los 2 procesos la suma de los 2 fue mayor a la generada por un único proceso, la única explicación que se nos ocurrió es que al dividirse en 2 al momento de hacer el print de el número de operaciones realizadas el segundo aprovechaba más esos milisegundos que el cpu estaba desocupado y permite usarlo de manera más eficiente.

# Histogramas

Aclaracion: las medidas de I/O estan dadas en I/O operations por cada 100 ticks de reloj, y las medidas de cpu estan dadas en kflops por cada 100 ticks de reloj.

![histograma](https://i.ibb.co/XLdVSSX/Screenshot-from-2022-10-24-23-50-07.png)

![histograma](https://i.ibb.co/SJjVL6T/Screenshot-from-2022-10-24-23-50-12.png)

![histograma](https://i.ibb.co/0hYv2ZF/Screenshot-from-2022-10-24-23-50-15.png)

---
# **Parte 3: análisis de medidas en MLFQ**

### Mediciones con MLFQ
El objetivo de este tipo de scheduler es darle más prioridad a los procedimientos como iobench que no suelen emplear por completo su quantum.

Apariciones por pantalla con Multi-level Feedback Queue (MLFQ).

| MLFQ          | E0 | E1   | E2    | E3    |
|----           |----|----  |----   |----   |
| C0            | 90 | 869  | 8772  | 4048  |
| C1            | 74 | 249  | 261   | 45    |
| C2 (cpu)      | 46 | 468  | 4478  | 2480  |
| C2 (i/o)      | 76 | 281  | 145   | 16    |
| C3 (promedio) | 88 | 858  | 4999  | 4045.5|
| C4 (promedio) | 81 | 365  | 326   | 25    |

Algunos cambios que se pueden observar al analizar los datos del C0 y el C1 es que el mlfq es mejor en cuanto al número de operaciones realizado por un único ionbench ya que con nuestra implementación tarda un poco menos en volver a seleccionarlo permitiéndole aprovechar un poco más el tiempo, un dato no menor es que esta mejoría se mantiene a lo largo de los distintos escenarios en los que variamos el quantum. 
Por otro lado en el E0 la diferencia parece favorecer un poco más al Round Robin pero a pesar de esto en los siguientes escenarios el costo de reducir el tiempo que tarda en ser seleccionado al haber más momentos que la sobrecarga de elegir el proceso está presente el modelo termina siendo más efectivo.Otro ejemplo que nos permite observar esto es el del C2 en el  E0 parece que el Round Robin es mas efectivo pero al reducir el quantum pasa lo mismo que el caso anterior.

En el resto de los casos se puede observar que el número de operaciones realizadas por el MLFQ y el Round Robin son bastantes similares ya que de cierta manera como todos los procesos tienen la misma prioridad terminan actuando de la misma manera.

Algunas pruebas que hicimos además de las mediciones para comprobar el correcto funcionamiento de la MLFQ es controlar el número de veces que entraba cada proceso al ejecutar un caso con 2 ionbench y un cpubench se noto una diferencia importante en el número de veces que se corrían ya que en el Round Robin se incrementaron en la misma medida mientras que en el del MLFQ se noto una gran preferencia por los ionbench haciendo que el cpubench se ejecutará menos veces en un mismo periodo de tiempo. 
Esta prueba también nos permitió ver un punto negativo de nuestra implementación ya que si hacemos crecer el número de ionbench hasta que el tiempo que tardan en hacer la solicitud de entrada o salida es menor al que tardan en completarse los demas no sera muy dificil que nos encontremos ante casos de starvation.

---

![](https://cdn.discordapp.com/attachments/1034179399005782036/1034300929178095656/unknown.png)

![](https://cdn.discordapp.com/attachments/1034179399005782036/1034300967795040256/unknown.png)

![](https://cdn.discordapp.com/attachments/1034179399005782036/1034449462841573377/unknown.png)

![](https://cdn.discordapp.com/attachments/1034179399005782036/1034301035864404039/unknown.png)

![](https://cdn.discordapp.com/attachments/1034179399005782036/1034301074510716958/unknown.png)

![](https://cdn.discordapp.com/attachments/1034179399005782036/1034301099752042586/unknown.png)

![](https://cdn.discordapp.com/attachments/1034179399005782036/1034301125131767909/unknown.png)

![](https://cdn.discordapp.com/attachments/1034179399005782036/1034301148674408528/unknown.png)

![](https://cdn.discordapp.com/attachments/1034179399005782036/1034301238919041034/unknown.png)

![](https://cdn.discordapp.com/attachments/1034179399005782036/1034301255176163328/unknown.png)

![](https://cdn.discordapp.com/attachments/1034179399005782036/1034305674689118339/unknown.png)

![](https://cdn.discordapp.com/attachments/1034179399005782036/1034305712140070922/unknown.png)

![](https://cdn.discordapp.com/attachments/1034179399005782036/1034305798911832195/unknown.png)

![](https://cdn.discordapp.com/attachments/1034179399005782036/1034305857468506142/unknown.png)

![](https://cdn.discordapp.com/attachments/1034179399005782036/1034305885683585134/unknown.png)

![](https://cdn.discordapp.com/attachments/1034179399005782036/1034305914544599131/unknown.png)

![](https://cdn.discordapp.com/attachments/1034179399005782036/1034305935495147520/unknown.png)

![](https://cdn.discordapp.com/attachments/1034179399005782036/1034446374999167046/unknown.png)

![](https://cdn.discordapp.com/attachments/1034179399005782036/1034306197186158642/unknown.png)

![](https://cdn.discordapp.com/attachments/1034179399005782036/1034306236516147211/unknown.png)

![](https://cdn.discordapp.com/attachments/1034179399005782036/1034446829967921202/unknown.png)

![](https://cdn.discordapp.com/attachments/1034179399005782036/1034306286621294602/unknown.png)

![](https://cdn.discordapp.com/attachments/1034179399005782036/1034306322570694676/unknown.png)

![](https://cdn.discordapp.com/attachments/1034179399005782036/1034306344402034698/unknown.png)

![](https://cdn.discordapp.com/attachments/1034179399005782036/1034306407547277312/unknown.png)

![](https://cdn.discordapp.com/attachments/1034179399005782036/1034437158741737472/unknown.png)

![](https://cdn.discordapp.com/attachments/1034179399005782036/1034437194150051840/unknown.png)

![](https://cdn.discordapp.com/attachments/1034179399005782036/1034437214265942036/unknown.png)

![](https://cdn.discordapp.com/attachments/1034179399005782036/1034437237733072937/unknown.png)

![](https://media.discordapp.net/attachments/1034179399005782036/1034437315013132349/unknown.png)

![](https://cdn.discordapp.com/attachments/1034179399005782036/1034448462726582333/unknown.png)

![](https://cdn.discordapp.com/attachments/1034179399005782036/1034448523917283348/unknown.png)

![](https://cdn.discordapp.com/attachments/1034179399005782036/1034448546272915558/unknown.png)

# Conclusion

En conclusión la implementación de MLFQ que realizamos presenta una mejoría en cuanto al número de operaciones que realiza ambos tipos de operaciones en los casos que el número de procesos que generen solicitud de inputs o algún tipo de output no generan son suficientes para que solo roten entre ellos (en especial en el caso E1), pero a cambio de un gran riesgo de starvation en los otros casos.
