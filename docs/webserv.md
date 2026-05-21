# Como hacer mi webserv

## Webserver - 1

He analizado a fondo el proyecto furyweb de tu compañero. Aquí tienes una guía
  detallada de cómo está estructurado y las "normas" o patrones clave que puedes usar
  como inspiración para tu propio webserv:

  1. Arquitectura General
  El proyecto sigue una estructura orientada a objetos muy clara, separando las
  responsabilidades de red, parsing de protocolos y ejecución de scripts.

   * Http (El Orquestador): Contiene el bucle principal (select()). Gestiona qué
     sockets están listos para leer o escribir.
   * Server (Configuración y Acceso): Representa un servidor virtual. Maneja el bind,
     listen, accept y decide qué hacer con una petición según la configuración.
   * RequestParser (La "Joyita"): Un autómata finito (máquina de estados) que procesa
     la petición carácter por carácter. Esto es fundamental para I/O no bloqueante, ya
     que permite pausar y reanudar el parsing si los datos llegan fragmentados.
   * Response (Generador): Mapea la petición al sistema de archivos, maneja errores
     (404, 405, etc.), autoindex y prepara el cuerpo de la respuesta.
   * Cgi (Ejecutor): Gestiona la ejecución de scripts externos (Python, JS) mediante
     fork y execve.

  2. El "Corazón" No Bloqueante (Http.cpp)
  Tu compañero utiliza select() para manejar múltiples clientes sin necesidad de hilos
  (threads).
   * Sets de FDs: Mantiene un read_set y un write_set. 
   * Transición: Solo añade un socket al write_set cuando el RequestParser devuelve
     PARSE_SUCCESS. Mientras está leyendo, el socket solo está en el read_set.
   * Non-blocking: Todos los sockets (tanto el del servidor como los de los clientes)
     se configuran con O_NONBLOCK usando fcntl.

  3. Parsing Robusto (RequestParser.cpp)
  En lugar de usar split o funciones pesadas de strings, utiliza un switch gigante con
  estados como REQUEST_METHOD, URI, HEADER_KEY, POST_BODY, etc.
   * Ventaja: Puede manejar peticiones Chunked de forma natural.
   * Eficiencia: Es extremadamente rápido y consume poca memoria, ya que procesa el
     flujo de datos según entra por el socket.

  4. Manejo de CGI (Cgi.cpp)
  Es uno de los puntos más complejos. Lo hace así:
   1. Entorno: Crea un array de strings char** env con variables como CONTENT_LENGTH,
      QUERY_STRING, etc.
   2. Redirección: Usa dup2 para que el stdin del script sea un archivo temporal con el
      cuerpo del POST, y su stdout sea otro archivo temporal donde se captura la
      salida.
   3. Ejecución: fork() -> execve(). El servidor padre espera el resultado (ojo: su
      implementación es síncrona en el wait(), podrías mejorar esto haciéndolo
      asíncrono).

  5. Trucos y Buenas Prácticas (Normas)
   * Normalización de Rutas: Tiene una función normalizePath en Utils.cpp que limpia
     los .. y . de la URI. Esto es vital para evitar ataques de Directory Traversal
     (que alguien acceda a /etc/passwd a través de tu servidor).
   * MIME Types: Mapea extensiones (.jpg, .html, .json) a sus tipos correspondientes
     para que el navegador sepa cómo renderizar el contenido.
   * Páginas de Error Personalizadas: Si el config define un error 404, el servidor
     busca ese archivo antes de mostrar el error por defecto.

  ¿Por dónde empezar tu proyecto?
  Si quieres inspirarte en él, te recomiendo este orden:
   1. Define tu clase Config: Lee un archivo simple y extrae puerto, host y rutas.
   2. Crea el Socket Servidor: Haz que escuche en un puerto.
   3. Implementa el select() básico: Acepta un cliente y léele un "Hello world".
   4. Crea tu RequestParser de estados: Es lo que más tiempo te llevará pero lo que
      hará que tu proyecto sea sólido.
   5. Añade la lógica de Response: Servir archivos estáticos primero, y luego CGI.

## Webserver - 2

✦ Entender cómo está construido un servidor web como este es el primer paso para crear
  el tuyo. Tu compañero ha seguido una arquitectura modular y robusta basada en
  Programación Orientada a Objetos (POO) y un modelo de E/S multiplexada.

  Aquí tienes la ingeniería inversa detallada del proyecto webservachraf:

  1. El Corazón: El Bucle de Eventos (poll)
  A diferencia de un servidor simple que solo atiende a un cliente a la vez, este
  utiliza poll().
   - Cómo funciona: El servidor mantiene una lista de todos los "File Descriptors"
     (FDs) abiertos. Esto incluye los sockets que "escuchan" (donde llegan nuevos
     clientes) y los sockets de los clientes ya conectados.
   - Eficiencia: El programa se queda "durmiendo" en la función poll() y el Sistema
     Operativo lo despierta solo cuando hay datos listos para leer o escribir. Esto
     permite manejar cientos de conexiones simultáneas en un solo hilo.

  2. Estructura Jerárquica del Servidor
  El código divide las responsabilidades en tres capas:
   1. Server (El Orquestador): Contiene el bucle principal (start()). Decide si un FD
      listo es una nueva conexión (llama a accept) o un cliente enviando una petición
      (llama a _handleRequest).
   2. VirtualServer (La IP/Puerto): Representa un socket de escucha en un puerto
      específico (ej. 8080). Si tienes dos bloques de servidor en el mismo puerto pero
      con distinto server_name, ambos pertenecen al mismo VirtualServer.
   3. SubServer (El Host Virtual): Contiene la configuración específica de un dominio
      (root, index, error pages). Es el encargado de saber qué archivos servir.

  3. El Parser de HTTP: Una Máquina de Estados
  Esta es la parte más técnica y elegante. En lugar de leer toda la petición de golpe
  (lo cual sería peligroso con peticiones grandes), usa una Máquina de Estados Finita
  (FSM) en HttpParser:
   - Lee carácter por carácter.
   - Estados: p_status_line (leyendo GET /...), p_headers (leyendo Host,
     User-Agent...), y p_body (leyendo el contenido).
   - Ventaja: Puede manejar peticiones "fragmentadas" o muy lentas sin bloquear el
     resto del servidor.

  4. Flujo de una Petición (Ingeniería de Vuelo)
  Cuando llega un GET /index.html:
   1. Multiplexión: poll detecta datos en el socket del cliente.
   2. Parsing: El HttpParser construye un objeto Request.
   3. Matching: El Server busca qué SubServer coincide con el header Host y qué
      location coincide con la URI /index.html.
   4. Ejecución:
       * Si es un archivo estático: Lo lee y construye una Response.
       * Si es CGI (como un script .py): Llama al CGIHandler, hace un fork(), ejecuta
         el script y devuelve la salida al cliente.
       * Si es un directorio y no hay index: El IndexGenerator crea un HTML sobre la
         marcha (Autoindex).

  5. Detalles de Implementación "Pro"
   - Configuración TOML: En lugar de un formato inventado difícil de parsear, usa TOML,
     lo que hace que el archivo default.toml sea muy legible y fácil de extender.
   - Gestión de Errores: Tiene un sistema de mapas para error_pages. Si algo falla,
     busca en la configuración si el usuario definió un HTML personalizado para ese
     código (ej. 404).
   - MIME Types: El servidor sabe qué Content-Type enviar (image/png, text/html)
     gracias a un mapa que relaciona extensiones con tipos.

  ¿Cómo puedes replicarlo en "tu terreno"?
  Para llevarlo a tu terreno, te sugiero este orden:
   1. Empieza por el Socket: Crea un programa que escuche en un puerto y haga un
      accept() simple.
   2. Añade poll o select: Modifícalo para que acepte varios clientes a la vez.
   3. Crea tu Parser: No uses regex. Intenta hacer una función que lea línea a línea e
      identifique el método y la URI.
   4. Separa la Lógica: Crea una clase Response que reciba un código de estado y un
      cuerpo, y sepa formatearse como HTTP.

## Webserver - 3

 ¡Hola! Entiendo perfectamente tu objetivo. Hacer ingeniería inversa de un proyecto
  funcional es una de las mejores formas de aprender cómo se estructuran sistemas
  complejos como un servidor web.

  He analizado el código de tu compañero y aquí tienes una explicación detallada de
  cómo está construido, dividida por sus componentes fundamentales. Este diseño sigue
  un patrón orientado a objetos en C++ y utiliza I/O Multiplexing (multiplexación de
  entrada/salida) con select, lo cual es el estándar para servidores de alto
  rendimiento.

  ---

  1. El Corazón del Servidor: ServerManager (Multiplexación)
  Tu compañero decidió centralizar toda la lógica de ejecución en una clase llamada
  ServerManager. En lugar de crear un hilo por cada cliente (que consumiría mucha
  memoria), utiliza select().

   * Flujo Principal (runServers): Es un bucle infinito que vigila un "pool" de
     descriptores de archivos (sockets).
   * Gestión de Estados: El servidor sabe en qué estado está cada cliente:
       * Si el socket del servidor está "listo", llama a acceptNewConnection.
       * Si un socket de cliente tiene datos, llama a readRequest.
       * Si un socket de cliente está listo para recibir, envía la respuesta con
         sendResponse.
   * No bloqueante: Todos los sockets se configuran con fcntl(..., O_NONBLOCK). Esto
     asegura que el servidor nunca se quede "congelado" esperando a un cliente lento.

  2. Análisis de Peticiones: HttpRequest (FSM)
  Para procesar lo que envía el navegador, utiliza una Máquina de Estados Finos (FSM).
  Esto es muy elegante y eficiente.

   * Cómo funciona: A medida que llegan bytes por el socket, se pasan a la función
     feed(). La clase tiene un enumerado ParsingState (como Request_Line, Field_Name,
     Message_Body).
   * Ventaja: Si una petición llega en varios paquetes pequeños, el servidor guarda el
     estado y continúa parseando exactamente donde se quedó la última vez sin perder
     datos.

  3. Generación de Respuestas: Response
  Una vez que la petición está completa, la clase Response entra en acción.
   * Mapeo de Recursos: Busca el archivo solicitado en el sistema de archivos basándose
     en la configuración (ServerConfig y Location).
   * Tipos MIME: Tiene una clase Mime que asocia extensiones (como .html, .jpg) con su
     tipo de contenido correspondiente para que el navegador sepa qué está recibiendo.
   * Manejo de Errores: Si algo falla (un 404 o un 500), genera automáticamente una
     página de error, ya sea una por defecto o una personalizada definida en el archivo
     de configuración.

  4. Ejecución de Scripts: CgiHandler
  Esta es la parte que permite ejecutar código dinámico (como Python o PHP).
   * Fork & Exec: Cuando detecta que el recurso es un script CGI, el servidor hace un
     fork().
   * Tuberías (Pipes): Crea dos tuberías para comunicarse con el proceso hijo:
       1. Una para enviarle el cuerpo de la petición (stdin).
       2. Otra para leer el resultado del script (stdout).
   * Variables de Entorno: Configura manualmente el entorno (PATH_INFO, QUERY_STRING,
     etc.) para que el script sepa qué procesar.

  5. Configuración: ConfigParser
  El servidor es modular porque lee un archivo .conf.
   * Virtual Hosts: Permite tener múltiples servidores escuchando en diferentes puertos
     o con diferentes nombres (como localhost o misitio.com) dentro del mismo proceso.
   * Bloques de Localización: Puedes definir reglas específicas para carpetas (por
     ejemplo: "en /uploads, solo permite el método POST").

  ---

  ¿Cómo podrías replicarlo "a tu terreno"?

  Si quieres crear tu propia versión inspirándote en esta, te sugiero este orden:

   1. Crea el Socket básico: Haz un servidor que solo escuche en un puerto y acepte una
      conexión.
   2. Implementa el select: Mueve esa lógica a un bucle que pueda manejar varios
      clientes simultáneamente.
   3. Parser de HTTP sencillo: No intentes hacer la FSM completa al principio. Empieza
      leyendo la primera línea (GET / index.html) y construye una respuesta manual.
   4. Añade la Configuración: Crea una clase que lea un archivo y guarde cosas como el
      puerto y el "root" (carpeta donde están los archivos).
   5. CGI (Opcional al final): Deja esto para el final, ya que manejar procesos hijos y
      tuberías es la parte más propensa a errores.

  Un detalle clave que usó tu compañero: El uso de std::map<int, Client>. Esto le
  permite asociar cada socket (un simple número entero) con un objeto Client que guarda
  toda la información de esa conexión específica (su petición, su respuesta y su tiempo
  de actividad para cerrar conexiones inactivas).

## Webserver - 4

 Este proyecto es una excelente base para entender cómo funciona un servidor web desde
  cero. Tu compañero utilizó una arquitectura orientada a objetos en C++98 (típica de
  42) que separa muy bien las responsabilidades.

  Aquí tienes la ingeniería inversa detallada de cómo está construido:

  1. El Corazón: La Clase Cluster
  El servidor no maneja una sola conexión a la vez, sino que orquesta múltiples
  servidores virtuales.
   * Multiplexación con select(): En Cluster::run(), se utiliza un único bucle infinito
     con la función select(). Esto permite monitorizar decenas de sockets (escucha y
     clientes) simultáneamente sin usar hilos (threads), lo cual es muy eficiente en
     términos de recursos.
   * Gestión de FDs: Mantiene un set de descriptores de fichero (_fd_set). Si un socket
     de escucha está "listo", acepta una nueva conexión. Si un socket de cliente está
     "listo para leer", recibe datos. Si está "listo para escribir", envía la
     respuesta.

  2. El Ciclo de Vida de una Petición (Server)
  Cada instancia de la clase Server representa un servidor configurado en un
  host:puerto específico.
   1. recv(): Lee los datos del socket. No asume que la petición llega entera de golpe.
      Controla el final de la petición buscando \r\n\r\n y verificando el
      Content-Length o si es chunked.
   2. process(): Una vez recibida la petición completa:
       * Crea un objeto Request que parsea el texto plano de HTTP (método, headers,
         body).
       * Busca la configuración específica (RequestConfig) para esa URL (mapeo de
         rutas, permisos, CGI).
       * Llama a Response::call() para generar la respuesta.
   3. send(): Envía la respuesta generada al cliente en trozos pequeños para no
      bloquear el servidor.

  3. Resolución de Configuración y Virtual Hosting
  El sistema de configuración es jerárquico.
   * Config y ConfigReader: Parsean el archivo .conf. Permiten definir múltiples
     bloques server con diferentes server_name compartiendo el mismo puerto.
   * RequestConfig: Es una clase "al vuelo". Cuando llega una petición, el servidor
     busca en su configuración qué reglas se aplican a esa ruta específica (directorio
     raíz, archivos índice, si el CGI está activo, etc.).

  4. Generación de Respuesta (Response)
  Es una "fábrica" de respuestas HTTP.
   * Mapeo de Métodos: Usa un std::map de punteros a funciones para despachar la lógica
     según el método (GET, POST, DELETE, etc.).
   * Static Files: Si es un archivo normal, lo lee y determina el Content-Type por la
     extensión.
   * Autoindex: Si pides un directorio y no hay un archivo índice, la clase
     AutoIndexGenerator genera un HTML dinámico con el listado de archivos (similar a
     lo que hace Apache o Nginx).

  5. CGI (Common Gateway Interface)
  Para ejecutar scripts (como PHP o Python):
   * fork() y execve(): Crea un proceso hijo, configura las variables de entorno
     necesarias (como QUERY_STRING o REQUEST_METHOD) y ejecuta el binario del CGI.
   * Comunicación vía Pipes/Archivos: Usa archivos temporales (tmpfile()) para pasar el
     cuerpo de la petición al CGI y capturar su salida, que luego se devuelve al
     cliente.

  ¿Cómo "llevártelo a tu terreno"?
  Si quieres replicarlo pero mejorarlo, aquí tienes unos puntos clave:
   1. Cambia select por epoll (Linux) o kqueue (BSD/Mac): Son mucho más rápidos para
      miles de conexiones.
   2. Manejo de estados: En lugar de almacenar strings gigantes en _requests, usa un
      buffer circular o una máquina de estados para el parseo, así ahorrarás memoria.
   3. Abstracción de E/S: Crea una clase EventLoop que se encargue solo de la red,
      dejando a Server solo la lógica HTTP.

  Este código es muy "limpio" porque separa el cómo se comunica (Cluster/Server) de qué
  se comunica (Request/Response). ¡Es un gran punto de partida!

## Webserv - 5

¡Hola! Es genial que quieras hacer ingeniería inversa a este proyecto. El servidor de
  tu compañero es un ejemplo sólido de cómo implementar un servidor web moderno y
  eficiente desde cero en C++, siguiendo los principios de la arquitectura dirigida por
  eventos.

  Aquí tienes una explicación detallada de cómo está estructurado y cómo funciona, para
  que puedas replicarlo y adaptarlo a tu propio estilo.

  ---

  1. La Arquitectura: Multiplexación con epoll
  Lo primero que debes entender es que este no es un servidor que crea un hilo (thread)
  por cada conexión. Eso sería ineficiente con miles de clientes. En su lugar, utiliza
  epoll (en Connection.cpp).

   * El Corazón (Connection): Esta clase gestiona el bucle principal (eventLoop).
     Registra todos los sockets (los que escuchan puertos y los de los clientes) en una
     instancia de epoll.
   * Eficiencia: Cuando sucede algo (llega una conexión o datos), el sistema operativo
     avisa al servidor. Esto permite manejar cientos de conexiones en un solo hilo de
     ejecución.

  2. Organización de la Configuración (Jerarquía y Herencia)
  Tu compañero usó un patrón muy inteligente para manejar el archivo de configuración:

   * Context (La Base): Define directivas comunes (como root, index, error_page).
   * Server y Location: Ambas heredan de Context. Esto permite que una "Location"
     herede la configuración del "Server", pero también pueda sobrescribirla (por
     ejemplo, un root diferente para /scripts).
   * Virtual Hosting: El servidor puede distinguir entre diferentes dominios en el
     mismo puerto usando el header Host (implementado en Response::matchServer).

  3. El Ciclo de Vida de una Petición (Request-Response)
  El flujo es "reactivo":

   1. Parsing (Request.cpp): Cuando llegan datos, se crea un objeto Request. Este lee
      la primera línea (Método, Recurso, Protocolo) y los encabezados. Si algo falla
      (ej. URL demasiado larga), lanza una excepción que se convierte automáticamente
      en un código de error HTTP.
   2. Matching (Response.cpp): Antes de responder, el servidor busca:
       * ¿A qué bloque server pertenece esta petición? (Por puerto o IP/Host).
       * ¿Qué bloque location encaja mejor? (Usa un algoritmo de "Longest Prefix
         Match", similar a Nginx).
   3. Ejecución (doResponse): Una vez sabe qué tiene que hacer, elige un "handler":
       * Estático: Lee un archivo del disco y lo envía.
       * Autoindex: Si pides una carpeta y está activado, genera dinámicamente un HTML
         con la lista de archivos.
       * CGI (Dinámico): Si la ruta coincide con un script (ej. .php), usa fork() y
         execve() para ejecutarlo, pasando los datos por tuberías (pipes).
       * Upload/Delete: Maneja métodos POST (con multipart) y DELETE directamente en el
         sistema de archivos.

  4. Detalles de Implementación "Pro"
  Si quieres llevarlo a tu terreno, fíjate en estos puntos clave:

   * CGI y el Entorno: En Response::getEnv, verás cómo se preparan las variables de
     entorno (como QUERY_STRING o REQUEST_METHOD). Esto es vital para que scripts
     externos entiendan la petición.
   * Manejo de Errores: En lugar de if/else infinitos, utiliza excepciones
     personalizadas en el constructor de Request y Response. Esto hace que el código
     sea mucho más limpio y fácil de seguir.
   * Sockets No Bloqueantes: Todos los sockets se configuran como SOCK_NONBLOCK. Esto
     es fundamental para que el servidor nunca se "quede colgado" esperando a un
     cliente lento.

  Resumen para tu propio Webserver:
  Si vas a replicarlo, te sugiero este orden:
   1. Networking Base: Crea un socket simple que escuche un puerto.
   2. Event Loop: Implementa epoll (o poll si prefieres algo más sencillo al inicio).
   3. Parsing: Crea una clase que entienda el protocolo HTTP (separa los headers del
      body).
   4. Routing: Implementa la lógica de carpetas y archivos (root e index).
   5. CGI: Deja esto para el final, ya que requiere entender bien procesos (fork) y
      redirección de entrada/salida (dup2).