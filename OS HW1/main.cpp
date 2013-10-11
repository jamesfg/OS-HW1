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
    int *available; // available[resource] = assigned number of instances
    int **max; // max[process+1][resource+1] = max number of instances
    int *deadline; // deadline for numProcess
    int *computationTime; //computation time for numProcess
    int **allocated;
    int **need;
    int *requestType;
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


            //printf("request from process ");
            //cout<<procID<<endl;

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
                            //cout<<"process id is "<<tempProcID<<endl;
                            findProcID = 1;
                            int tempStringToInt;
                            getline(myfile,maxLine);
                            stringstream(maxLine) >> tempStringToInt;
                            process1.deadline[procID] = tempStringToInt;
                            //cout<<"process1.deadline["<<procID<<"]="<<process1.deadline[procID]<<endl;
                            getline(myfile,maxLine);
                            stringstream(maxLine) >> tempStringToInt;
                            process1.computationTime[procID] = tempStringToInt;
                            //cout<<"process1.computationTime["<<procID<<"]="<<process1.computationTime[procID]<<endl;


                            for(int ctLoop = 1; ctLoop<process1.computationTime[procID]+1; ctLoop++)
                            {
                                int requestIter = 1;
                                string maxLine;
                                string checkType;
                                //parse out maximum demand for resource m by process n into max[][]
                                getline(myfile,maxLine);
                                checkType = maxLine.substr (0,7);
                                if(checkType == "request")
                                {
                                    maxLine = maxLine.substr(7);
                                    //cout<<"request"<<maxLine<<endl;
                                    process1.requestType[requestIter] = 1;
                                    requestIter++;

                                    maxLine = maxLine.substr(1);
                                    //cout<<procID<<" has ";
                                    //cout<<"request(";
                                    //loop through request vector | find numbers
                                    for(int rvLoop = 1; rvLoop<process1.numResource+1; rvLoop++)
                                    {
                                        char * buffer = new char [maxLine.length()+1];
                                        strcpy (buffer, maxLine.c_str());
                                        int requestVector;
                                        requestVector = atoi (buffer);

                                        //cout<<requestVector<<",";

                                        maxLine = maxLine.substr(maxLine.find(",")+1);
                                        delete[] buffer;
                                    }
                                    //cout<<")"<<endl;
                                }
                                else if (checkType == "release")
                                {
                                    maxLine = maxLine.substr(7);
                                    //cout<<"release"<<maxLine<<endl;
                                    process1.requestType[requestIter] = 0;
                                    requestIter++;

                                    maxLine = maxLine.substr(1);
                                    //loop through request vector | find numbers
                                    //cout<<procID<<" has ";
                                    //cout<<"release(";
                                    for(int rvLoop = 1; rvLoop<process1.numResource+1; rvLoop++)
                                    {
                                        char * buffer = new char [maxLine.length()+1];
                                        strcpy (buffer, maxLine.c_str());
                                        int requestVector;
                                        requestVector = atoi (buffer);

                                        //cout<<requestVector<<",";

                                        maxLine = maxLine.substr(maxLine.find(",")+1);
                                        delete[] buffer;
                                    }
                                    //cout<<")"<<endl;
                                }
                                else cout<<"This is neither a request nor a release"<<endl;
                            }
                        }
                    }
                }
            }
            myfile.close();

            ostringstream convert;

            convert << process1.deadline[procID];
            string itos = convert.str();

            int buffer_sz = 20;
            char buffer[buffer_sz];
            snprintf(buffer ,buffer_sz, "%d", process1.deadline[procID]);

            write(parentPipes[1], buffer, buffer_sz);

            /* END OF READ FILE */
        }
    }
}

void getInfo(process process, int **childPipes)
{

    char buffer[2];
    int deadlineFC;
    for(int i = 1; i<process.numProcess+1; i++)
    {
        write(childPipes[i][1], "i", 2);
        cout<<"read child pipes"<<endl;
        read(childPipes[i][0], buffer,2);

        deadlineFC = atoi(buffer);

        cout<<deadlineFC<<endl;
    }
}

void manager(process process, int **childPipes)
{
    printf("parent\n");
    getInfo(process, childPipes);
//    while(true)
//    {

//    }
}



void getNextProcess(process process, int **childPipes)
{
    //
}


int main(int argc, const char * argv[])
{
    process process1;

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

        //cout<<"process1.numResource="<<process1.numResource<<endl;
        //cout<<"process1.numProcess="<<process1.numProcess<<endl;


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
            //cout<<"process1.available["<<i<<"]="<<process1.available[i]<<endl;
        }

        /* MAX */

        //allocate memory for max
        process1.max = new int*[process1.numResource+1];
        for(int i = 0; i< process1.numProcess+1; i++)
        {
            process1.max[i] = new int(process1.numProcess);
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
                //cout<<"process1.max["<<n<<"]["<<m<<"]="<<process1.max[n][m]<<endl;
            }
        }
        //allocate memory for deadline
        process1.deadline = new int[process1.numProcess];
        //allocate memory for computationTime
        process1.computationTime = new int[process1.numProcess];
        process1.requestType = new int[500];
    }
    else cout << "Unable to open file\n";
    myfile.close();
    /* END READ FILE */


    //allocate memory for pipes | childPipes | parentPipes
    int **pipes = new int*[process1.numProcess+1];
    int **childPipes = new int*[process1.numProcess+1]; // write to child | read from child
    int *parentPipes = new int[2]; // write to parent | read from parent

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
        manager(process1, childPipes);
    }
}

