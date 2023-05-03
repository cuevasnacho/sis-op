# **Sistemas Operativos**

## **Laboratorio 2 : Semáforos en xv6**

**Programando nuestras propias llamadas al sistema para xv6**
<br></br>
Desarrollado por:

+ Salas, Pedro
+ Hubmann, Tomás
+ Strasorier, Marcos
+ Cuevas, Ignacio









Profesores a cargo:

+ Facundo Bustos
+ Luigi Finetti




**Universidad Nacional de Córdoba, Fa.M.A.F.**


<br></br>

<br></br>

<br></br>

<br></br>









## **Introducción**
En este laboratorio creamos nuestras propias llamadas al sistema (syscalls) para la implementación de un sistema de semáforos en el sistema operativo xv6-riscv. Para ello comenzamos investigando cómo debe ser el comportamiento de un semáforo nombrado. Luego leímos el código de xv6 para comprender cómo se implementan las syscalls en este sistema operativo (qué archivos debíamos modificar), para guiarnos con ellas y usarlas para hacer las que eran necesarias para poner en marcha la creación del sistema de semáforos (sem_open, sem_close, sem_up y sem_down).

## **Syscalls**
Para el correcto funcionamiento de las funciones mencionadas anteriormente (implementadas por nosotros) fue necesario utilizar las siguientes syscalls (propias de xv6) que se comportan de las siguientes maneras:

**acquire()**
Esta syscall toma como argumento una instancia del tipo spinlock y para cortar todas las interrupciones del sistema y de esta manera garantizar que se ejecuten exclusivamente partes del código que sean “delicadas” (que de haber una interrupción lleve al mal funcionamiento del mismo), siempre y cuando el spinlock no haya sido bloqueado con anterioridad, en este caso saltara un error.


**release()**
Esta llamada es la “función inversa de acquire”, vuelve a permitir las interrupciones que habían sido previamente bloqueadas por acquire. Así que podríamos decir que donde hay un acquire debe haber un release, para evitar que el código de usuario o de kernel se apropie del (o los) cpu. 


**sleep()**

Con sleep ponemos a “dormir” un proceso en un determinado valor “chan” (ubicación donde éste se va a dormir), para que podamos ejecutar otro en su lugar. Hasta que no haya una llamada que lo vuelva a poner “ready”.

**wakeup()**

Wakeup nos permite ”levantar” todos los procesos que estaban durmiendo en el mismo valor “chan” que se utilizó para ponerlos a dormir, independientemente de qué proceso llama a la función. Dejando a todos los procesos listos para ejecutarse cuando el scheduler lo decida. 

**argint()**

Argint cumple la función de leer un argumento y pasarselo a las syscalls que creamos. Lo usamos en sysproc.c (donde se definen las funciones que manejan las llamadas del sistema).

## **Decisiones de diseño**

### **Uso de la función de decrease**

La necesidad de usar decrease se dio porque debíamos tener cuidado en el caso de que hubiera múltiples procesos durmiendo en el mismo semáforo, y que no todos los procesos decrementen el valor de “value” a la vez, evitando que  pueda ser negativo y prevenir un mal funcionamiento del semáforo.Esto se hace en una función aparte en vez de llamar de nuevo a sem_down para evitar el costo computacional de encontrar el semaforo con esa id en el arreglo.

### **Uso de spinlock dentro del semáforo**

Es necesario usar un nuevo lock específico para cada semáforo debido a que encontramos una vulnerabilidad del código durante la ejecución del sleep, que consiste en la posibilidad de perder el control del lock global por otra función que lo necesite mientras la función sleep todavía lo esté usando. 

### **Función de set_sems**

Se creó para ser ejecutada la primera vez que se abre un semáforo. Esta función setea todos los valores del arreglo de semáforos en -1, esto nos sirve para identificar si un semáforo está cerrado o no. De esta forma la única forma de que un value sea -1 es internamente, dejando el valor reservado para el kernel. Esta fue la solución más simple que se nos ocurrió ya que si seteamos cada id con un valor fijo, este podría coincidir con el ingresado por el usuario y que luego se pise al abrir uno nuevo. 

### **Arreglo de semáforos**

Creamos un arreglo global (siguiendo el formato de xv6 con los procesos y los cpus), donde se guardan todos los semáforos del sistema. El número máximo de estos, está definido en param.h con un define, de este modo podemos cambiar la cantidad límite de semáforos desde allí.

### **”sem” como id universal**

Nuestro semáforo tiene implementado un sistema de Id que te permite pasarle cualquier unsigned int como identificador, independientemente de la posición en el array en la que está, de esta forma, el usuario no tiene la necesidad de saber el índice real del semáforo que está usando. Luego en sem_open(), se le asigna a este semáforo nuevo, la primera posición disponible si es que existe.
