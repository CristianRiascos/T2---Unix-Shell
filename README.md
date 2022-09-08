# T2 - Unix-Shell

## Cómo se compila?
Mediante el comando "make t2" y posteriormente "./t2" excluyendo los "".

## Cómo funciona?
El programa toma el comando ingresado por el usuario como una cadena de caracteres, posteriormente elimina el '\0' al final de la cadena y almacena los comandos ingresados en un array de char fragmentado por espacios, este array y el número de comandos es pasado como argumento a getCommand.

El programa toma como función principal getCommand que es la encargada de crear al proceso padre e hijo, que se encarga de redirigir (si es necesario) hacia otras funciones para la aplicación de las distintas funcionalidades. Las funcionalidades del programa son:

1. Ejecución de comandos en el proceso hijo 
2. item Historial que de los comandos en ejecución que se guarda en un txt, acepta de igual forma los !! para ejecutar último comando
3. item Redirecciones tanto de <  como de >
4. item Comunicación de 2 procesos via pipe


Todos los comandos son ejecutados por medio de execv, el "caso base" son los comandos básicos tales como ls, ps, cat, echo, etc. También cuenta con control de errores en caso de que el comando ingresado sea erróneo. Los comandos erróneos también son agregados al historial.

Para el historial se implementó el uso de almacenamiento por medio de archivos .txt, el programa está configurado para que mientras esté en ejecución añada cada comando ingresado por el usuario al historial, en caso de que se termine la ejecución del programa se seguirán viendo dichos comandos en el txt, no obstante, si se reinicia el programa el contenido del txt se sobreescribe por uno nuevo. Esto se realizó mediante el manejo de FILE. De igual forma soporta el uso de !! para ejecutar el último comando elegido por el usuario.

Para las redirecciones se usó el manejo de archivos mediante funciones open y close en conjunto a dup2 principalmente. Inicialmente se particiona el comando en 2 lados, lado de comando y lado del nombre del archivo teniendo de base la ubicación del símbolo > o <. Posteriormente se abre el archivo y se indicia que bien sea el output o input sea mediante el archivo y finalmente se ejecuta el execvp mediante una función executeCommand.

Finalmente, la comunicación por pipes entre procesos se hace mediante el dup2 y otro fork. Se hace que el proceso hijo ejecute el primer comando (lado izquierdo del comando |) y que su output sea escrito en el lado de escritura del pipe usando dup2, en el padre cierra el lado de escritura para que sea heredado al hijo y se crea otro fork, en dicho fork el hijo toma el lado de lectura donde recibe lo que haya escrito en el pipe y ejecuta dicho comando (lado derecho del comando |) escribiendo su resultado en la terminal, finalmente el padre cierra el lado de escritura. Al salir de los fork se usan 2 status para determinar el fin de ambos procesos. 
