#ifndef __USERMANAGEMENT_H__
#define __USERMANAGEMENT_H__

#include "TcpSocket.h"
#include <mysql++/mysql++.h>
#include <mysql++/result.h>
#include <vector>
#include <algorithm>
#include <stdlib.h>
#include <time.h>
#include <fstream>

class UserManagement       
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
    static int specialCharCheck(std::string param,std::string plot="")
    {
        for(char tmp:param)
            for(char plotTmp:plot)
                if(tmp==plotTmp)
                    return false;
        return true;
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
        std::string tmp=userID;
        writeLog("logout request ID("+userID+")");
        if(userID.length()==0)  
            throw end;
        userID="";
        writeLog("logout success ID("+tmp+")");
    }
    UserManagement& IDoverlapCheck()  
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
    UserManagement& join()
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
            if(specialCharCheck(id,""))
            query<<"insert into userdata(ID,PW,NAME,_) values('"+id+"',password('"+pw+"'),'"+name+"','"+pw+"')";
            result=query.store();
            server.send("success",key);
            writeLog("join success ID("+id+")");
        }
        catch(mysqlpp::BadQuery& e)
        {
            if(e.errnum()==1062)    
                throw end;
        }
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
            std::vector<std::string> friendArray=list2array(std::string(result.at(0)["friendsList"]));
            for(std::string friendID: friendArray)
            {
                query<<"select friendsList from userdata where ID='"+friendID+"'";
                result=query.store();
                std::vector<std::string> array=list2array(std::string(result.at(0)["friendsList"]));
                std::string list;
                for(std::string tmp:array)
                    if(tmp!=userID)
                        list+=tmp+";";
                query<<"update userdata set friendsList='"+list+"' where ID='"+friendID+"'";
                query.store();
            }

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
        writeLog("searchID request ID("+userID+") friendID("+searchID+")");
        query<<"select * from userdata where ID = '"+searchID+"'"; //regexp
        result=query.store();

        if(result.num_rows()==1)
        {
            server.send(std::string(result.at(0)["ID"]),key);
            server.send(std::string(result.at(0)["NAME"]),key);
            writeLog("searchID success ID("+userID+") friendID("+searchID+")");
        }
        else
        {
            server.send("not Find!",key);
            writeLog("searchID fail ID("+userID+") friendID("+searchID+")");
        }

        return *this;
    }
    UserManagement& addFriend() 
    {
        if(userID.length()==0)  
            throw end;

        std::string friendsID=server.receive(key);
        writeLog("addFriend request ID("+userID+") friendID("+friendsID+")");

        query<<"select friendsList from userdata where ID='"+userID+"'";
        result=query.store();

        std::vector<std::string> array=list2array(std::string(result.at(0)["friendsList"]));
        array.push_back(friendsID);
        std::sort(array.begin(),array.end());
        std::unique(array.begin(),array.end());

        std::string list;
        for(std::string tmp:array)
            list+=tmp+";";
        query<<"update userdata set friendsList='"+list+"' where ID='"+userID+"'";
        result=query.store();
        

        query<<"select friendsList from userdata where ID='"+friendsID+"'";
        result=query.store();

        array.clear();
        array=list2array(std::string(result.at(0)["friendsList"]));
        array.push_back(userID);
        std::sort(array.begin(),array.end());
        std::unique(array.begin(),array.end());

        list="";
        for(std::string tmp:array)
            list+=tmp+";";
        query<<"update userdata set friendsList='"+list+"' where ID='"+friendsID+"'";
        result=query.store();

        writeLog("addFriend success ID("+userID+") friendID("+friendsID+")");

        return *this;
    }
    UserManagement& myFriendsList()
    {
        if(userID.length()==0)  
            throw end;
        
        writeLog("friendsList request ID("+userID+")");

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
        server.send("end!",key); 
        writeLog("friendsList success ID("+userID+")");
        return *this;
    }
    UserManagement& deleteFriend()
    {
        if(userID.length()==0)  
            throw end;

        std::string friendsID=server.receive(key);
        writeLog("deleteFriend request ID("+userID+") friendID("+friendsID+")");

        query<<"select friendsList from userdata where ID='"+userID+"'";
        result=query.store();
        std::vector<std::string> array=list2array(std::string(result.at(0)[0]));
        
        std::string list;
        for(std::string tmp:array)
            if(tmp!=friendsID)
                list+=tmp+";";

        query<<"update userdata set friendsList='"+list+"' where ID='"+userID+"'";
        result=query.store();
        writeLog("deleteFriend success ID("+userID+") friendID("+friendsID+")");

        return *this;
    }
    UserManagement& makeChatRoom()
    {
        if(userID.length()==0)  
            throw end;

        std::string roomname;
        std::vector<std::string> invite;  
        roomname=server.receive(key);

        writeLog("makeChatRoom request ID("+userID+")");


        std::string tmp;
        invite.push_back(userID);
        while(1)
        {
            tmp=server.receive(key);
            if(tmp=="end!")
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
            std::sort(array.begin(),array.end());
            std::unique(array.begin(),array.end());

            std::string list1;
            for(std::string tmp1:array)
                list1+=tmp1+";";
            query<<"update userdata set chatList='"+list1+"' where ID='"+tmp+"'";
            result=query.store();
        }
        query<<"create table chatroom.room"+std::to_string(roomNumber)+" (number int primary key auto_increment ,date timestamp not null,sender varchar(20) not null,msg varchar(200) not null)";
        query.store();

        writeLog("makeChatRoom success ID("+userID+")");

        return *this;
    }
    UserManagement& myChatList()    
    {
        if(userID.length()==0)  
            throw end;

        writeLog("chatList request ID("+userID+")");

        query<<"select chatList from userdata where ID='"+userID+"'";
        result=query.store();

        std::vector<std::string> array=list2array(std::string(result.at(0)["chatList"]));
        for(std::string tmp:array)
        {
            query<<"select name from roomnumber where number='"+tmp+"'";
            result=query.store();

            server.send(tmp,key);
            server.send(std::string(result.at(0)["name"]),key);
        }
        server.send("end!",key); 

        writeLog("chatList success ID("+userID+")");
        return *this;
    }
    UserManagement& outChatRoom()  
    {
        if(userID.length()==0)  
            throw end;

        std::string roomNumber=server.receive(key);

        writeLog("outChatRoom request ID("+userID+") roomNumber("+roomNumber+")");

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

        list.clear();
        for(std::string tmp:array)
        {
            if(tmp==userID)
                continue;
            list+=tmp+";";
        }
        query<<"update roomnumber set chatList='"+list+"' where number='"+roomNumber+"'";
        result=query.store();
        
        writeLog("outChatRoom success ID("+userID+") roomNumber("+roomNumber+")");

        return *this;
    }
};


#endif