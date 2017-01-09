#ifndef __USERMANAGEMENT_H__
#define __USERMANAGEMENT_H__

#include "TcpSocket.h"
#include <mysql++/mysql++.h>
#include <mysql++/result.h>
#include <vector>
#include <stdlib.h>
#include <time.h>
#include <fstream>

class UserManagement        //edit key
{
private:
    const int key=23;
    ServerTcpSocket& server;
    mysqlpp::Connection con;
    mysqlpp::Query query;
    mysqlpp::StoreQueryResult result;
    std::string userID;
    std::string logFile;
public:
    enum Flag{end};
    UserManagement(ServerTcpSocket& _server,
                    std::string DBname,
                    std::string IP,
                    std::string id,
                    std::string pw,
                    std::string logFile) : 
                    server(_server) , 
                    con(DBname.c_str(),IP.c_str(),id.c_str(),pw.c_str(),3306) ,
                    query(con.query()) ,
                    logFile(logFile)
    {
        //con.connect(DBname.c_str(),IP.c_str(),id.c_str(),pw.c_str(),3306);
        //query=con.query();
    }
    static std::vector<std::string> list2array(std::string list,char plot=';')
    {
        std::string tmp;
        std::vector<std::string> ret;
        for(int i=0;i<list.length();i++)
        {
            if(list.at(i)==plot)
            {
                ret.push_back(tmp);
                tmp="";
            }

            else tmp.push_back(list.at(i));
        }
        return ret;
    }
    void writeLog(std::string param)
    {
        std::ofstream log(logFile,std::ios::app);
        time_t nowTime=time(NULL);
        std::string timeString(ctime(&nowTime));
        log<<timeString.substr(0,timeString.length()-1)<<" : "<<param<<std::endl;
        log.close();
    }
    UserManagement& login() //send userdata
    {
        if(userID.length()!=0)  
            throw end;
        std::string id=server.receive(key);
        std::string pw=server.receive(key);
        writeLog("login request ID("+id+")");
        query<<"select * from userdata where ID='"+id+"' and PW=password('"+pw+"')";
        result=query.store();
        if(result.num_rows()==1)
        {
            userID=std::string(result.at(0)["ID"]);
            server.send(std::string(result.at(0)["NAME"]),key);
            writeLog("login success ID("+userID+") ");
        }
        else 
        {
            server.send("fail",key);
            writeLog("login fail ID("+id+") ");
        }
        return *this;
    }
    UserManagement& logout()
    {
        writeLog("logout request ID("+userID+")");
        if(userID.length()==0)  
            throw end;
        userID="";
        server.send("logout seccess",key);
        writeLog("logout success ID("+userID+")");
    }
    UserManagement& IDoverlapCheck()  //
    {
        std::string id=server.receive(key);

        writeLog("request ID overlap check ID("+id+")");

        if(id.length()<5||id.length()>20)
            throw "ID length error";
        query<<"select ID from userdata where ID='"+id+"'";
        result=query.store();
        if(result.num_rows()==0)
        {
            server.send("success",key);
            writeLog("ID overlap check success ID("+id+") ");
        }
        else
        { 
            server.send("ID OverLap!",key); 
            writeLog("ID overlap check fail ID("+id+") ");
        }

        return *this;
    }
    UserManagement& join()  //IDoverlapCheck edit
    {
        if(userID.length()!=0)  
            throw end;
            
        std::string id=server.receive(key);
        std::string pw=server.receive(key);
        std::string name=server.receive(key);
        writeLog("join request ID("+id+")");
        try
        {
            if(id.length()<5||id.length()>20||pw.length()<5||pw.length()>20)
                throw end;
            query<<"insert into userdata(ID,PW,NAME,_) values('"+id+"',password('"+pw+"'),'"+name+"','"+pw+"')";
            result=query.store();
            server.send("success",key);
            writeLog("join success ID("+id+")");
        }
        catch(mysqlpp::BadQuery& e)
        {
            if(e.errnum()==1062)    //ID OverLap
                throw end;
                //server.send("ID OverLap!",key);
        }
        std::cout<<"a"<<std::endl;
        return *this;
    }
    UserManagement& withdraw()
    {
        if(userID.length()==0)
            throw end;

        std::string pw=server.receive(key);
        writeLog("withdraw request ID("+userID+")");
        query<<"select * from userdata where ID='"+userID+"' and PW=password('"+pw+"')";
        result=query.store();
        if(result.num_rows()==0)
        {
            server.send("PW different",key);
            writeLog("withdraw fail ID("+userID+")");
        }
        else
        {
            query<<"delete from userdata where ID='"+userID+"' and PW=password('"+pw+"')";
            result=query.store();
            userID="";
            server.send("success",key);
            writeLog("withdraw success ID("+userID+")");
        }

        return *this;
    }
    UserManagement& searchID()
    {
        std::string searchID=server.receive(key);
        query<<"select * from userdata where ID regexp '"+searchID+"'";
        result=query.store();
        for(int i=0;i<result.num_rows();i++)
        {
            server.send(std::string(result.at(i)["ID"]),key);
            server.send(std::string(result.at(i)["NAME"]),key);
        }
        server.send("end",key); //end flag //edit need

        return *this;
    }
    UserManagement& addFriend()
    {
        if(userID.length()==0)  
            throw end;

        std::string friendsID=server.receive(key);
        query<<"select friendsList from userdata where ID='"+userID+"'";
        result=query.store();
        std::vector<std::string> array=list2array(std::string(result.at(0)[0]));
        array.push_back(friendsID);

        std::string list;
        for(std::string tmp:array)
            list+=tmp+";";
        query<<"update userdata set friendsList='"+list+"' where ID='"+userID+"'";
        result=query.store();

        return *this;
    }
    UserManagement& myFriendsList() //send ID //edit need name send
    {
        if(userID.length()==0)  
            throw end;

        query<<"select friendsList from userdata where ID='"+userID+"'";
        result=query.store();

        std::vector<std::string> array=list2array(std::string(result.at(0)["friendsList"]));
        for(std::string tmp:array)
        {
            query<<"select NAME from userdata where ID='"+tmp+"'";
            result=query.store();

            server.send(tmp,key);
            server.send(std::string(result.at(0)["NAME"]),key);
        }
        server.send("end",key); //end flag //need edit

        return *this;
    }
    UserManagement& deleteFriend()
    {
        if(userID.length()==0)  
            throw end;

        std::string friendsID=server.receive(key);
        query<<"select friendsList from userdata where ID='"+userID+"'";
        result=query.store();
        std::vector<std::string> array=list2array(std::string(result.at(0)[0]));
        
        std::string list;
        for(std::string tmp:array)
        {
            if(tmp==friendsID)
                continue;
            list+=tmp+";";
        }
        query<<"update userdata set friendsList='"+list+"' where ID='"+userID+"'";
        result=query.store();
    }
    UserManagement& makeChatRoom()
    {
        if(userID.length()==0)  
            throw end;

        std::string roomname;
        std::vector<std::string> invite;  
        roomname=server.receive(key);
        std::string tmp;
        invite.push_back(userID);
        while(1)
        {
            tmp=server.receive(key);
            if(tmp=="end")//end flag //need edit
                break;
            invite.push_back(tmp);
        }
        std::string list;
        for(std::string tmp:invite)
        list+=tmp+";";

        srand(time(NULL)*getpid());
        int checkValue=rand()%99999+1;
        query<<"insert into roomnumber (name,memberlist,`check`) values ('"+roomname+"','"+list+"','"+std::to_string(checkValue)+"')";
        query.store();

        query<<"select number from roomnumber where `check`='"+std::to_string(checkValue)+"'";
        result=query.store();
        int roomNumber=result.at(result.num_rows()-1)["number"];

        query<<"update roomnumber set `check`='0' where `check`='"+std::to_string(checkValue)+"'";
        result=query.store();

        for(std::string tmp:invite)
        {
            query<<"select chatList from userdata where ID='"+tmp+"'";
            result=query.store();

            std::vector<std::string> array=list2array(std::string(result.at(0)[0]));
            array.push_back(std::to_string(roomNumber));

            std::string list1;
            for(std::string tmp1:array)
                list1+=tmp1+";";
            query<<"update userdata set chatList='"+list1+"' where ID='"+tmp+"'";
            result=query.store();
        }
        char buf[10];
        sprintf(buf,"%d",roomNumber);
        query<<"create table chatroom.room"+std::string(buf)+" (number int primary key auto_increment ,date timestamp not null,sender varchar(20) not null,msg varchar(200) not null)";
        query.store();

        //trigger
        /*query<<"delimiter !";
        query.store();
        query<<"create trigger test after update on roomtrigger"+roomNumber+" for each row begin set";*/

        return *this;
    }
    UserManagement& myChatList()    //send num //edit need name send
    {
        if(userID.length()==0)  
            throw end;

        query<<"select chatList from userdata where ID='"+userID+"'";
        result=query.store();

        std::vector<std::string> array=list2array(std::string(result.at(0)[0]));
        for(std::string tmp:array)
            server.send(tmp,key);
        server.send("end",key); //end flag //need edit
    }
    UserManagement& outChatRoom()   //T.T need edit
    {
        if(userID.length()==0)  
            throw end;

        std::string roomNumber=server.receive(key);

        //userdata update
        query<<"select chatList from userdata where ID='"+userID+"'";
        result=query.store();
        std::vector<std::string> array=list2array(std::string(result.at(0)[0]));
        std::string list;
        for(std::string tmp:array)
        {
            if(tmp==roomNumber)
                continue;
            list+=tmp+";";
        }
        query<<"update userdata set chatList='"+list+"' where ID='"+userID+"'";
        result=query.store();

        //roomnumber update
        query<<"select memberList from roomnumber where number='"+roomNumber+"'";
        result=query.store();
        array=list2array(std::string(result.at(0)[0]));
        /*if(array.size()==1)
        {
            query<<"delete from roomnumber where number='"+roomNumber+"'";
            query.store();
            query<<"drop table chatroom.room"+roomNumber;
            query.store();
        }*/

        list.clear();
        for(std::string tmp:array)
        {
            if(tmp==userID)
                continue;
            list+=tmp+";";
        }
        query<<"update roomnumber set chatList='"+list+"' where number='"+roomNumber+"'";
        result=query.store();

        //add need  send signal to chating server

        return *this;
    }
};


#endif