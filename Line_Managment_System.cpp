#include <cstddef>
#include<iostream>
#include <memory>
#include <sstream>
#include<string>
#include<queue>
#include <utility>
#include<vector>
#include<thread>
#include<fstream>
#include "rapidjson/document.h"
#include <map>
#include<chrono>
#include <ctime>
#include <iomanip>
#include<mutex>
#include <algorithm>

std::mutex coutMutex;


// Function for getting the time
std::string GetTime()
{
   // We get the current time with milliseconds
    auto now = std::chrono::system_clock::now();
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    // Convert the time to the std::time_t format to display the date and time
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm* localTime = std::localtime(&now_time_t);

    // We use a string stream to format a string
    std::ostringstream oss;
    oss << (localTime->tm_year + 1900) << '-'
        << std::setw(2) << std::setfill('0') << (localTime->tm_mon + 1) << '-'
        << std::setw(2) << std::setfill('0') << localTime->tm_mday << ' '
        << std::setw(2) << std::setfill('0') << localTime->tm_hour << ':'
        << std::setw(2) << std::setfill('0') << localTime->tm_min << ':'
        << std::setw(2) << std::setfill('0') << localTime->tm_sec << '.'
        << std::setw(3) << std::setfill('0') << milliseconds.count();

    // return string
    return oss.str();
    
}


// The class for storing information about the client
class Client
{
public:
    Client(std::string name, std::queue<std::string> sequence, unsigned int time, unsigned int priority) : ClientName(name), SequenceOfDepartments(sequence), SpendTime(time), PriorityOfClient(priority){}
    
    
    // Name of client
    std::string ClientName;

    // list of departments that the client should visit
    std::queue<std::string> SequenceOfDepartments;
    // Service Time
    unsigned int SpendTime;
    // Client priority in the service queue 
    unsigned int PriorityOfClient;
    
    // operator overload = for priority_queue
    bool operator < (const Client& otherClient) const
    {
        return PriorityOfClient < otherClient.PriorityOfClient;
    }

    // operator overload for client comparison
    bool operator==(const Client& other) const 
    {
    return (ClientName == other.ClientName && SpendTime == other.SpendTime && PriorityOfClient == other.PriorityOfClient && SequenceOfDepartments == other.SequenceOfDepartments);
    }
};




// The class for servicing the client queue
class Department
{
public:
    Department(size_t count, std::string name) 
        : CountEmployees(count), 
          NameOfDepartment(std::move(name)), 
          ClientsAreNotInQueue(std::make_unique<std::vector<Client>>()) 
    {
        BusyEmployees=0;
    }
    
    Department(const Department&) = delete;             // ban of copying
    Department& operator=(const Department&) = delete;  // ban of assignment

    Department(Department&&) = default;                 // allow movement
    Department& operator=(Department&&) = default;      // allow assignment by movement
    

    // Method in which the employee serves the client
    void MaintenanceWindow(Client CurrentClient) 
    {
        {
            std::lock_guard<std::mutex> lock(coutMutex);
            BusyEmployees++;
            std::cout << '[' << GetTime() << "] Client " << CurrentClient.ClientName << " is served by the department "<< CurrentClient.SequenceOfDepartments.front() << "\n";
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(CurrentClient.SpendTime));
        {

            std::lock_guard<std::mutex> lock(coutMutex);
            std::cout << '[' << GetTime() << "] Client " << CurrentClient.ClientName << " left the department "<< CurrentClient.SequenceOfDepartments.front() << "\n";
            CurrentClient.SequenceOfDepartments.pop();
            ClientsAreNotInQueue->push_back(CurrentClient);
            BusyEmployees--;
              
        }
    }

    void ClientService()
    {   
        
        if((BusyEmployees<CountEmployees))
        {
            for(int i=0; i<(CountEmployees-BusyEmployees);i++)
            {
                if(!QueueOfClients.empty())
                {
                    
                    employees.emplace_back([this](Client client) {MaintenanceWindow(client);}, QueueOfClients.top());
                    QueueOfClients.pop();
                    employees.back().detach();
                }
            }
        }
    }

    // Streams that serve clients
    std::vector<std::thread> employees;

    // pointer to FreeClients
    std::shared_ptr<std::vector<Client>> ClientsAreNotInQueue;  
    
    std::string NameOfDepartment;

    // queue for service by priority
    std::priority_queue<Client> QueueOfClients;

private:

    const size_t CountEmployees;

    size_t BusyEmployees;
};


// The class in which departments serve customers
class Bank
{
public:
    Bank(std::vector<Client>&& Clients, std::vector<Department>&& Departments_)
    : FreeClients(std::make_shared<std::vector<Client>>(std::move(Clients))), Departments(std::move(Departments_))
    {
        CountOfClients = FreeClients->size();

        // Here each department gets access to FreeClients
        for (auto &i : Departments)
        {
            
            if (i.ClientsAreNotInQueue == nullptr) 
            {
                i.ClientsAreNotInQueue = std::make_shared<std::vector<Client>>();
            }
            i.ClientsAreNotInQueue = FreeClients;
            NameToDepartment[i.NameOfDepartment] = &i;
        }
    }

    // distribution of clients by departments
    void CustomerDistribution()
    {
        
        auto it = FreeClients->begin();
        while (it != FreeClients->end())
        {
            if (!it->SequenceOfDepartments.empty())
            {
                
                std::cout << '[' << GetTime() << "] Client " << it->ClientName << " came to the department " << it->SequenceOfDepartments.front() << "\n";
                
                NameToDepartment[it->SequenceOfDepartments.front()]->QueueOfClients.push(*it);
                
                it = FreeClients->erase(it);
            }
            else
            {
                std::cout << '[' << GetTime() << "] Client " << it->ClientName << " left the bank \n";
                it = FreeClients->erase(it);
                CountOfClients--;
            }
        }
    }


    // The main method
    void WorkDayOfBank()
    {
        std::cout<<'['<<GetTime()<<']'<<" Bank opened\n";
        CustomerDistribution();
        std::thread ThreadForDepartments([&](){
        
        while (true) 
        {
            
            for(auto& i : Departments)
            {
                i.ClientService();
        
            
            }
            if (CountOfClients==0) 
            {
                std::cout<<'['<<GetTime()<<']'<<" Bank closed\n";
                break;
            }
            
        }
        
        
        });
        
        ThreadForDepartments.detach();
        while (CountOfClients!=0) 
        {
            CustomerDistribution();
        }
        
    }

    
    unsigned int CountOfClients;

    // clients awaiting their distribution by department
    std::shared_ptr<std::vector<Client>> FreeClients;
private:

    std::vector<Department> Departments;

    // It helps to easily find the right department to send the client to the queue
    std::map<std::string, Department*> NameToDepartment;
    

    
};

int main(int argc, char *argv[])
{
    // check for parameters
    if(argc<2)
    {
        std::cerr<<"no required parameters\n";
        exit(1);
    }

    std::stringstream Data;
    std::ifstream StreamOfData(argv[1]);
    std::string Line;
    while(std::getline(StreamOfData, Line))
    {
        Data<<Line<<"\n";
    }

    std::vector<Client> clients;
    std::vector<Department> departments;


    //Converting JSON data into arrays of clients and departments
    rapidjson::Document d;
    d.Parse(Data.str().data());
    if (d.HasMember("clients") && d["clients"].IsArray())
    {
        rapidjson::Value& Value_departments = d["departments"];
        rapidjson::Value& Value_clients = d["clients"];

        for (rapidjson::SizeType i = 0; i < Value_departments.Size(); i++)
        {
            Department CurrentDepartment(Value_departments[i]["employees"].GetInt(), Value_departments[i]["name"].GetString());
            departments.push_back(std::move(CurrentDepartment));
        }

        for (rapidjson::SizeType i = 0; i < Value_clients.Size(); i++)
        {
            std::queue<std::string> DepartmentsOfClient;
            for (rapidjson::SizeType j = 0; j < Value_clients[i]["departments"].Size(); j++) {
            DepartmentsOfClient.push(Value_clients[i]["departments"][j].GetString());
        }
            Client CurrentClient(Value_clients[i]["name"].GetString(), DepartmentsOfClient ,Value_clients[i]["time"].GetInt(), Value_clients[i]["priority"].GetInt());
            clients.push_back(CurrentClient);
            
        }
    }

    
    Bank TheBank(std::move(clients), std::move(departments));
    
    TheBank.WorkDayOfBank();
    

    return 0;
}