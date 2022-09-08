#include <stdio.h>
#include <unistd.h> // exec, dup2
#include <sys/types.h>
#include <sys/wait.h> // Wait
#include <stdlib.h> // system("clear")
#include <string.h> // stlren, strncmp
#include <fcntl.h>  // open, close


#define COMMAND_LENGHT 81  /* Máxima cantidad de caracteres + /n */
#define READ_PIPE 0 /* Para leer en el Pipe */
#define WRITE_PIPE 1 /* Para escribir en el Pipe */
#define CONTINUE_PROGRAM 1  // Flag para continuar el programa
#define END_PROGRAM 0   // Flag para terminar programa
#define SIZE 20     // Número de char permitidos
#define LENGHT 20   // Tamaño de los char


/*
    La función recibe una cadena de texto y un array de cadenas de texto.
    La función pone dentro del array las palabras que hayan dentro de la cadena por
    medio de buscar espacios entre la cadena.
    Devuelve el número de palabras encontradas y el array con las palabras.
*/
int parseCommand( char *line, char parse[SIZE][LENGHT] )
{
    int iterator, j, numberWords;

    numberWords = 0;
    j = 0;

    for( iterator=0; iterator<=strlen(line);iterator++ )
    {
        if( line[iterator] != ' ' )
            parse[numberWords][j++] = line[iterator];
        else{
            parse[numberWords][j++] = '\0';
            numberWords++;
            j = 0;
        }       
        if( line[iterator] == '\0' )    
            return numberWords;   
    }
}


// Copia los elementos del segundo arreglo al primero indicando donde inicia y termina
void copyArgs( char copyTo[SIZE][LENGHT], char copyFrom[SIZE][LENGHT], int start, int end )
{
    int i = 0;
    for( start; start<=end; start++ )
        strcpy( copyTo[i++], copyFrom[start] );
    return;
}


void addCommandHistory( char *addComm, int run )
{
    FILE *history;

    /*
        run almacena el valor del número de veces que el usuario ha ingresado comandos, si run=1
        significa que es el primer comando que se ingresa, esto hace que cree el archivo en modo 
        "w" para sobreescribir el archivo, caso contrario en modo "a" para agregar
    */
    if( run == 1 )
        history = fopen( "historial.txt", "w" );

    else    
        history = fopen( "historial.txt", "a" );

    // Control de error
    if( history == NULL ){
        puts( "ERROR: History file could not be created\n" );
        EXIT_FAILURE;
    }

    // Añade el comando al txt
    fprintf( history, "%s\n", addComm );
    fclose( history );

    return;
}


void executeCommand( char commands[SIZE][LENGHT], int totalArgs ) 
{   
    char *file, *args[20];
    int i;
    
    // Hace que la variable file almacene el comando, por ejemplo, ls o clear
    file = commands[0];

    // Asigna los valores de commands a args 
    for( i=0; i<=totalArgs; i++ )
        args[i] = commands[i];
    
    // Asignación de NULL a último valor
    args[totalArgs+1] = NULL;
    
    // Control de error
    if( execvp( file, args ) < 0 ){
        printf( "ERROR: Command '%s' does not exist\n", file );
        EXIT_FAILURE;
    }

    return;
}


// Función para encontrar signo un signo especifico
int searchInLine( char lineOp[SIZE][LENGHT], char* target , int end  )
{
    int i;

    /* 
        For para verificar cada valor en el comando ingresado, compara bytes de target con la posición
        actual para ver si está o no 
    */
    for( i=0; i<=end; i++ ){
        if( strncmp( target, lineOp[i], 1 ) == 0 )
            return i;   // Retorna la posición en donde está el signo 
    }
    
    // Si target no está en lineOp devuelve -1
    return -1;
}


// REVISAR ARGS2 CUANDO SE TERMINE DE AÑADIR LAS FUNCIONALIDADES
void redirection( char redirectComms[SIZE][LENGHT], int redirectPosition, int endCommand ) // Se debe hacer cambio para cuando exec funcione
{
    char arg[10][20], arg2[10][20], *redirect, *higher, *file;
    int i, fd;
    
    higher = ">";
    redirect = redirectComms[redirectPosition];     // Almacena en redirect el signo de redireccionamiento para saber a donde apunta
    
    // Guarda el comando y argumento de redirectComms del lado izquierdo antes de llegar a < o >
    copyArgs( arg, redirectComms, 0, redirectPosition-1 );

    // Guarda el comando y argumento de redirectComms del lado derecho hasta llegar al final de los comandos
    //copyArgs( arg2, redirectComms, redirectPosition+1, endCommand );
    
    file = redirectComms[redirectPosition+1];

    /*
        O_WRONLY = Archivo en modo de solo lectura
        O_RDONLY = Archivo en solo modo de lectura
        O_CREAT = Crear archivo por si no existe
        0777 = Cualquiera tiene acceso al archivo
    */

    // Si el signo es  > entra aquí
    if( strncmp( higher, redirect, 1 ) == 0 ){

        // Abre y/o crea el archivo si no existe en permisos para todo usuario
        fd = open( file, O_WRONLY | O_CREAT, 0777 );

        if( fd < 0 ){       // Control de errores
            puts( "ERROR: File descriptor could not be created\n" );
            EXIT_FAILURE;  
        }

        // Copia lo que se escriba en la terminal al archivo
        dup2( fd, STDOUT_FILENO );
        close( fd );
        executeCommand( arg, redirectPosition-1 );
    }

    // Si el signo es < entra aquí
    else{            
        // Asigna el nombre del archivo a la siguiente posición de args
        strcpy( arg[endCommand-1], file );
        executeCommand( arg, endCommand-1 );
    }
    return;

}


void getCommand( char commands[SIZE][LENGHT], int lastNumber, int totalComms  )
{
    pid_t pid, pid2; 
    FILE *exclamation;
    char *arg, *arg2, *exclamSymbol, leftVal[SIZE][LENGHT], rightVal[SIZE][LENGHT]; 
    char lastCommand[COMMAND_LENGHT], lastArg[SIZE][LENGHT], *compLast;  
    int getSearchHigher, getSearchLower, getSearchPipe, fd[2], getInPipe, lastNumberDouble, move;

    pid = fork();
    arg = commands[0];  // Almacena el primer comando en un apuntador
    getInPipe = 0;  // Flag para saber si hay un | y crear fork en padre
    exclamSymbol = "!!";    // Puntero a !! para verificar su existencia en el comando
    move = totalComms;  // Variable para leer líneas cuando se pone !!
    getSearchHigher = searchInLine( commands, ">", lastNumber );
    getSearchLower = searchInLine( commands, "<", lastNumber );
    getSearchPipe = searchInLine( commands, "|", lastNumber );
    
    if( pid > 0 ) /* Proceso padre */
        wait(NULL);


    else if( pid == 0 ) /* Proceso hijo - Ejecutar comando */
    {

        // Si hay un | entra aquí
        if( getSearchPipe != -1 )
        {
            pipe( fd );
            pid2 = fork();
            int status;     // Status para saber cuando los fork hayan terminado

            if( pid2 > 0 )      // Primer padre    
            {
                close( fd[WRITE_PIPE] );
                pid2 = fork();

                if( pid2 == 0 )     // Segundo hijo - Ejecuta comando de la izquierda
                {
                    dup2( fd[READ_PIPE], STDIN_FILENO );
                    close( fd[READ_PIPE] );
                    copyArgs( rightVal, commands, getSearchPipe+1, lastNumber );
                    executeCommand( rightVal, lastNumber-(getSearchPipe+1) );
                }
                else    // Cerrar el puerto de lectura
                    close( fd[READ_PIPE] );
            }

            else    // Primer hijo -  Ejecuta comando de la derecha
            {
                close( fd[READ_PIPE] );
                dup2( fd[WRITE_PIPE], STDOUT_FILENO );
                close( fd[WRITE_PIPE] );
                copyArgs( leftVal, commands, 0, getSearchPipe-1 );
                executeCommand( leftVal, getSearchPipe-1 );
            }

            // Wait para cada proceso
            wait( &status );
            wait( &status );

        }


        // Si el comando ingresado es igual a !! entra aquí
        else if( strncmp( exclamSymbol, arg, 2 ) == 0  )
        {
            move--;
            // Control de errores si no hay comandos previos
            if( move == 0 ){
                puts( "ERROR: There are no commands in history\n" );
                EXIT_FAILURE;
                puts("\n");
            }

            exclamation = fopen( "historial.txt", "r" );    

            // Control de errores por si no se pudo crear el archivo 
            if( exclamation == NULL ){        
                puts( "ERROR: History file could not be accessed\n" );
                EXIT_FAILURE;
            }

            while( move > 0 ){
                fgets( lastCommand, COMMAND_LENGHT, exclamation );
                move--;
            }

            lastCommand[ strcspn(lastCommand, "\n") ] = 0;
            lastNumberDouble = parseCommand( lastCommand, lastArg );
            
            addCommandHistory( lastCommand, totalComms );
            getCommand( lastArg, lastNumberDouble, totalComms );

        
        }


        // Si hay un signo de redireccionamiento entra aquí
        else if( (getSearchHigher != -1) || (getSearchLower != -1) )
            {
                if( getSearchHigher != -1 ) 
                    redirection( commands, getSearchHigher, lastNumber );
                else
                    redirection( commands, getSearchLower, lastNumber );
            }

        else
            executeCommand( commands, lastNumber  );

    }   

    else{   /* Control de error */
        puts( "ERROR: Fork failed\n" );
        EXIT_FAILURE;
    }
    
    return;
}


int main()
{
    char commandLine[COMMAND_LENGHT], args[SIZE][LENGHT], *exclam;
    int continueProgram, lastWordNumber, totalLines; 
    
    continueProgram = CONTINUE_PROGRAM; // Variable para continuar o terminar programa
    totalLines = 0; // Se inicializa en cero por ser el primer comando
    exclam = "!!";
    system( "clear" );

    while( continueProgram )
    {
        printf( "chris> " );
        fflush( stdout );
        fflush( stdin );   
        
        fgets( commandLine, COMMAND_LENGHT, stdin );
        commandLine[strcspn(commandLine, "\n")] = 0;
        totalLines++;
        lastWordNumber = parseCommand( commandLine, args );
        
        // Si el comando ingresado no es !! entra aquí
        if( strcmp( args[0], exclam ) )
        {
            addCommandHistory( commandLine, totalLines );
            getCommand( args, lastWordNumber, totalLines ); 
        }
        // Si el comando ingresado es !!
        else
            getCommand( args, lastWordNumber, totalLines );

    } 

    return 0;
}