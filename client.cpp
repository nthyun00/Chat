#include "TcpSocket.h"
#include <string>
#include <vector>
using namespace std;

void login(ClientTcpSocket& client,string id,string pw)
{
    client.send("0",23);
    client.send(id,23);
    client.send(pw,23);
    cout<<client.receive(23)<<endl;
}
void adduser(ClientTcpSocket& client,string id,string pw,string name)
{
    client.send("1",23);
    client.send(id,23);
    client.send(pw,23);
    client.send(name,23);
    cout<<client.receive(23)<<endl;
}
void deleteuser(ClientTcpSocket& client,string id,string pw)
{
    client.send("2",23);
    client.send(id,23);
    client.send(pw,23);
    cout<<client.receive(23)<<endl;
}
void addFriend(ClientTcpSocket& client,string name)
{
    client.send("4",23);
    client.send(name,23);
}
void printFriendList(ClientTcpSocket& client)
{
    client.send("5",23);
    while(1)
    {
        string tmp;
        tmp=client.receive(23);
        if(tmp=="end")
            break;
        cout<<tmp<<endl;
    }
}
void deleteFriend(ClientTcpSocket& client,string name)
{
    client.send("6",23);
    client.send(name,23);
}
void makeChatRoom(ClientTcpSocket& client,string roomName,vector<string> invite)
{
    client.send("7",23);
    client.send(roomName,23);
    for(string tmp:invite)
        client.send(tmp,23);
    client.send("end",23);
}
void echo(ClientTcpSocket& client)
{
    client.send("9999",23);
    pid_t pid=fork();
    string tmp;
    while(1)
    {
        if(pid>0)
        {
            tmp=client.receive(23);
            cout<<tmp<<endl;
        }
        else
        {
            cin>>tmp;
            client.send(tmp,23);
        }
    }
}
int main(int argc,char** argv)
{
    try
    {
        ClientTcpSocket* client=new ClientTcpSocket;
        /*adduser(*client,"afwes","afdsafsd","adsfd");
        adduser(*client,"shangus","fewaaef","a");
        adduser(*client,"gwear","greagewa","sh");*/


        login(*client,"a","b");
        vector<string> a;
        a.push_back("afwes");
        a.push_back("shangus");
        
        makeChatRoom(*client,"ro1h",a);

        //printFriendList(*client);
        //addFriend(*client,"afwes");
        //deleteuser(client,argv[1],argv[2]);
        //echo(*client);
        delete client;
    }
    catch(const char* e)
    {
        perror(e);
    }
    return 0;
}
