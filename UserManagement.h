#ifndef __USERMANAGEMENT_H__
#define __USERMANAGEMENT_H__

#include "TcpSocket.h"
#include <mysql++/mysql++.h>
#include <mysql++/result.h>
#include <vector>

class UserManagement        //edit key
{
private:
    const int key=23;
    ServerTcpSocket& server;
    mysqlpp::Connection con;
    mysqlpp::Query query;
    mysqlpp::StoreQueryResult result;
    std::string userID;
public:
    enum Flag{end};
    UserManagement(ServerTcpSocket& _server,
                    std::string DBname,
                    std::string IP,
                    std::string id,
                    std::string pw) : 
                    server(_server) , 
                    con(DBname.c_str(),IP.c_str(),id.c_str(),pw.c_str(),3306) ,
                    query(con.query())
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
    UserManagement& login() //send userdata
    {
        if(userID.length()!=0)  
            throw end;
        std::string id=server.receive(key);
        std::string pw=server.receive(key);
        query<<"select * from userdata where ID='"+id+"' and PW=password('"+pw+"')";
        result=query.store();
        if(result.num_rows()==1)
        {
            userID=std::string(result.at(0)["ID"]);
            server.send("success",key);
        }
        else server.send("fail",key);

        return *this;
    }
    UserManagement& logout()
    {
        if(userID.length()==0)  
            throw end;
        userID="";
        server.send("logout seccess",key);
    }
    UserManagement& IDoverlapCheck()  //
    {
        std::string id=server.receive(key);
        if(id.length()<5||id.length()>20)
            throw "ID length error";
        query<<"select ID from userdata where ID='"+id+"'";
        result=query.store();
        if(result.num_rows()==0)
            server.send("sucecss",key);
        else server.send("ID OverLap!",key); 

        return *this;
    }
    UserManagement& join()  //IDoverlapCheck edit
    {
        if(userID.length()!=0)  
            throw end;
            
        std::string id=server.receive(key);
        std::string pw=server.receive(key);
        std::string name=server.receive(key);
        try
        {
            if(id.length()<5||id.length()>20)
                throw "ID length error";
            if(pw.length()<5||pw.length()>20)
                throw "PW length error";
            query<<"insert into userdata(ID,PW,NAME,_) values('"+id+"',password('"+pw+"'),'"+name+"','"+pw+"')";
            result=query.store();
            server.send("sucecss",key);
        }
        catch(mysqlpp::BadQuery& e)
        {
            if(e.errnum()==1062)
                server.send("ID OverLap!",key);
        }
        catch(const char* e)
        {
            server.send(std::string(e),key);
        }
        std::cout<<"a"<<std::endl;
        return *this;
    }
    UserManagement& withdraw()
    {
        if(userID.length()==0)
            throw end;

        std::string pw=server.receive(key);
        query<<"select * from userdata where ID='"+userID+"' and PW=password('"+pw+"')";
        result=query.store();
        if(result.num_rows()==0)
            server.send("PW different",key);
        else
        {
            query<<"delete from userdata where ID='"+userID+"' and PW=password('"+pw+"')";
            result=query.store();
            server.send("success",key);
        }

        return *this;
    }
    UserManagement& searchID()
    {
        std::string searchID=server.receive(key);
        query<<"select * from userdata where ID regexp '"+searchID+"'";
        result=query.store();
        for(int i=0;i<result.num_rows();i++)
            server.send(std::string(result.at(i)["ID"]),key);
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

        std::vector<std::string> array=list2array(std::string(result.at(0)[0]));
        for(std::string tmp:array)
            server.send(tmp,key);
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

        query<<"insert into roomnumber(name,memberlist) values('"+roomname+"','"+list+"')";
        result=query.store();

        query<<"select number from roomnumber where name='"+roomname+"'";
        result=query.store();
        int roomNumber=result.at(result.num_rows()-1)[0];

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
    UserManagement& outChatRoom()
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
    }
};


#endif