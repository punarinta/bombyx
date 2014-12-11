#define BOMBYX_WEB 1
#define THREADS_PER_CPU 1
#define SOCKET_PATH ":8000"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <libgen.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "fcgiapp.h"
#include "core/common.h"
#include "core/var_2.h"
#include "core/cocoon_2.h"
#include "core/challoc.h"
#include "core/larva.h"

static int socketId;    // is not written within the thread — safe
void *thread(void *);
long getCpuCount();

int main(void)
{
    pthread_t *id;
    bombyx_env_t **env;
    int i, thread_count;

    verbose = 0;

    FCGX_Init();
    umask(0);
    socketId = FCGX_OpenSocket(SOCKET_PATH, 2000);
    if (socketId < 0)
    {
        fprintf(stderr, "Cannot open socket: %s\n", strerror(errno));
        return 1;
    }

    printf("Socket opened, listening to %s.\n", SOCKET_PATH);

    thread_count = THREADS_PER_CPU * getCpuCount();
    printf("Creating %d threads (%d per CPU)...\n", thread_count, THREADS_PER_CPU);

    env = malloc(sizeof(bombyx_env_t *) * thread_count);
    id = malloc(sizeof(pthread_t) * thread_count);

    for (i = 0; i < thread_count; i++)
    {
        env[i] = calloc(1, sizeof(bombyx_env_t));
        env[i]->thread_id = i + 1;
        pthread_create(&id[i], NULL, thread, (void *)env[i]);
    }

    for (i = 0; i < thread_count; i++)
    {
        pthread_join(id[i], NULL);
    }

    return 0;
}

void *thread(void *a)
{
    int rc;
    bombyx_env_t *env = (bombyx_env_t *)a;

    if (FCGX_InitRequest(&env->request, socketId, 0) != 0)
    {
        printf("Cannot init request in thread #%d.\n", env->thread_id);
        return NULL;
    }

    // alloc data storages for the thread
    env->wd = malloc(sizeof(web_data));
    env->blocks             = block_table_create(MIN_BLOCKS);
    env->vars               = var_table_create(MIN_VARIABLES);
    env->cocoons            = cocoon_table_create(MIN_COCOONS);
    env->pool_of_doubles    = chcreate(POOL_OF_DOUBLES_SIZE, sizeof(double));


    for (;;)
    {
        static pthread_mutex_t accept_mutex = PTHREAD_MUTEX_INITIALIZER;
        static pthread_mutex_t setup_mutex  = PTHREAD_MUTEX_INITIALIZER;

        pthread_mutex_lock(&accept_mutex);
        rc = FCGX_Accept_r(&env->request);
        pthread_mutex_unlock(&accept_mutex);

        if (rc < 0)
        {
            printf("Can not accept new request.\n");
            break;
        }

        char *uri = FCGX_GetParam("DOCUMENT_URI", env->request.envp),
             *filename = FCGX_GetParam("LEAF_FILENAME", env->request.envp);

        printf("Requested '%s', accepted by thread %d.\n", uri, env->thread_id);

        FCGX_PutS("Content-type: text/html\r\n", env->request.out);
        FCGX_PutS("X-Powered-By: Bombyx 0.1\r\n", env->request.out);

        env->gl_error = 0;
        env->wd->body_started = 0;

        char *source;
        size_t newLen = 0;

        FILE *fp = fopen(filename, "rt");

        if (fp != NULL)
        {
            pthread_mutex_lock(&setup_mutex);

            fseek(fp, 0L, SEEK_END);
            long bufsize = ftell(fp);
            source = malloc(sizeof(char) * (bufsize + 2));
            fseek(fp, 0L, SEEK_SET);
            newLen = fread(source, sizeof(char), bufsize, fp);
            source[newLen++] = '\0';
            fclose(fp);

            getcwd(env->dir_home, sizeof(env->dir_home));
            char *dir_leaf_temp = dirname(filename);
            strcpy(env->dir_leaf, dir_leaf_temp);
            chdir(env->dir_leaf);

            pthread_mutex_unlock(&setup_mutex);

#ifdef __APPLE__
            // free(dir_leaf_temp);
#endif

            // cache POST
            const unsigned long STDIN_MAX = 1000000;
            unsigned long content_length = STDIN_MAX;
            char *content_length_str = FCGX_GetParam("CONTENT_LENGTH", env->request.envp);

            if (content_length_str)
            {
                content_length = strtol(content_length_str, &content_length_str, 10);

                if (content_length > STDIN_MAX)
                {
                    content_length = STDIN_MAX;
                }

                env->wd->http_length = content_length;
                env->wd->http_content = malloc(content_length + 1);
                FCGX_GetStr(env->wd->http_content, content_length, env->request.in);
                env->wd->http_content[content_length] = '\0';
            }
            else
            {
                env->wd->http_length = 0;
                env->wd->http_content = NULL;
            }

            setjmp(env->error_exit);

            if (env->gl_error)
            {
                // stop execution, but don't kill thread
                larva_stop(env);
            }
            else
            {
                larva_init(env, source, newLen);
                larva_digest(env);
                larva_silk(env);
                larva_stop(env);
            }
        }
        else
        {
            fputs("File does not exist.\n", stderr);
        }

        if (!env->wd->body_started)
        {
            // no body was sent
            FCGX_PutS("\r\n", env->request.out);
        }

        FCGX_Finish_r(&env->request);
    }

    return NULL;
}

long getCpuCount()
{
    long nprocs = sysconf(_SC_NPROCESSORS_ONLN);

    if (nprocs < 1)
    {
        fprintf(stderr, "Cannot determine number of CPUs online:\n%s\n", strerror(errno));
        return 1;
    }

    printf("%ld x CPU found.\n", nprocs);

    return nprocs;
}