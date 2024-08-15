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
std::mutex queue;
std::string GetTime()
{
   // Получаем текущее время с миллисекундами
    auto now = std::chrono::system_clock::now();
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    // Преобразуем время в формат std::time_t для вывода даты и времени
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm* localTime = std::localtime(&now_time_t);

    // Используем строковый поток для форматирования строки
    std::ostringstream oss;
    oss << (localTime->tm_year + 1900) << '-'
        << std::setw(2) << std::setfill('0') << (localTime->tm_mon + 1) << '-'
        << std::setw(2) << std::setfill('0') << localTime->tm_mday << ' '
        << std::setw(2) << std::setfill('0') << localTime->tm_hour << ':'
        << std::setw(2) << std::setfill('0') << localTime->tm_min << ':'
        << std::setw(2) << std::setfill('0') << localTime->tm_sec << '.'
        << std::setw(3) << std::setfill('0') << milliseconds.count();

    // Возвращаем строку
    return oss.str();
    
}



class Client
{
public:
    Client(std::string name, std::queue<std::string> sequence, unsigned int time, unsigned int priority) : ClientName(name), SequenceOfDepartments(sequence), SpendTime(time), PriorityOfClient(priority){}
    
    

    std::string ClientName;
    std::queue<std::string> SequenceOfDepartments;
    unsigned int SpendTime;
    unsigned int PriorityOfClient;
    
    bool operator < (const Client& otherClient) const
    {
        return PriorityOfClient < otherClient.PriorityOfClient;
    }
    bool operator==(const Client& other) const 
    {
    return (ClientName == other.ClientName && SpendTime == other.SpendTime && PriorityOfClient == other.PriorityOfClient && SequenceOfDepartments == other.SequenceOfDepartments);
    }
};
class Department
{
public:
    Department(size_t count, std::string name) 
        : CountEmployees(count), 
          NameOfDepartment(std::move(name)), 
          ClientsAreNotInQueue(std::make_unique<std::vector<Client>>()) 
    {
        EmployedEmployees=0;
    }
    
    Department(const Department&) = delete;             // Запрещаем копирование
    Department& operator=(const Department&) = delete;  // Запрещаем присваивание

    Department(Department&&) = default;                 // Разрешаем перемещение
    Department& operator=(Department&&) = default;      // Разрешаем присваивание перемещением
    


    void MaintenanceWindow(Client CurrentClient) 
    {
        {
            std::lock_guard<std::mutex> lock(coutMutex);
            EmployedEmployees++;
            std::cout << '[' << GetTime() << "] Client " << CurrentClient.ClientName << " is served by the department "<< CurrentClient.SequenceOfDepartments.front() << "\n";
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(CurrentClient.SpendTime));
        {

            std::lock_guard<std::mutex> lock(coutMutex);
            // Вместо удаления клиента, перемещаем его из ClientsServed в ClientsAreNotInQueue
            //auto it = std::find(clients.begin(), clients.end(), CurrentClient);
            std::cout << '[' << GetTime() << "] Client " << CurrentClient.ClientName << " left the department "<< CurrentClient.SequenceOfDepartments.front() << "\n";
            CurrentClient.SequenceOfDepartments.pop();
            ClientsAreNotInQueue->push_back(CurrentClient);
            EmployedEmployees--;
            //clients.erase(it); // Удаляем из ClientsServed после перемещения 
              
        }
    }

    void ClientService()
    {   
        
        if((EmployedEmployees<CountEmployees))
        {
            for(int i=0; i<(CountEmployees-EmployedEmployees);i++)
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

    std::vector<std::thread> employees;
    std::shared_ptr<std::vector<Client>> ClientsAreNotInQueue;  // Now using unique_ptr for unique ownership
    std::string NameOfDepartment;
    std::priority_queue<Client> QueueOfClients;

private:
    const size_t CountEmployees;
    size_t EmployedEmployees;
    //std::mutex queueMutex;
};


class Bank
{
public:
    //Bank(const Bank &) = delete;
    //Bank& operator=(const Bank &)=delete;

    Bank(std::vector<Client>&& Clients, std::vector<Department>&& Departments_)
    : FreeClients(std::make_shared<std::vector<Client>>(std::move(Clients))), Departments(std::move(Departments_))
    {

        CountOfClients = FreeClients->size();
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

    
    void CustomerDistribution()
    {
        
        auto it = FreeClients->begin();
        while (it != FreeClients->end())
        {
            if (!it->SequenceOfDepartments.empty())
            {
                
                std::cout << '[' << GetTime() << "] Client " << it->ClientName << " came to the department " << it->SequenceOfDepartments.front() << "\n";
                
                NameToDepartment[it->SequenceOfDepartments.front()]->QueueOfClients.push(*it);
                
                // Удаляем клиента из FreeClients, после добавления его в QueueOfClients
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
    std::shared_ptr<std::vector<Client>> FreeClients;
private:
    std::vector<Department> Departments;
    std::map<std::string, Department*> NameToDepartment;
    

    
};

int main(int argc, char *argv[])
{
    
    std::stringstream Data;
    std::ifstream StreamOfData(argv[1]);
    std::string Line;
    while(std::getline(StreamOfData, Line))
    {
        Data<<Line<<"\n";
    }

    std::vector<Client> clients;
    std::vector<Department> departments;

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
            //departments.emplace_back(Value_departments[i]["employees"].GetInt(), Value_departments[i]["name"].GetString());

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