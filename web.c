#define THREAD_COUNT 8
#define SOCKET_PATH ":8000"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "fcgiapp.h"
#include "core/common.h"
#include "core/larva.h"

static int socketId;    // is not written within the thread — safe
void *thread(void *);

int main(void)
{
    int i;
    pthread_t id[THREAD_COUNT];
    verbose = 0;

    FCGX_Init();
    umask(0);
    socketId = FCGX_OpenSocket(SOCKET_PATH, 2000);
    if (socketId < 0)
    {
        return 1;
    }

    printf("Socket is opened. Creating %d threads...\n", THREAD_COUNT);

    for (i = 0; i < THREAD_COUNT; i++)
    {
        pthread_create(&id[i], NULL, thread, NULL);
    }

    for (i = 0; i < THREAD_COUNT; i++)
    {
        pthread_join(id[i], NULL);
    }

    return 0;
}

void *thread(void *a)
{
    int rc;
    bombyx_env_t *env = calloc(1, sizeof(bombyx_env_t));

    if (FCGX_InitRequest(&env->request, socketId, 0) != 0)
    {
        printf("Cannot init request\n");
        return NULL;
    }

    for (;;)
    {
        static pthread_mutex_t accept_mutex = PTHREAD_MUTEX_INITIALIZER;
        static pthread_mutex_t setup_mutex  = PTHREAD_MUTEX_INITIALIZER;

        pthread_mutex_lock(&accept_mutex);
        rc = FCGX_Accept_r(&env->request);
        pthread_mutex_unlock(&accept_mutex);

        if (rc < 0)
        {
            printf("Can not accept new request\n");
            break;
        }

        char *uri = FCGX_GetParam("DOCUMENT_URI", env->request.envp),
             *query = FCGX_GetParam("QUERY_STRING", env->request.envp),
             *cookies = FCGX_GetParam("HTTP_COOKIE", env->request.envp),
             *filename = FCGX_GetParam("SCRIPT_FILENAME", env->request.envp),
             *method = FCGX_GetParam("REQUEST_METHOD", env->request.envp);

        FCGX_PutS("Content-type: text/html\r\n", env->request.out);
        FCGX_PutS("X-Powered-By: Bombyx 0.1\r\n", env->request.out);
        FCGX_PutS("\r\n", env->request.out);

        env->gl_error = 0;
        size_t newLen = 0;
        char *source;

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
            free(dir_leaf_temp);
#endif

            setjmp(env->error_exit);

            if (env->gl_error)
            {
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

        end_request:;

        FCGX_Finish_r(&env->request);
    }

    if (env) free(env);

    return NULL;
}
