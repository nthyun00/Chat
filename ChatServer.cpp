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

int main(int argc,char** argv)  //
{
    signal(SIGCHLD,childHandler);
    try
    {
        ServerTcpSocket server;
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
                    if(uuid==0)//login
                        user.login();
                    else if(uuid==1)//add user
                        user.join();
                    else if(uuid==2)//delete user   //edit need //secure
                        user.withdraw();
                    else if(uuid==3)//search ID -> send List
                        user.searchID();
                    else if(uuid==4)//add friends   //edit need //nothing account
                        user.addFriend();
                    else if(uuid==5)//friends list ->vector ->send
                        user.myFriendsList();
                    else if(uuid==6)//delete friend
                        user.deleteFriend();
                    else if(uuid==7)//make chatroom
                        user.makeChatRoom();
                    else if(uuid==8)
                        user.logout();
                    else if(uuid==9)
                        user.myChatList();
                    else if(uuid==10)
                        user.outChatRoom();
                    else if(uuid==11)
                        user.IDoverlapCheck();
                    /*else //echo
                    {
                        pid_t pid1=fork();
                        string tmp;
                        while(1)
                        {
                            if(pid1>0)
                            {
                                tmp=server.receive(key);
                                cout<<tmp<<endl;
                            }
                            else
                            {
                                cin>>tmp;
                                server.send(tmp,key);
                            }
                        }
                    }*/
                }
                
            }
            //wait(0);
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