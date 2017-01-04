#include <mysql++/mysql++.h>
#include <mysql++/result.h>
#include <iostream>

using namespace std;
int main(int argc, char *argv[])
{
	mysqlpp::Connection con("chatserver","10.156.145.48","root",argv[1]);
	mysqlpp::Query query = con.query(); 
	query << "select * from userdata";
	mysqlpp::StoreQueryResult res=query.store();
	if(res)
	{
		mysqlpp::Row row;
		mysqlpp::Row::size_type size=res.num_rows();
		for(int i=0; i<size ; i++) 
			cout<< res.at(i)["ID"]<<' '<< res.at(i)["password"]<<' '<< res.at(i)["name"] <<endl;
	}
	return 0;
}

