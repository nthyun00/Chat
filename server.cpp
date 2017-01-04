#include "TcpSocket.h"
#include <wait.h>

int main()
{
    try
    {
        ServerTcpSocket a;
        while(a.accept()>0)
        {
            pid_t pid;
            if((pid=fork())==0)
            {
                std::cout<<"connect"<<std::endl;
                //while(1)
                //{
                std::cout<<a.receive(23)<<std::endl;
                std::cout<<a.receive(23);
                //}
                std::cout<<"connect end"<<std::endl;
                a.closeAccept();
                exit(1);
            }
            wait(0);
        }
    }
    catch(const char* e)
    {
        perror(e);
    }
    return 0;
}