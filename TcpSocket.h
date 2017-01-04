#ifndef __TCPSOCKET_H__
#define __TCPSOCKET_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <string>

class TcpSocket
{
protected:
    struct SocketData
    {
        int socketDescriptor;
        struct sockaddr_in address;
    };
    SocketData mySocketData;
public:
    TcpSocket(int port) 
    {
        if((mySocketData.socketDescriptor=socket(AF_INET,SOCK_STREAM,0))<0)
            throw "socket";
        memset(&(mySocketData.address),0x00,sizeof(struct sockaddr_in));
        mySocketData.address.sin_family=AF_INET;
        mySocketData.address.sin_port=htons(port);
    }
    ~TcpSocket()
    {
        close(mySocketData.socketDescriptor);
    }
    void send(int& socketDescriptor,std::string msg,int key)
    {
        if(::send(socketDescriptor, cryption(msg,key).c_str(), msg.length()+1,0)<0)
            throw "send";
    }
    std::string receive(int& socketDescriptor,int key)
    {
        std::string ret;
        char tmp;
        int status;
        while(1)
        {
            if((status=recv(socketDescriptor,&tmp,1,0))<0)
                throw "recv";
            else if(status==0)
                throw "recv(connection end)";
            if(tmp=='\0')
                break;
            ret.push_back(tmp);
        }
        return cryption(ret,key);
    }
    std::string cryption(std::string str,int key)
    {
        for(int i=0;i<str.length();i++)
            str.at(i)^=key;
        return str;
    }
};

class ServerTcpSocket : public TcpSocket
{
private:
    SocketData& serverSocketData=mySocketData;
    SocketData clientSocketData;
public:
    ServerTcpSocket(int port=6974) : TcpSocket(port)
    {
        serverSocketData.address.sin_addr.s_addr=htonl(INADDR_ANY);
        
        if(bind(serverSocketData.socketDescriptor,(struct sockaddr*)&(serverSocketData.address),sizeof(struct sockaddr_in))<0)
            throw "bind";
        if(listen(serverSocketData.socketDescriptor,0)<0)
            throw "listen";
    }
    int accept()
    {
        socklen_t addressLength=sizeof(struct sockaddr_in);
        clientSocketData.socketDescriptor=::accept(serverSocketData.socketDescriptor,(struct sockaddr*)&(clientSocketData.address),&addressLength);
        return clientSocketData.socketDescriptor;
    }
    ServerTcpSocket& send(std::string msg,int key=0)
    {
        TcpSocket::send(clientSocketData.socketDescriptor,msg,key);
        return *this;
    }
    std::string receive(int key=0)
    {
        return TcpSocket::receive(clientSocketData.socketDescriptor,key);
    }
    void closeAccept()
    {
        close(clientSocketData.socketDescriptor);
    }
};

class ClientTcpSocket : public TcpSocket
{
private:
    SocketData& clientSocketData=mySocketData;
public:
    ClientTcpSocket(std::string IP="127.0.0.1",int port=6974) : TcpSocket(port)
    {
        clientSocketData.address.sin_addr.s_addr=inet_addr(IP.c_str());

        if(connect(clientSocketData.socketDescriptor,(struct sockaddr*)&clientSocketData.address,sizeof(struct sockaddr_in))<0)
            throw "connect";
    }
    ClientTcpSocket& send(std::string msg,int key=0)
    {
        TcpSocket::send(clientSocketData.socketDescriptor,msg,key);
        return *this;
    }
    std::string receive(int key=0)
    {
        return TcpSocket::receive(clientSocketData.socketDescriptor,key);
    }
};

#endif