#define NUM_MAX_THREADS 100

typedef struct _threadpool
{
    int num_threads;        //Numero de hilos
    int activated_threads;  //Numero de hilos que estan ejecutandose
    pthread_mutex_t mutex;  //Semaforo mutex
    pthread_t *threads;     //Array de hilos
}threadpool;

/**
 * @brief Crea un threadpool con el numero de hilos asignado
 * 
 * @param num_threads numero de hilos
 * @return puntero threadpool
 */
threadpool* threadpool_create(int num_threads);

/**
 * @brief Crea los hilos que estarán esperando a los usuarios
 * @param tp threadpool previamente creado
 * 
 * @return Correcto=0  Error=-1
 */
int threadpool_start(threadpool *tp);

/**
 * @brief Elimina el pthreadpool
 * 
 */
void threadpool_destroy(threadpool *tp);