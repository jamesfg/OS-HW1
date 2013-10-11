//
//  main.cpp
//  OS HW1
//
//  Created by James Garcia on 10/3/13.
//  Copyright (c) 2013 James Garcia. All rights reserved.
//

#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <cstring>

using namespace std;

struct process
{
    int numProcess; // Number of processes
    int numResource; // Number of resources
    int timeElapsed;
    int *available; // available[resource] = assigned number of instances
    int **max; // max[process+1][resource+1] = max number of instances
    int *deadline; // deadline for numProcess
    int *computationTime; //computation time for numProcess
    int **allocated;
    int **need;
    int **requestType; //1 for request | 0 for release
    string **request;
};


int forkProcess(process process, int **pipes, int **childPipes, int *parentPipes)
{
    int forkValue;

    for(int i = 1; i<process.numProcess+1; i++)
    {
        pipe(pipes[i]); // [0]read form child | [1]write to parent
        pipe(&(pipes[i][2])); // [2]read from parent | [3]write to child



        if((forkValue = fork())>0)
        {
            /* PARENT */
            childPipes[i][0] = pipes[i][0]; //read from child
            childPipes[i][1] =  pipes[i][3]; //write to child
            close(pipes[i][1]);
            close(pipes[i][2]);

        }
        else if (forkValue==0)
        {
            /* CHILD */
            parentPipes[0] = pipes[i][2]; //read from parent
            parentPipes[1] = pipes[i][1]; //write to parent
            close(pipes[i][0]);
            close(pipes[i][3]);

            return i; //is child
        }
        else
        {
            perror("Error 404, Fork not found");
        }
    }
    return 0; // is parent
}

void requests(int *parentPipes, int procID, process process1)
{
    printf("child\n");
    char buffer[2];
    size_t readSize;
    while(1)
    {
        readSize = read(parentPipes[0], buffer, 2);
        if(readSize == -1 || readSize == 0) {
            // error or done
            return;
        }
        //printf("%d\n", readSize);
        if(buffer[0] == 'i' )
        {
            string maxLine;
            int findProcID = 0;

            /* READ FILE */
            ifstream myfile ("input.txt");
            if(myfile.is_open())
            {
                while(!myfile.eof())
                {
                    getline(myfile,maxLine);
                    if( 0 == maxLine.compare(0,8, "process_"))
                    {
                        maxLine = maxLine.substr(8);
                        int tempProcID;
                        tempProcID = atoi (maxLine.c_str());
                        if(procID == tempProcID)
                        {
                            findProcID = 1;
                            int tempStringToInt;
                            getline(myfile,maxLine);
                            stringstream(maxLine) >> tempStringToInt;
                            process1.deadline[procID] = tempStringToInt;
                            getline(myfile,maxLine);
                            stringstream(maxLine) >> tempStringToInt;
                            process1.computationTime[procID] = tempStringToInt;

                            for(int ctLoop = 1; ctLoop<process1.computationTime[procID]+1; ctLoop++)
                            {
                                string maxLine;
                                string checkType;
                                //parse out maximum demand for resource m by process n into max[][]
                                getline(myfile,maxLine);
                                checkType = maxLine.substr (0,7);
                                if(checkType == "request")
                                {
                                    maxLine = maxLine.substr(7);
                                    process1.requestType[procID][ctLoop] = 1;
                                    process1.request[procID][ctLoop] = maxLine;
                                }
                                else if (checkType == "release")
                                {
                                    maxLine = maxLine.substr(7);
                                    process1.requestType[procID][ctLoop] = 0;
                                    process1.request[procID][ctLoop] = maxLine;
                                }
                                else cout<<"This is neither a request nor a release"<<endl;
                                
                                
                            }
                        }
                    }
                }
            }
            myfile.close();

            int buffer_sz = 20;
            char buffer[buffer_sz];
            snprintf(buffer ,buffer_sz, "%d,%d" , process1.deadline[procID], process1.computationTime[procID]);
            write(parentPipes[1], buffer, buffer_sz);
            
            for(int ctLoop = 1; ctLoop<process1.computationTime[procID]+1; ctLoop++)
            {
                snprintf(buffer, buffer_sz,  "%d,%s" , process1.requestType[procID][ctLoop], process1.request[procID][ctLoop].c_str());
                write(parentPipes[1], buffer, buffer_sz);
            }

            /* END OF READ FILE */
        }
    }
}

void getInfo(process process, int **childPipes)
{

    char buffer[20];
    int deadlineFC;
    int computationFC;
    for(int i = 1; i<process.numProcess+1; i++)
    {
        write(childPipes[i][1], "i", 20);
        read(childPipes[i][0], buffer, 20);
        
        deadlineFC = atoi(buffer);
        computationFC = atoi(strchr(buffer, ',')+1);
        
        process.deadline[i] = deadlineFC;
        process.computationTime[i] = computationFC;
    }
}


int getNextProcSFJ(process process)
{
    
    int smallestID = -1;
    int smallest = 10000;
    for(int i = 1; i<process.numProcess+1; i++)
    {
        if(process.computationTime[i]<=0)
        {
            continue;
        }
        else if(smallest>process.computationTime[i])
        {
            smallest = process.computationTime[i];
            smallestID = i;
        }
    }
    return smallestID;
}

int getnextProcLLF(process process)
{
    int smallestID = -1;
    int smallest = 1000;
    for(int i=1; i<process.numProcess+1; i++)
    {
        int laxity = process.deadline[i]-process.computationTime[i];
        if(laxity<=0)
        {
            continue;
        }
        else if(smallest>laxity)
        {
            smallest = laxity;
            smallestID = i;
        }
    }
    return smallestID;
}

int bankers(process process, int next, int requestType, string request)
{
    int errorCount = 0;
    
    request = request.substr(1);
    int requestVector[process.numResource];
    //converst string to usable integers
    //cout<<requestType<<",";
    for(int m = 1; m<process.numResource+1; m++)
    {    
        requestVector[m] = atoi (request.c_str());
        
        //cout<<requestVector[m]<<",";
        
        request = request.substr(request.find(",")+1);
    }
    cout<<endl;
    
    
    cout<<"bankers algorithm for process "<<next<<" for ";
    
   
        if (requestType == 1)
        {
            cout<<" request "<<endl;
            for(int m = 1; m<process.numResource+1; m++)
            {
                cout<<requestVector[m]<<" is ";
                //BANKERS BEGIN
                if(requestVector[m] > process.need[next][m])
                {
                    cout<<"error:   ";
                    cout<<requestVector[m]<<">"<<process.need[next][m]<<endl;
                    errorCount++;
                }
                else if(requestVector[m] > process.available[next])
                {
                    cout<<"wait:    ";
                    cout<<requestVector[m]<<">"<<process.available[next]<<endl;
                }
                else
                {
                    cout<<"pretend: "<<endl;
                    
                    process.available[next] -= requestVector[m];
                    process.allocated[next][m]+= requestVector[m];
                    process.need[next][m] -= requestVector[m];
                }
            }
        }
        else if(requestType == 0)
        {
            cout<<" release "<<endl;
            for(int m = 1; m<process.numResource+1; m++)
            {
                cout<<requestVector[m]<<" is ";
                //BANKERS BEGIN
                if(requestVector[m] > process.need[next][m])
                {
                    cout<<"error:   ";
                    cout<<requestVector[m]<<">"<<process.need[next][m]<<endl;
                    errorCount++;
                }
                else if(requestVector[m] > process.available[next])
                {
                    cout<<"wait:    ";
                    cout<<requestVector[m]<<">"<<process.available[next]<<endl;
                }
                else
                {
                    cout<<"pretend: "<<endl;
                    
                    process.available[next] += requestVector[m];
                    process.allocated[next][m]-= requestVector[m];
                    process.need[next][m] += requestVector[m];
                }
            }
        }
    if(errorCount>0)
    {
        return 1;
    }
    else return 0;
}

void manager(process process, int **childPipes, string schedule)
{
    printf("parent\n");
    getInfo(process, childPipes);
    
    for(int n = 1; n<process.numProcess+1; n++)
    {
        for(int m = 1; m<process.numResource+1; m++)
        {
            process.need[n][m] = process.max[n][m];
        }
    }
    
    while(true)
    {
        if(schedule == "sjf" || schedule == "SJF")
        {
            int next = getNextProcSFJ(process);
            
            if( next == -1 )
            {
                return;
            }
            
            //loop through request from child
            for(int resCount = 1; resCount<process.computationTime[next]+1; resCount++)
            {
                char request[20];

                read(childPipes[next][0], request, 20);
                //conversion
                process.requestType[next][resCount] = atoi(request);
                process.request[next][resCount] = strchr(request, ',')+1;
                
                
                
                int bankerReturn = bankers(process, next, process.requestType[next][resCount], process.request[next][resCount]);
                
                if(bankerReturn == 1)
                {
                    cout<<"ERROR"<<endl;
                }
                else
                {
                    cout<<"NOT ERROR"<<endl;
                }

                
            }
            process.computationTime[next]=0;
        }
        else if(schedule == "llf" || schedule == "LLF")
        {
            int next = getnextProcLLF(process);
        
            if( next == -1 )
            {
                return;
            }
            
            //loop through request from child
            for(int resCount = 1; resCount<process.computationTime[next]+1; resCount++)
            {
                char request[20];
                
                read(childPipes[next][0], request, 20);
                //conversion
                process.requestType[next][resCount] = atoi(request);
                process.request[next][resCount] = strchr(request, ',')+1;
                
                                
                int bankerReturn = bankers(process, next, process.requestType[next][resCount], process.request[next][resCount]);
                
                if(bankerReturn == 1)
                {
                    cout<<"ERROR"<<endl;
                }
                else
                {
                    cout<<"NOT ERROR"<<endl;
                }
            }
            process.computationTime[next]=0;
            process.deadline[next] = 0;
        }
        else
        {
            cout<<"Not a valid schedule"<<endl;
            cout<<"Exiting....."<<endl;
            exit(0);
        }
    }
}

int main(int argc, const char * argv[])
{
    process process1;
    
    string schedule;
    cout<<"enter sjf or llf:";
    cin>>schedule;

    /* READ FILE */
    ifstream myfile ("input.txt");
    if (myfile.is_open())
    {
        string line;
        //get m resources
        getline(myfile,line);
        int m;
        stringstream(line) >> m;
        process1.numResource = m;

        //get n process
        getline(myfile,line);
        int n;
        stringstream(line) >> n;
        process1.numProcess = n;

        /* AVAILABLE */

        //allocate memory for available
        process1.available = new int[process1.numResource];

        //feed input to struct *available
        for(int i = 1; i<process1.numProcess+1; i++)
        {
            int tempAvail;
            string availLine;
            //parse out instances of resources into available[]
            getline(myfile.ignore(20, '='),availLine);
            stringstream(availLine) >> tempAvail;
            process1.available[i] = tempAvail;
            cout<<"process["<<i<<"] available= "<<process1.available[i]<<endl;
        }

        /* MAX */

        //allocate memory for max
        process1.max = new int*[process1.numResource+1];
        process1.need = new int*[process1.numResource+1];
        process1.allocated = new int*[process1.numResource+1];
        for(int i = 0; i< process1.numProcess+1; i++)
        {
            process1.max[i] = new int(process1.numProcess);
            process1.need[i] = new int(process1.numProcess);
            process1.allocated[i] = new int(process1.numProcess);
        }
        //feed input to struct **max

        for(int n = 1; n<process1.numProcess+1; n++)
        {
            for(int m = 1; m<process1.numResource+1; m++)
            {
                int tempStringToInt;
                string maxLine;
                //parse out maximum demand for resource m by process n into max[][]
                getline(myfile.ignore(20, '='),maxLine);
                stringstream(maxLine) >> tempStringToInt;
                process1.max[n][m] = tempStringToInt;
            }
        }
        //allocate memory for deadline
        process1.deadline = new int[process1.numProcess];
        //allocate memory for computationTime
        process1.computationTime = new int[process1.numProcess];
       
        process1.requestType = new int*[process1.numResource+1];
        process1.request = new string*[process1.numResource+1];
        for(int i =0; i<process1.numResource+1; i++)
        {
            process1.requestType[i] = new int(process1.numProcess+1);
            process1.request[i] = new string[process1.numProcess+1];
        }
        
    }
    else cout << "Unable to open file\n";
    myfile.close();
    /* END READ FILE */


    //allocate memory for pipes | childPipes | parentPipes
    int **pipes = new int*[process1.numProcess+1];
    int **childPipes = new int*[process1.numProcess+1]; // read from child | write to child
    int *parentPipes = new int[2]; // read from parent | write to parent

    for(int i = 0; i< process1.numProcess+1; i++)
    {
        pipes[i] = new int(4);
        childPipes[i] = new int(2);
    }
    
    int procID = forkProcess(process1, pipes, childPipes, parentPipes); //return process ID

    if(procID > 0)
    {
        requests(parentPipes, procID, process1);
    }
    else
    {
        manager(process1, childPipes, schedule);
    }
}

