#include "TcpSocket.h"
#include <mysql++/mysql++.h>
#include <mysql++/result.h>
#include <signal.h>
#include <vector>

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
        ServerTcpSocket server(9999);
        while(server.accept()>0)
        {
            pid_t pid;
            if((pid=fork())==0)
            {
                cout<<"connect!"<<endl;
                int roomNumber=atoi(server.receive(23).c_str());
                string userID=server.receive(23);
                
                
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