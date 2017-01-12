#include "TcpSocket.h"
#include "UserManagement.h"
#include <mysql++/mysql++.h>
#include <mysql++/result.h>
#include <signal.h>

using namespace std;

void childHandler(int tmp)
{
    wait(0);
}

int main(int argc,char** argv)  
{
    signal(SIGCHLD,childHandler);
    try
    {
        ServerTcpSocket server(33333);
        while(server.accept()>0)
        {
            pid_t pid;
            if((pid=fork())==0)
            {
                cout<<"connect!"<<endl;
                UserManagement user(server,"chatserver","10.156.145.48","root","shangus1","../log.txt");
                while(1)
                {
                    int uuid=atoi(server.receive(23).c_str());
                    if(uuid==0)
                        user.login();
                    else if(uuid==1)
                        user.join();
                    else if(uuid==2)
                        user.withdraw();
                    else if(uuid==3)
                        user.searchID();
                    else if(uuid==4)
                        user.addFriend();
                    else if(uuid==5)
                        user.myFriendsList();
                    else if(uuid==6)
                        user.deleteFriend();
                    else if(uuid==7)
                        user.makeChatRoom();
                    else if(uuid==8)
                        user.logout();
                    else if(uuid==9)
                        user.myChatList();
                    else if(uuid==10)
                        user.outChatRoom();
                    else if(uuid==11)
                        user.IDoverlapCheck();
                }
                
            }
        }
    } 
    catch (mysqlpp::BadQuery& e) 
    {
        cerr << "BadQuery : " << e.what() << endl;
        cerr<<e.errnum()<<endl;
    }
    catch(const char* e)
    {
        perror(e);
    }
    catch(UserManagement::Flag e)
    {
        if(e==UserManagement::end)
            exit(1);
    }
    return 0;
}
