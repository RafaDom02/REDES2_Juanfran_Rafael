# REDES2_Juanfran_Rafael

Esta practica ha sido realizada por [Juan Francisco Flores Bocanegra](https://github.com/juanfranUam) y por [Rafael Dominguez Saez](https://github.com/RafaDom02).

Este trabajo es un servidor básico para comprender el funcionamiento de los métodos `GET`, `POST` y `OPTIONS`, creando para ello un socket.

Uso
---

1º Instalar el paquete libconfuse-dev para poder compilar el programa

2º Configuraremos a nuestro gusto el fichero `server.conf` con los siguientes parametros:

    interface=(nombre de la interfaz en la que se abrirá el socket, para ello ip=Default, en caso de que interface=Default, se abrirá el socket a nivel del router).
    ip=(nombre de la IP, ip=default si se quiere usar una interfaz).
    server_signature=(Nombre del servidor).
    port=(Numero del puerto al que se quiere conectar).
    childs=(Numero de hijos del pre-fork).
    
3º Ejecución del `Makefile` ya sea con `make` o `make all`. 

4º Ejecución del servidor ejecutando `server` que se encuentra dentro de la carpeta /[www](https://github.com/RafaDom02/REDES2_Juanfran_Rafael/tree/main/www).

5º Por la terminal se imprimirá la `IP`, `Puerto`, junto a más información de interes.

6º Abrir un navegador de internet y acceder realizando una busqueda URL como `IP:Puerto`.

Referencias
-----------

Codigo de [Pacman](https://codepen.io/hellokatili/pen/xwKRmo).

Codigo de [Snake](https://codepen.io/CaioPaiola/pen/nojJmQ).
