#include "TcpSocket.h"
#include "UserManagement.h"
#include <mysql++/mysql++.h>
#include <mysql++/result.h>
#include <signal.h>
#include <vector>
#include <thread>
#include <fstream>

using namespace std;

const int key=23;


void childHandler(int tmp)
{
    /*int status;
    pid_t pid;

    while((pid=waitpid(-1,&status,WNOHANG))>0)
        ;*/
    wait(0);
}
void writeLog(string param,string logFile="../chatinglog.txt")
{
    ofstream log(logFile,ios::app);
    time_t nowTime=time(NULL);
    string timeString(ctime(&nowTime));
    log<<timeString.substr(0,timeString.length()-1)<<" : "<<param<<std::endl;
    log.close();
}
/*
{
    while(1)
                    {
                        string msg=server.receive(key);

                        writeLog("send room("+to_string(roomNumber)+") ID("+userID+")");

                        query<<"insert into room"+to_string(roomNumber)+"(sender,msg) values('"+userID+"','"+msg+"')";
                        query.store();

                        query<<"update chatserver.roomnumber set nowmsg=nowmsg+1 where number="+to_string(roomNumber)+"";
                        query.store();
                    }
}
{
int nowmsg;
                    query<<"select nowmsg from chatserver.roomnumber where number='"+to_string(roomNumber)+"'";
                    result=query.store();
                    nowmsg=result.at(0)["nowmsg"];

                    query<<"select * from room"+to_string(roomNumber);
                    result=query.store();
                    for(int i=0;i<result.num_rows();i++)
                    {
                        server.send(string(result.at(i)["date"]),key);
                        server.send(string(result.at(i)["sender"]),key);
                        server.send(string(result.at(i)["msg"]),key);
                    }


                    while(1)
                    {
                        query<<"select nowmsg from chatserver.roomnumber where number='"+to_string(roomNumber)+"'";
                        result=query.store();

                        if(nowmsg!=atoi(result.at(0)["nowmsg"]))
                        {
                            writeLog("receive room("+to_string(roomNumber)+") ID("+userID+")");
                            nowmsg++;
                            query<<"select * from room"+to_string(roomNumber)+" where number='"+to_string(nowmsg)+"'";
                            result=query.store();
                            if(result.at(0)["sender"]!=userID)
                            {
                                server.send(string(result.at(0)["date"]),key);
                                server.send(string(result.at(0)["sender"]),key);
                                server.send(string(result.at(0)["msg"]),key);
                            }
                        }
                        sleep(0.1);
                    }
}
*/
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
                cout<<"connectRoom!"<<endl;
                string userID=server.receive(key);
                int roomNumber=atoi(server.receive(key).c_str());
                writeLog("connect room("+to_string(roomNumber)+") ID("+userID+")");
        
                mysqlpp::Connection con("chatroom","10.156.145.48","root","shangus1",3306);
                mysqlpp::Query query=con.query();
                mysqlpp::StoreQueryResult result;

                query<<"select * from chatserver.roomnumber where number='"+to_string(roomNumber)+"'";
                result=query.store();
                vector<string> array=UserManagement::list2array(string(result.at(0)["memberlist"]));
                
                bool check=false;
                for(string tmp:array)
                    if(tmp==userID)
                    {
                        check=true;
                        break;
                    }
                
                if(check==false)
                    exit(0);

                writeLog("connect success room("+to_string(roomNumber)+") ID("+userID+")");


                thread recv([&](){
                    while(1)
                    {
                        string msg=server.receive(key);

                        writeLog("send room("+to_string(roomNumber)+") ID("+userID+")");

                        query<<"insert into room"+to_string(roomNumber)+"(sender,msg) values('"+userID+"','"+msg+"')";
                        query.store();

                        query<<"update chatserver.roomnumber set nowmsg=nowmsg+1 where number="+to_string(roomNumber)+"";
                        query.store();
                    }
                });

                thread send([&](){
                    int nowmsg;
                    query<<"select nowmsg from chatserver.roomnumber where number='"+to_string(roomNumber)+"'";
                    result=query.store();
                    nowmsg=result.at(0)["nowmsg"];

                    query<<"select * from room"+to_string(roomNumber);
                    result=query.store();
                    for(int i=0;i<result.num_rows();i++)
                    {
                        server.send(string(result.at(i)["date"]),key);
                        server.send(string(result.at(i)["sender"]),key);
                        server.send(string(result.at(i)["msg"]),key);
                    }


                    while(1)
                    {
                        query<<"select nowmsg from chatserver.roomnumber where number='"+to_string(roomNumber)+"'";
                        result=query.store();

                        if(nowmsg!=atoi(result.at(0)["nowmsg"]))
                        {
                            writeLog("receive room("+to_string(roomNumber)+") ID("+userID+")");
                            nowmsg++;
                            query<<"select * from room"+to_string(roomNumber)+" where number='"+to_string(nowmsg)+"'";
                            result=query.store();
                            if(result.at(0)["sender"]!=userID)
                            {
                                server.send(string(result.at(0)["date"]),key);
                                server.send(string(result.at(0)["sender"]),key);
                                server.send(string(result.at(0)["msg"]),key);
                            }
                        }
                        sleep(0.1);
                    }
                });

                send.join();
                recv.join();
                //error
                /*query<<"select NAME from room"+to_string(roomNumber)+" where ID="+userID;
                result=query.store();
                string userName=result.at(0)["NAME"];*/

                //thread recv(msgReceive,server,query,result);
                //thread send(msgAlarm,server,query,result);
                
            }
        }
    } 
    catch (mysqlpp::BadQuery& e) 
    {
        cerr << "BadQuery : " << e.what() << endl;
        cerr<<e.errnum()<<endl;
        cout<<"room"<<endl;
    }
    catch(const char* e)
    {
        perror(e);
        cout<<"room"<<endl;
    }
    return 0;
}