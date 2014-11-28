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
#include "common.h"
#include "core/larva.h"

static int socketId;    // is not written within the thread — safe
void *thread(void *);

int main(void)
{
    int i;
    pthread_t id[THREAD_COUNT];

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
    FCGX_Request request;

    if (FCGX_InitRequest(&request, socketId, 0) != 0)
    {
        printf("Cannot init request\n");
        return NULL;
    }

    for (;;)
    {
        static pthread_mutex_t accept_mutex = PTHREAD_MUTEX_INITIALIZER;

        pthread_mutex_lock(&accept_mutex);
        rc = FCGX_Accept_r(&request);
        pthread_mutex_unlock(&accept_mutex);

        if (rc < 0)
        {
            printf("Can not accept new request\n");
            break;
        }

        char *uri = FCGX_GetParam("DOCUMENT_URI", request.envp),
             *query = FCGX_GetParam("QUERY_STRING", request.envp),
             *cookies = FCGX_GetParam("HTTP_COOKIE", request.envp),
             *filename = FCGX_GetParam("SCRIPT_FILENAME", request.envp),
             *method = FCGX_GetParam("REQUEST_METHOD", request.envp);

        FCGX_PutS("Content-type: text/html\r\n", request.out);
        FCGX_PutS("X-Powered-By: Bombyx 0.1\r\n", request.out);
        FCGX_PutS("\r\n", request.out);

        pRequest = &request;
        verbose = 0;
        gl_error = 0;
        char *source;
        size_t newLen = 0;

        FILE *fp = fopen(filename, "rt");

        if (fp != NULL)
        {
            fseek(fp, 0L, SEEK_END);
            long bufsize = ftell(fp);
            source = malloc(sizeof(char) * (bufsize + 2));
            fseek(fp, 0L, SEEK_SET);
            newLen = fread(source, sizeof(char), bufsize, fp);
            source[newLen++] = '\0';
            fclose(fp);

            getcwd(dir_home, sizeof(dir_home));
            char *dir_leaf_temp = dirname(filename);
            strcpy(dir_leaf, dir_leaf_temp);
            chdir(dir_leaf);

#ifdef __APPLE__
            free(dir_leaf_temp);
#endif

            setjmp(error_exit);

            if (gl_error)
            {
                larva_stop();
            }
            else
            {
                larva_init(source, newLen);
                larva_digest();

                larva_silk(&request);

                larva_stop();
            }
        }
        else
        {
            fputs("File does not exist.\n", stderr);
        }

        end_request:;

        FCGX_Finish_r(&request);
    }

    return NULL;
}
