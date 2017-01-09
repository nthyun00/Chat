#include "TcpSocket.h"
#include <mysql++/mysql++.h>
#include <mysql++/result.h>
#include <signal.h>
#include <vector>

using namespace std;

const int key=23;

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
                string userID=server.receive(23);
                int roomNumber=atoi(server.receive(23).c_str());
        
                mysqlpp::Connection con("chatroom","10.156.145.48","root","shangus1",3306);
                mysqlpp::Query query=con.query();
                mysqlpp::StoreQueryResult result;

                query<<"select NAME from room"+to_string(roomNumber)+" where ID="+userID;
                result=query.store();
                string userName=result.at(0)["NAME"];

                {//text receive
                    string msg=server.receive(key);
                    query<<"insert into room"+to_string(roomNumber)+"(sender,msg) values('"+userID+"','"+msg+"')";
                    query.store();
                }
                {//text alarm

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
    return 0;
}