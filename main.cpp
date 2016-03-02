#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <fcntl.h>
#include <errno.h>
#include <ev.h>
#include <string>
#include <fstream>

using namespace std;

// Example from the internet
void accept_connection(EV_P_ struct ev_io *w, int revents);
void read_connection(EV_P_ struct ev_io *w, int revents);

// Example from the course
void accept_cb(struct ev_loop *loop, struct ev_io *watcher, int revents);
void read_cb(struct ev_loop *loop, struct ev_io *watcher, int revents);

void http_GET(std::string request, struct ev_io *watcher, ssize_t r);


std::string home_dir;

int main(int argc, char **argv)
{
    int ip_idx;
    int port_idx;
    int dir_idx;
    for(int i = 0; i < argc; i++){
        if (argv[i][1] == 'p') {
            port_idx = i + 1;
        }

        if (argv[i][1] == 'h')
            ip_idx = i+1;

        if (argv[i][1] == 'd')
            dir_idx = i+1;
    }

    home_dir = argv[dir_idx];
    home_dir.erase(0,1); // костыль связанный с относительным путем



    // Create server socket
    int server_socket = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(server_socket == -1) {
        std::cout << "Failed to create socket" << std::endl;
        return 0;
    }

    struct ev_loop *loop = ev_default_loop(EVBACKEND_EPOLL);
    //struct ev_loop *loop = ev_default_loop(0);

    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[port_idx]));
    addr.sin_addr.s_addr = inet_addr(argv[ip_idx]);


    if(bind(server_socket, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        std::cout << "Failed bind" << endl;
        return 0;
    }

    if(listen(server_socket, SOMAXCONN) != 0) {
        std::cout << "Failed listen" << endl;
        return 0;
    }

    struct ev_io w_accept;
    ev_io_init(&w_accept, accept_cb, server_socket, EV_READ);
    ev_io_start(loop, &w_accept);

    int idDemon = -1;
    if(idDemon = fork()) {

    } else {
        //запустили Демона потеряли над процессом управление
        while(1) ev_loop(loop, 0);
    }

    return 0;
}

///////////////////////////////////////////////
// Example from the course
//////////////////////////////////////////////
/*
 * callback подтверждения соединения
 */
void accept_cb(struct ev_loop *loop, struct ev_io *watcher, int revents)
{
    int client_sd = accept(watcher->fd, 0, 0);
    struct ev_io *w_client = (struct ev_io*)malloc(sizeof(struct ev_io));

    ev_io_init(w_client, read_cb, client_sd, EV_READ);
    ev_io_start(loop, w_client);
}

/*
 * callback чтения
 */
void read_cb(struct ev_loop *loop, struct ev_io *watcher, int revents)
{
    char buffer[1024];
    ssize_t r = recv(watcher->fd, buffer, 1024, MSG_NOSIGNAL);
    if(r < 0) {
        return;
    } else if(r == 0) {
        ev_io_stop(loop, watcher);
        free(watcher);
        return;
    } else { // обработка запроса с клиента
        std::string str(buffer);

        http_GET(str, watcher, r);
        //cout << str << std::endl;
        //send(watcher->fd, buffer, r, MSG_NOSIGNAL);
        // TODO обработка запроса с сервера
    }
}

void http_GET(std::string request, struct ev_io *watcher, ssize_t r)
{
    //cout << "in http_get" << endl;
    int fs = request.find("/", 0);

    int i = fs;
    while ((request[i] != ' ') && (request[i] != '?')) i++;

    std::string fileName = request.substr(fs , i - fs);

    fileName = home_dir + fileName;

    std::string response("");

    cout << fileName << endl;
    ifstream file(fileName.c_str(), std::ifstream::binary);

    if (!file.is_open()){
        cout << "file is not open" << endl;

        response.append("HTTP/1.0 404 NOT FOUND\r\nContent-Type: text/html\r\n\r\n");

        send(watcher->fd, response.c_str() , response.size(), MSG_NOSIGNAL);
    } else {
        cout << "file is open" << endl;


        response.append("HTTP/1.0 200 OK\r\n");
        response.append("Content-type: text/html\r\n\r\n");

        file.seekg (0, file.end);
        int length = file.tellg();
        file.seekg (0, file.beg);

        char * buffer = new char [length];

        file.read (buffer,length);
        send(watcher->fd, response.c_str() , response.size(), MSG_NOSIGNAL);
        send(watcher->fd, buffer , length, MSG_NOSIGNAL);

        delete [] buffer;
    }
    file.close();
}

///////////////////////////////////////////////
// Example from the internet
//////////////////////////////////////////////
/*
 * Функция, которую вызываем когда сокет переключается на приём.
 */
//void read_connection(EV_P_ struct ev_io *w, int revents)
//{
//    int size, buf_size = 1024;
//    char buf[1024];
//    size = read(w->fd, buf, buf_size);
//    printf("read message - '%s' from fd #%i\n", buf, w->fd);
//
///* Если размер пришедшего пакета <= 0 - отрубаем. */
//    if (size <= 0) {
//        if( size == -1 && errno == EAGAIN )
//            printf("\t EAGAIN\n");
//
//        fds[(long)w->fd] = 0;
//        ev_io_stop(loop, w);
//        close(w->fd);
//        free(w);
//        printf("\t -> closed connection (fd %i)\n", w->fd);
//
//        return;
//    }
//
//    write(w->fd, "Hi\n", strlen("Hi\n")); // Отправка пакета
//}
//
///*
// * Функция, вызываемая при инициализации соединения.
// */
//void accept_connection(EV_P_ struct ev_io *w, int revents)
//{
//    printf("accept connection from fd #%i\n", w->fd);
//    struct ev_io *io = (ev_io *)malloc(sizeof(struct ev_io));
//    struct sockaddr sa;
//    socklen_t sizeof_sa = sizeof(sa);
//    long fd = accept(w->fd, &sa, &sizeof_sa);
//    if (fd <= 0) return;
//
//    fds[fd] = 1; // Делаем пометку что клиент подключен.
//    printf("fds[%ld] = 1;\n", fd);
//
//    if (fd > maxfd)
//        maxfd = fd;
//
//    fcntl(fd, F_SETFL, O_NONBLOCK);  // Превращаем сокет fd в неблокирующий.
//    ev_io_init(io, read_connection, fd, EV_READ);
//    ev_io_start(loop, io);
//}

