#include <cstddef>
#include<iostream>
#include <sstream>
#include<string>
#include<queue>
#include<vector>
#include<thread>
#include<fstream>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"



class Client
{
public:
    Client(std::string name, std::queue<std::string> sequence, unsigned int time, unsigned int priority) : ClientName(name), SequenceOfDepartments(sequence), SpendTime(time), PriorityOfClient(priority){}
    
    //Removing the ability to copy clients
    Client(const Client &) = delete;
    Client operator=(const Client &) = delete;

    std::string ClientName;
    std::queue<std::string> SequenceOfDepartments;
    unsigned int SpendTime;
    unsigned int PriorityOfClient;
    
};


class Department
{
public:
    Department(size_t count, std::string name) : CountEmployees(count), NameOfDepartment(name) 
    {

    }

    //Removing the ability to copy department
    Department(const Department &) = delete;
    Department operator=(const Department &) = delete;

    std::string NameOfDepartment;
    size_t CountEmployees;
    std::priority_queue<Client> QueueOfClients;

};

class Bank
{
public:
    Bank(const Bank &) = delete;
    Bank operator=(const Bank &)=delete;



    std::vector<Department> Departments;
    std::vector<Client> AllClients;

    //Client queue for distribution
    std::priority_queue<Client> DistributedClients;
};

int main()
{
    
}