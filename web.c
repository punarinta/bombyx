#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "fcgi_stdio.h"

#define THREAD_COUNT 8
#define SOCKET_PATH ":8000"

static int socketId;
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

    printf("Socket is opened\n");

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
    int i, rc;
    FCGX_Request request;

    if (FCGX_InitRequest(&request, socketId, 0) != 0)
    {
        printf("Can not init request\n");
        return NULL;
    }

    /*streambuf *cin_streambuf  = cin.rdbuf();
    streambuf *cout_streambuf = cout.rdbuf();
    streambuf *cerr_streambuf = cerr.rdbuf();*/

    for (;;)
    {
        static pthread_mutex_t accept_mutex = PTHREAD_MUTEX_INITIALIZER;

        pthread_mutex_lock(&accept_mutex);
        rc = FCGX_Accept_r(&request);
        pthread_mutex_unlock(&accept_mutex);

        if (rc < 0)
        {
            printf("Can not accept new request\n");
            break; // continue; (?)
        }

        /*fcgi_streambuf cin_fcgi_streambuf(request.in);
        fcgi_streambuf cout_fcgi_streambuf(request.out);
        fcgi_streambuf cerr_fcgi_streambuf(request.err);
        cin.rdbuf(&cin_fcgi_streambuf);
        cout.rdbuf(&cout_fcgi_streambuf);
        cerr.rdbuf(&cerr_fcgi_streambuf);*/

        char *uri = FCGX_GetParam("DOCUMENT_URI", request.envp),
             *query = FCGX_GetParam("QUERY_STRING", request.envp),
             *cookies = FCGX_GetParam("HTTP_COOKIE", request.envp),
             *filename = FCGX_GetParam("SCRIPT_FILENAME", request.envp),
             *method = FCGX_GetParam("REQUEST_METHOD", request.envp);

        printf("Requested URI: %s\n", uri);
    }

    /*cin.rdbuf(cin_streambuf);
    cout.rdbuf(cout_streambuf);
    cerr.rdbuf(cerr_streambuf);*/

    return NULL;
}
