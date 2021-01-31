#include <bits/stdc++.h> 
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string>
#include <sstream>
#include <chrono>
#include <vector>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fstream>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <thread>
#include <mutex> 

using namespace std;

//Color Codes
#define RESET   "\033[0m"
/*FOREGROUNDS OR TEXT*/
#define BLACKTXT   "\033[30m"      /* Black */
#define REDTXT     "\033[31m"      /* Red */
#define GREENTXT   "\033[32m"      /* Green */
#define YELLOWTXT  "\033[33m"      /* Yellow */
#define BLUETXT    "\033[34m"      /* Blue */
#define MAGENTATXT "\033[35m"      /* Magenta */
#define CYANTXT    "\033[36m"      /* Cyan */
#define WHITETXT   "\033[37m"      /* White */
/*BACKGROUND COLOURS*/
#define BGBLACK   "\033[40m"      /* Black */
#define BGRED     "\033[41m"      /* Red */
#define BGGREEN   "\033[42m"      /* Green */
#define BGYELLOW  "\033[43m"      /* Yellow */
#define BGBLUE    "\033[44m"      /* Blue */
#define BGMAGENTA "\033[45m"      /* Magenta */
#define BGCYAN    "\033[46m"      /* Cyan */
#define BGWHITE   "\033[47m"      /* White */
/*BOLD TEXTS*/
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */
/*STYLES*/
#define UNDERLINED		"\033[4m"			/*UNDERLINE*/
#define BOLDED			"\033[1m"			/*BOLD*/
#define INVERSED		"\033[7m"			/*SWITCH BG WITH FG*/
#define UNDERLINEOFF	"\033[24m"			/*UNDERLINE OFF*/
#define BOLDOFF			"\033[21m"			/*BOLD OFF*/
#define INVERSEOFF		"\033[27m"			/*INVERSE OFF*/

#define mtxPrint printMtx.lock();cout
#define endMtxPrint ;printMtx.unlock()

//Defining port number
int NODE_PORT; 			//Second Input of File 	23010
//int RECEIVE_PORT; 	//First Input of File 	22010
int EDGE_INITIAL_PORT; 	//Third Input of File 	12500

//Node Adjacency Global Graph
vector<vector<int>> adj;
vector<vector<int>> minStretchAdj;
vector<int> minStretchDistances;

//Vector storing index corresponding to edges
vector<vector<int>> EdgeIndexes;

//Vector to store distances in original graph
vector<vector<int>> originalDistances;

//Vector to store distances in Max Centrality Spanning graphs
vector<vector<int>> maxSpanningTreeDistances;
//Vector to store distances in Dijkstra graphs
vector<vector<int>> minStretchDistanceAll;

//Vector Storing Stretch per Edge
vector<vector<float>> maxSpanningTreeStretchAll;
float maxSpanningTreeStretch;
vector<vector<float>> dijkstraStretchAll;
float dijkstraStretch;

//Mutex for distance Addition
mutex distanceAddition;
mutex maxSpanningDistanceAddition;
mutex minStretchDistanceAddition;

//Printing Mutex
mutex printMtx;

//Lock for calculating the edge nodes per node
vector<mutex> processRcvMtx;
vector<mutex> edgeRcvMtx;

mutex processReceiveMutex;
mutex edgeReceiveMutex;


//Number of nodes
int n;

//Number of edges or Edge nodes
int totalEdges;

//Stores number of responsible edges for each node (Assigned to lower id between the 2 connected)
vector<int> PortsToAssign;

//Mutex to add to Global graph
mutex globalGraphAddition;

//Global Edge graph Adjacency list
vector<vector<struct edge_node>> globalEdgeGraph;

//Map to Convert Edge graph to indexes
map<pair<int, int>, int> edgeToIndex;
map<int, pair<int, int>> indexToEdge;

//Vector containing converted graph
vector<vector<int>> convertedEdgeGraphGlobal;

//Vector for All Edge centrality
vector<float> edgeCentralities;
vector<float> edgeCentralitiesSender;
mutex centralityAccess;
mutex centralityAccessSender;

vector<struct EdgeCentralitiesWithIndex> indexedEdgeCentralities;
vector<int> sortedEdgeCentralityIndexes;

//Stores Maximum Edge Centrality Index
int firstMaxEdgeCentralityIndex;



//Struct for Edges request by id  for edge Graph construction
#pragma pack(1)
struct request
{
	int process_id;
	int request_flag;
}request_msg;
#pragma pack(0)

// Reply for edge Graph construction
#pragma pack(1)
struct reply
{
	int process_id;
	vector<int> v;
}reply_msg;
#pragma pack(0)

//Struct for two nodes in an edge node
struct edge_node
{
	int u;
	int v;
};

//Vector for Maximum Spanning Tree
vector<int> maxSpanningTreeEdges;
vector<vector<int>> maxSpanningTreeAdjacencyList;

//Total Centrality at each process_id as start
//--Unused
struct betweenness_reply
{
	int process_id;
	float cb;
}betweenness_centrality_reply;


//Struct for Stack for Leaf Calculation
// or Number of Shortest paths nodes lie on between fixed Source and varying T
struct treeLeafCount
{
	int Value;
	int lastPred;
	int leafs;
}treeLeaf;

struct EdgeCentralitiesWithIndex
{
	float value;
	int index;
};



//MST calculation using Disjoint Sets
struct disjointSet
{
	int value;
	int rank;
};
vector<int> disjointArray;
map<int, int> disjointMap;


//Entry in stack using the function
struct treeLeafCount StackEntry(int value)
{
	struct treeLeafCount temp;
	temp.Value = value;
	temp.lastPred = -1;
	temp.leafs = 0;
	return temp;
}


//Comparison function for Sorting Indexed Edge Centralities
bool IndexedSmallerEdgeCentrality(struct EdgeCentralitiesWithIndex a,struct EdgeCentralitiesWithIndex b)
{
	return a.value < b.value;
}

//Comparison for array[TotalEdges] storing index i in array[i] for Sorting Indexes of Edge Centralities
bool IndexesSmallerEdgeCentrality(int a,int b)
{
	return edgeCentralities[a] < edgeCentralities[b];
}

//Comparison for array[TotalEdges] storing index i in array[i] for Sorting Indexes of Edge Centralities
bool IndexesGreaterEdgeCentrality(int a,int b)
{
	return edgeCentralities[a] > edgeCentralities[b];
}

//Returns the edge_node after parsing the string used in socket comm to send it
struct edge_node ParseEdgeNode(string str,char delimeter)
{
  string a="",b="";
  int i = 0, flag = 0;
  while(str[i])
  {
      if(!flag)
      {
          if(str[i]==delimeter)
          {
              flag = 1;
          }
          else{
              a+=str[i];
          }
      }
      else
      {
          b+=str[i];
      }
      i++;
  }
  struct edge_node parsedEdge;
  parsedEdge.u=stoi(a);
  parsedEdge.v=stoi(b);
  return parsedEdge;
  //If unordered edgeNode sequence expected use
  //return concat2(stoi(a),stoi(b));
}


//Struct Edge Node with two ints, smaller being u, and greater being v
struct edge_node concat2(int a, int b)
{
	struct edge_node edgeNode;
	if(a<b)
	{
		edgeNode.u = a;
		edgeNode.v = b;
	}
	else
	{
		edgeNode.u = b;
		edgeNode.v = a;	
	}
    return edgeNode; 
} 

//Concat Pair int
pair<int, int> concatPair(int a, int b)
{
	int u,v;
	if(a<b)
	{
		u = a;
		v = b;
	}
	else
	{
		u = b;
		v = a;	
	}
    pair<int, int> edgeNode = make_pair(u,v);
    return edgeNode; 
} 

//Orders the concat in lexicographic order
string concat(int a, int b) 
{
	string s1,s2;
	if(a<b)
	{
		 s1 = to_string(a); 
   		 s2 = to_string(b); 
	}
	else
	{
		 s1 = to_string(b); 
   		 s2 = to_string(a); 	
	}
  
    // Concatenate both strings 
    string s = s1 + s2; 
  
    // Convert the concatenated string to integer 
  	//  int c = stoi(s); 
  
    // return the formed integer 
    return s; 
} 

//Receive event of the process number i and having port number port_num with its adj list vector
int receiver(int i,vector<int> vect,int port_num)
{

	//Declaration of the socketid
	int serversockid,status,z;
	
	struct sockaddr_in serverAddr;	
	
	//Server socket creation
	serversockid = socket(AF_INET, SOCK_STREAM,0);
	if(serversockid==-1)
	{
		mtxPrint<<REDTXT<<endl<<"socket creation fail of server "<<i<<RESET<<endl endMtxPrint;
		return 0;
	}
	printMtx.lock();
	cout <<YELLOWTXT<< "Receiver Socket created at " << i <<endl<<RESET;
	printMtx.unlock();

	//specifying addresses
	serverAddr.sin_family=AF_INET;
	serverAddr.sin_port=htons(port_num);
	serverAddr.sin_addr.s_addr=htonl(INADDR_ANY);

	//bind the socket to our specified ip and port
	 status = bind(serversockid,(struct sockaddr*) &serverAddr, sizeof(serverAddr));
	 // To check whether bind is successful or not
	if(status==-1)
	{
		mtxPrint<<endl<<REDTXT << "binding failed at "<< port_num<<"at server "<<i<< RESET <<endl endMtxPrint;
		return 0;
	}
	printMtx.lock();
	cout <<YELLOWTXT<<"Receiver Binding done at " << i <<endl<<RESET;
	printMtx.unlock();

	//listening

	int status2;
    status2= listen(serversockid, vect.size());
	if(status2==-1)
	{
		mtxPrint<<REDTXT<<endl<<"listening error at server "<<i<<RESET<<endl endMtxPrint;
		return 0;
	}
	printMtx.lock();
	cout <<YELLOWTXT<<"Receiver Listening at " << i <<endl<<RESET;
	printMtx.unlock();

	//Accept requests from other servers
	for(z=1; z<=vect.size(); z++)
	{

				int new_socket;
				struct sockaddr_in clientAddr;
				int clientAddrLen= sizeof(clientAddr);

				// Accepting the requests
				printMtx.lock();
				cout << YELLOWTXT << "Receiver accepting requests at " << i << endl << RESET;
				printMtx.unlock();
				new_socket= accept(serversockid,(struct sockaddr *) &clientAddr, (socklen_t*)&clientAddrLen);	//&clientAddrLen we can put here NULL at the place of addresses 													if server is not interested in client's address
				if(new_socket == -1)
				{
						mtxPrint<<REDTXT <<endl<<"accepting failed at server "<<i<<endl<<RESET endMtxPrint;
						return 0;
				}

				//receive request from other processes
				
				recv(new_socket,&request_msg,sizeof(struct request),0);
				//cout<<"Process "<< i<<" receives request from process "<<request_msg.process_id<<" for sending its own adjacency list "<<endl;		
				request_msg.request_flag=0;
				reply_msg.process_id=i;
  				reply_msg.v=vect;
  				//cout<<"Process "<< i<<" sends its own adjacency list to process "<<request_msg.process_id<<endl;
				send(new_socket,&reply_msg,sizeof(struct reply),0);
					
				//Close the socket after communication
				close(new_socket);
				
				//if( z== vect.size())
				//{
					// Close socket when it accepts all the requests
					
				//}
 	}
	close(serversockid);
	return 0;
}

//Send event of the process i
int sender(int i,vector<int> vect,int port_num)
{		
	int j=0;

    int k=0;
    vector<string> line_graph_string[vect.size()];
	vector<struct edge_node> line_graph[vect.size()];
    vector<int> neighbor[vect.size()];
	printMtx.lock();
	cout <<MAGENTATXT<< "Reached above main loop " << i << endl<<RESET;
	printMtx.unlock();
		for(auto t = vect.begin(); t != vect.end(); ++t)	
		{
			int currentT = *t;
			printMtx.lock();
			cout << MAGENTATXT << i << " is going to lock " << currentT << endl << RESET;
			printMtx.unlock();
			processReceiveMutex.lock();	
				// Declaration of the variables
				int clientsockid,status;
				//Client socket creation
				clientsockid = socket(AF_INET, SOCK_STREAM,0);

				if(clientsockid==-1)
				{
					cout<<REDTXT <<endl<<"socket creation fail at client "<<i<<endl<<RESET;
					processReceiveMutex.unlock();
					return 0;
				}
				printMtx.lock();
				cout <<MAGENTATXT<< "Sender socket created for " << *t << " by " << i << endl<<RESET;
				printMtx.unlock();

				//specify address
				struct sockaddr_in serverAddr;
				serverAddr.sin_family=AF_INET;
				serverAddr.sin_port=htons(NODE_PORT+2*(*t)+1);
				serverAddr.sin_addr.s_addr=htonl(INADDR_ANY); 	

				//connection request to the other server
				int connection_status=connect(clientsockid,(struct sockaddr*) &serverAddr,sizeof(serverAddr));	
				if(connection_status==-1)
				{
					cout<< REDTXT << "socket connection fail at client"<<i << " for " << *t<< RESET << endl;
					processReceiveMutex.unlock();
					return 0;
				}

				request_msg.process_id= i;
				request_msg.request_flag=1;
				
				//sending request for giving their correspnding information of neighbors
				//cout<<"Process "<< i<<" sends request to process "<<*t<<" for sending its adjacency list "<<endl;
				send(clientsockid,&request_msg,sizeof(struct request),0);

				//receive synch. reply from the server
				recv(clientsockid,&reply_msg,sizeof(struct reply),0);
				//cout<<"Process "<< i<<" receives  adjacency list of process "<<reply_msg.process_id<<endl;	
					
				int pid=reply_msg.process_id;
				neighbor[j]=reply_msg.v;
				auto it = neighbor[j].insert(neighbor[j].begin(), pid);
				j++; 

				//close the socket after receiving the reply
			close(clientsockid);
			processReceiveMutex.unlock();	
			printMtx.lock();
			cout << MAGENTATXT << i << " has unlocked " << currentT << endl << RESET;
			printMtx.unlock();	 
		}

/*	processRecieveMutex.lock();	
 	cout<<endl<<"For process "<< i<<": "<<endl;
 	for (int v = 0; v < vect.size(); ++v) 
    { 
		        cout << "\n Adjacency list of vertex "
		             << neighbor[v][0]<< "\n head "; 
		        for (auto x : neighbor[v]) 
		           cout <<"-> "<< x; 
		        printf("\n"); 
    } 
*/


   for(auto p = vect.begin(); p != vect.end(); ++p)
   {
   				int r;
   				string edge;
				struct edge_node tempEdge, thisEdge;
   				thisEdge = concat2(*p,i);
   				line_graph[k].push_back(thisEdge);
   				for(auto q = vect.begin(); q != vect.end(); ++q )
   				{
   					if(*q != *p)
   					{
   						tempEdge = concat2(*q,i);
   						line_graph[k].push_back(tempEdge);
   					}
   				}
   				for( r=0;r<vect.size();r++)
   				{
   					if(neighbor[r][0]== *p)
   						break;
   				}

   				for(auto u = neighbor[r].begin(); u!= neighbor[r].end(); ++u)
   				{
   					if(*u != i && *u != *p)
   					{
   						tempEdge = concat2(*p, *u);
   						line_graph[k].push_back(tempEdge);
   					}
   				}
   				k++;
   }

	printMtx.lock();
   	cout<<endl<<UNDERLINED<<" Line graph For process "<< i<<": "<<endl<<RESET;
 	for (int v = 0; v < vect.size(); ++v) 
    { 
		//If the process is lower id one
		if(line_graph[v][0].u == i)
		{
			globalGraphAddition.lock();
			globalEdgeGraph.push_back(line_graph[v]);
			globalGraphAddition.unlock();
		}
    	cout<<"head ";
		        for (auto x : line_graph[v]) 
		           cout<<" ->"<< x.u << "+" << x.v; 
		        printf("\n");
    } 
	cout << endl;


    printMtx.unlock();
		
    return 0;
}

//Collector for Centrality value for each edge
int CentralityReceiver(int myProcessId,vector<int> neighbor_edges)
{

	int port_num = EDGE_INITIAL_PORT + 2* myProcessId + 1;
	float myTotalCentrality=0, tempCB;
	//Declaration of the socketid
	int serversockid,status,z;
	
	struct sockaddr_in serverAddr;	
	
	//Server socket creation
	serversockid = socket(AF_INET, SOCK_STREAM,0);
	if(serversockid==-1)
	{
		mtxPrint<<endl<<REDTXT<<"socket creation fail of server "<<myProcessId<<endl<<RESET endMtxPrint;
		return 0;
	}

	//specifying addresses
	serverAddr.sin_family=AF_INET;
	serverAddr.sin_port=htons(port_num);
	serverAddr.sin_addr.s_addr=htonl(INADDR_ANY);

	//bind the socket to our specified ip and port
	 status = bind(serversockid,(struct sockaddr*) &serverAddr, sizeof(serverAddr));
	 // To check whether bind is successful or not
	if(status==-1)
	{
		mtxPrint<<endl<<REDTXT<<"binding failed at "<< port_num<<"at server "<<myProcessId<<endl<<RESET endMtxPrint;
		return 0;
	}
  
	//listening

	int status2;
    status2= listen(serversockid, totalEdges);
	if(status2==-1)
	{
		mtxPrint<<endl<<REDTXT<<"listening error at server "<<myProcessId<<endl<<RESET endMtxPrint;
		return 0;
	}

	//Accept value of myTotalCentrality from other node
	for(z=0; z<totalEdges; z++)
	{
				int new_socket;
				struct sockaddr_in clientAddr;
				int clientAddrLen= sizeof(clientAddr);

				// Accepting the requests
				new_socket= accept(serversockid,(struct sockaddr *) &clientAddr, (socklen_t*)&clientAddrLen);	//&clientAddrLen we can put here NULL at the place of addresses 													if server is not interested in client's address
				if(new_socket == -1)
				{
						mtxPrint<<endl<<REDTXT<<"accepting failed at server "<<myProcessId<<endl<<RESET endMtxPrint;
						return 0;
				}

				//receive request from other processes
				
				recv(new_socket,(void *)&tempCB,sizeof(float),0);

				myTotalCentrality += tempCB;

				//Close the socket after communication
				close(new_socket);
	}

		// Close socket when it accepts all the requests
		close(serversockid);
		centralityAccess.lock();
		edgeCentralities[myProcessId] = myTotalCentrality/2;
		centralityAccess.unlock();
		return 0;
}

//Creating a Calculating Edge process
int CentralitySender(int myProcessId)
{
	int myPortNum = EDGE_INITIAL_PORT + 2 * myProcessId;
	//Stores the number of times node v occurs in paths from myProcessId to Target
	// where target is the column
	// and  v is the row
	int pathsVT[totalEdges][n];
	//Setting all values to 0
	memset(pathsVT, 0, sizeof(int) * totalEdges * n);

    //Total paths to t
    vector<int> TotalPaths(n,0);

	//Stores final calculated Centrality for myProcessId as origin
	vector<float> TotalCB(totalEdges,0);

	//Stores predecessor
	vector<vector<int>> pred(n);

	//Shortest Distance from myProcessId to each other node
	vector<int> dist(n,INT_MAX);

	//Visited list for Queue
	vector<bool> isVisited(n, false);

	//Queue for BFS to mark distances to each node
	queue<int> BFSQueue;

	//Stack for leaf Calculation
	stack<struct treeLeafCount> LeafCalculation;
	struct treeLeafCount LeafStack[totalEdges];

	
	
	//Calculating the distances with BFS
	BFSQueue.push(myProcessId);
	isVisited[myProcessId] = true;
	dist[myProcessId] = 0;
	///mtxPrint << CYANTXT << "Entering the Queue with "<< BFSQueue.front() << " for  " << myProcessId << endl << "\tQueue Empty : " << !BFSQueue.empty() <<endl <<RESET endMtxPrint;//Debug
	
	while(!(BFSQueue.empty()))
	{
		///mtxPrint << CYANTXT << "Entering the Queue Loop with "<< BFSQueue.front() << " for  " << myProcessId << endl <<RESET endMtxPrint;//Debug
		int addChildrenOf = BFSQueue.front();
		BFSQueue.pop();
		int distToNextNode = dist[addChildrenOf]+1;
		auto adjacentsRow = adj[addChildrenOf];
		///mtxPrint << CYANTXT << "Popped " << addChildrenOf <<" from the Queue for  " << myProcessId << endl<< RESET endMtxPrint;//Debug
		for(auto adjacent:adjacentsRow)
		{
			//Add minimum distance to next visited Node
			dist[adjacent] = min(dist[adjacent], distToNextNode);

			//If Distance set is equal to our current reaching distance to Next node
			// We are on a shortest path to the node
			// Add the current node as pred to Next node
			if(distToNextNode == dist[adjacent])
			{
				pred[adjacent].push_back(addChildrenOf);
			}

			//Add to queue only if previously not visited
			if(!isVisited[adjacent])
			{
				BFSQueue.push(adjacent);
				isVisited[adjacent] = true;
			}
		}
	}
	///mtxPrint << CYANTXT << "Exiting the Queue for  " << myProcessId << endl << RESET endMtxPrint;//Debug

	//Debug Printing the pred and distances
	printMtx.lock();
	cout << UNDERLINED << endl << " Printing the pred and distances at "<< myProcessId <<" : " << endl << RESET;
	for(int i=0;i<n;i++)
	{
		auto predecessors = pred[i];
		cout << "For " << i << " : " << dist[i]  << " : \t";
		for(auto j:predecessors)
		{
			cout << j << " ";
		}
		cout << endl;
	}
	cout << endl;
	printMtx.unlock();

    distanceAddition.lock();
    originalDistances[myProcessId] = dist;
    distanceAddition.unlock();

	
	//Number of shortest Paths between myProcessid and terminal t
	// passing through v calculation
	// for each t
    int terminalNode = 0; //Could start from myProcessId
	//mtxPrint << BOLDCYAN << "Reached Terminal Node Loop of " << myProcessId << " for " << terminalNode << endl << RESET endMtxPrint; //Debug
	while(terminalNode<n)
	{
		///mtxPrint << BOLDCYAN << "Inside Loop " << myProcessId << " for " << terminalNode << endl << RESET endMtxPrint; //Debug
		if(terminalNode==myProcessId)
		{
			///mtxPrint << BOLDCYAN << "Bypassed " << myProcessId << " for " << terminalNode << endl << RESET endMtxPrint; //Debug
			//pathsVT[myProcessId][myProcessId] = 0;
			terminalNode++;
			continue;
		}

		//Calculate leafs from self as root
		// while travelling from terminalNode(Root) as source to myProcessid(leafs)
		// through all n shortest path
		//					: There will be n leaf myProcessid nodes
		//Stack calculation
        int firstNode, secondNode, indexOfEdge;
        pair <int, int> edgePaired;
        map<pair<int, int>, int> :: iterator currentEdge;
		//currentEdge = indexToEdge.find(sortedEdgeCentralityIndexes[i]);
		int Top = 0;
		int From = -1;
		int tempLeaves;
		struct treeLeafCount stackTemp;
			stackTemp.Value = terminalNode;
			stackTemp.lastPred = -1;
			stackTemp.leafs = 0;
		LeafStack[0] = stackTemp;
		From++;
		Top++;
		///mtxPrint << BOLDCYAN << "Above Stack Loop with From : " << From << " of " << myProcessId << " for " << terminalNode << endl << RESET endMtxPrint; //Debug
		while (true)
		{
			///mtxPrint << BOLDCYAN << "Inside top of Stack Loop with From : " << From << " of " << myProcessId << " for " << terminalNode << endl << RESET endMtxPrint; //Debug

			//If we have reached a leaf (obviously myProcessId)
			if(LeafStack[From].Value == myProcessId)
			{
				//Increment Leafs and PathVT for its parent(if any) and self
				LeafStack[From].leafs=1;
                TotalPaths[terminalNode]++;
                tempLeaves = LeafStack[From].leafs;
				//pathsVT[myProcessId][terminalNode]++;
                firstNode = LeafStack[From].Value;
				Top--;
				From--;
				if(From<0)
				{
					break;
				}
                secondNode = LeafStack[From].Value;
                edgePaired = concatPair(firstNode, secondNode);
                currentEdge = edgeToIndex.find(edgePaired);
                // mtxPrint << YELLOWTXT << (currentEdge->first).first << " " << (currentEdge->first).second << " indexed "
                //          << currentEdge->second 
                //          << " at process " << myProcessId 
                //          << " with templeaves " << tempLeaves << endl << RESET; 
                printMtx.unlock();
                indexOfEdge = (currentEdge->second);
				LeafStack[From].leafs++;
				pathsVT[indexOfEdge][terminalNode]++;
			}

			//If all pred/ child have been added previously 
			// Remove the node, passing the leaf count to parent(if any)
			if(LeafStack[From].lastPred == (pred[LeafStack[From].Value].size() - 1 ) )
			{
				if(From<=0)
				{
					break;
				}
				tempLeaves = LeafStack[From].leafs;
                firstNode = LeafStack[From].Value;
				From--;
				Top--;
				LeafStack[From].leafs += tempLeaves;
                secondNode = LeafStack[From].Value;
                currentEdge = edgeToIndex.find(edgePaired);
                // mtxPrint << YELLOWTXT << (currentEdge->first).first << " " << (currentEdge->first).second << " indexed "
                //          << currentEdge->second 
                //          << " at process " << myProcessId 
                //          << " with Leaves " << tempLeaves << endl << RESET; 
                printMtx.unlock();
                indexOfEdge = (currentEdge->second);
				pathsVT[indexOfEdge][terminalNode] += tempLeaves;
			}

			//If a pred exists that has not been added
			// after traversing all previously added pred to their leafs
			// Add next pred
			else
			{
				LeafStack[From].lastPred++;
					stackTemp = StackEntry(pred[LeafStack[From].Value][LeafStack[From].lastPred]);
				LeafStack[Top] = stackTemp;
				From++;
				Top++;
			}
			///mtxPrint << BOLDCYAN << "Doing some Stack at " << myProcessId << " for " << terminalNode << endl << RESET endMtxPrint; //Debug
		}
		terminalNode++;
	}


	
	//Totalling all centrality
	for(int v=0;v<totalEdges;v++)
	{
		float SumCbv = 0;
		for(int t=0;t<n;t++)
		{
            if(t==myProcessId)
            {
                continue;
            }
			SumCbv += ((float)pathsVT[v][t])/((float)TotalPaths[t]);
		}
		TotalCB[v] = SumCbv;
	}


	//Debug Printing the Paths
	printMtx.lock();
    cout << UNDERLINED << endl << "The Paths at " << myProcessId << " : " << endl << RESET;
	cout << "\t";
	for(int i=0;i<n;i++)
	{
		cout << UNDERLINED<< i << RESET <<"\t"; 
	}
	cout << endl;
	for(int i=0;i<totalEdges;i++)
	{
		cout <<UNDERLINED<< i <<RESET "\t";
		for(int j=0;j<n;j++)
		{
			cout << pathsVT[i][j] << "\t";
		}
		cout <<endl;
	}
	//Debug Printing the Paths
	cout << UNDERLINED << endl << "The centralities at " << myProcessId << " : " << endl << RESET;
	for(int i=0;i<totalEdges;i++)
	{
		cout << i << " : " << TotalCB[i] << endl; 
	}
	printMtx.unlock();

    //Totalling with all processes
    centralityAccess.lock();
    for(int i=0;i<totalEdges;i++)
    {
        edgeCentralities[i] += TotalCB[i];
    }
    centralityAccess.unlock();
    

	
	//Sending this calculated value
	//Create socket
	//Send TotalCB[v] to InitialPort + 2 * v + 1
	//   From port InitialPort *
	// for(int i = 0; i<totalEdges; i++)	
	// {
	// 	/*printMtx.lock();
	// 	cout << CYANTXT << myProcessId << " is going to lock " << i << endl << RESET;
	// 	printMtx.unlock();*/
	// 	edgeReceiveMutex.lock();	
	// 		// Declaration of the variables
	// 		int clientsockid,status;
	// 		//Client socket creation
	// 		clientsockid = socket(AF_INET, SOCK_STREAM,0);

	// 		if(clientsockid==-1)
	// 		{
	// 			mtxPrint<<REDTXT <<endl<<"socket creation fail at client "<< myProcessId <<endl<<RESET endMtxPrint;
	// 			edgeReceiveMutex.unlock();
	// 			return 0;
	// 		}
	// 		/*printMtx.lock();
	// 		cout <<CYANTXT<< "Sender socket created for " << i << " by " << myProcessId << endl<<RESET;
	// 		printMtx.unlock();*/

	// 		//specify address
	// 		struct sockaddr_in serverAddr;
	// 		serverAddr.sin_family=AF_INET;
	// 		serverAddr.sin_port=htons(EDGE_INITIAL_PORT+2*i+1);
	// 		serverAddr.sin_addr.s_addr=htonl(INADDR_ANY); 	

	// 		//connection request to the other server
	// 		int connection_status=connect(clientsockid,(struct sockaddr*) &serverAddr,sizeof(serverAddr));	
	// 		if(connection_status==-1)
	// 		{
	// 			mtxPrint<< REDTXT << "socket connection fail at client"<< myProcessId << " for " << myProcessId << RESET << endl endMtxPrint;
	// 			edgeReceiveMutex.unlock();
	// 			return 0;
	// 		}

	// 		// request_msg.process_id= i;
	// 		// request_msg.request_flag=1;
			
	// 		//sending request for giving their correspnding information of neighbors
	// 		//cout<<"Process "<< i<<" sends request to process "<<*t<<" for sending its adjacency list "<<endl;
	// 		/*printMtx.lock();
	// 		cout <<CYANTXT<< "Sending " << TotalCB[i] <<" to " << i << " from " << myProcessId << endl<< RESET;
	// 		printMtx.unlock();*/
	// 		send(clientsockid,&TotalCB[i],sizeof(float),0);

	// 		//Updating Cross check Centralities
	// 		centralityAccessSender.lock();
	// 		edgeCentralitiesSender[i]+=TotalCB[i]/2;
	// 		centralityAccessSender.unlock();

	// 		//receive synch. reply fro the server
	// 		//recv(clientsockid,&reply_msg,sizeof(struct reply),0);
	// 		//cout<<"Process "<< i<<" receives  adjacency list of process "<<reply_msg.process_id<<endl;	

	// 		//close the socket after receiving the reply
	// 	close(clientsockid);
	// 	edgeReceiveMutex.unlock();	
	// 	/*printMtx.lock();
	// 	cout << CYANTXT << myProcessId << " has unlocked " << i << endl << RESET;
	// 	printMtx.unlock();*/	 
	// } 
	
	//Might Need to store the Total distance globally for each v

	mtxPrint << CYANTXT << endl <<"Exiting the Thread for  " << myProcessId << endl << RESET endMtxPrint;//Debug
	return 0;

}

//Used for Getting First Max Edge centrality index in increasing sorted indexes
bool GetFirstMaxOccurencePred(int a)
{
	return edgeCentralities[a]==edgeCentralities[sortedEdgeCentralityIndexes[totalEdges-1]];
}


//Min Stretch Calculation
// x' is nth index in AddedAdjLst
void CalculateMinStretch()
{
	int dummyNodeIndex = n,
	myProcessId = n;

	//Stores predecessor
	vector<vector<int>> pred(n+1);

	//Stores children
	vector<vector<int>> children(n+1);
	vector<vector<int>> oneParentchildren(n+1);

	//Shortest Distance from myProcessId to each other node
	vector<int> dist(n+1,INT_MAX);

	//Visited list for Queue
	vector<bool> isVisited(n+1, false);

	//Queue for BFS to mark distances to each node
	queue<int> BFSQueue;
	
	
	//Calculating the distances with BFS
	BFSQueue.push(myProcessId);
	isVisited[myProcessId] = true;
	dist[myProcessId] = -1;
	///mtxPrint << CYANTXT << "Entering the Queue with "<< BFSQueue.front() << " for  " << myProcessId << endl << "\tQueue Empty : " << !BFSQueue.empty() <<endl <<RESET endMtxPrint;//Debug
	
	while(!(BFSQueue.empty()))
	{
		///mtxPrint << CYANTXT << "Entering the Queue Loop with "<< BFSQueue.front() << " for  " << myProcessId << endl <<RESET endMtxPrint;//Debug
		int addChildrenOf = BFSQueue.front();
		BFSQueue.pop();
		int distToNextNode = dist[addChildrenOf]+1;
		auto adjacentsRow = adj[addChildrenOf];
		///mtxPrint << CYANTXT << "Popped " << addChildrenOf <<" from the Queue for  " << myProcessId << endl<< RESET endMtxPrint;//Debug
		for(auto adjacent:adjacentsRow)
		{
			//Add minimum distance to next visited Node
			dist[adjacent] = min(dist[adjacent], distToNextNode);

			//If Distance set is equal to our current reaching distance to Next node
			// We are on a shortest path to the node
			// Add the current node as pred to Next node
			if(distToNextNode == dist[adjacent])
			{
				pred[adjacent].push_back(addChildrenOf);
				if(pred[adjacent].size()==1)
				{
					children[addChildrenOf].push_back(adjacent);
				}
			}

			//Add to queue only if previously not visited
			if(!isVisited[adjacent])
			{
				BFSQueue.push(adjacent);
				isVisited[adjacent] = true;
			}
		}
	}
	///mtxPrint << CYANTXT << "Exiting the Queue for  " << myProcessId << endl << RESET endMtxPrint;//Debug

	//Debug Printing the pred and distances
	printMtx.lock();
	cout << UNDERLINED << endl << " Printing the distances and pred at dummy Variable : " << endl << RESET;
	for(int i=0;i<=n;i++)
	{
		auto predecessors = pred[i];
		cout << "For " << i << " : " << dist[i]  << " : \t";
		for(auto j:predecessors)
		{
			cout << j << " ";
		}
		cout << endl;
	}
	cout << endl;
	printMtx.unlock();

	//Debug Printing the pred and distances
	printMtx.lock();
	cout << UNDERLINED << endl << " Printing the distances and children at dummy Variable : " << endl << RESET;
	for(int i=0;i<=n;i++)
	{
		auto childrens = children[i];
		cout << "For " << i << " at children distance " << dist[i] + 1  << " : \t";
		for(auto j:childrens)
		{
			cout << j << " ";

		}
		cout << endl;
	}
	cout << endl;
	printMtx.unlock();

	//Copying distances to global for any Further Calculations
	dist.pop_back();
	dist.resize(n);
	minStretchDistances = dist;

	//Creating the Minimum Stretch Adjacency List
	minStretchAdj[children[n][0]].push_back(children[n][1]);
	minStretchAdj[children[n][1]].push_back(children[n][0]);
	for(int i=0;i<n;i++)
	{
		auto childrens = children[i];
		for(auto j:childrens)
		{
			minStretchAdj[i].push_back(j);
			minStretchAdj[j].push_back(i);			
		}
	}

	return;
}

void CalculateMaxSpanningTree()
{
	//Each index is parent of its own
	for(int i=0;i<n;i++)
	{
		disjointArray.push_back(i);
	}
	int maxEdges = n-1, currentEdges=0, i=0;

	//Filling the disjoint set
	while(currentEdges<maxEdges)
	{
        ///mtxPrint << "Entered the loop with " << currentEdges << endl endMtxPrint;
		map<int, pair<int, int>> :: iterator currentEdge;
		currentEdge = indexToEdge.find(sortedEdgeCentralityIndexes[i]);
		int u = (currentEdge->second).first;
		int v = (currentEdge->second).second;
		int u_set = disjointArray[u], v_set = disjointArray[v];
        i++;
		if(u_set==v_set)
		{
			continue;
		}
		for(int j=0;j<n;j++)
		{
			if(disjointArray[j]==v_set)
			{
				disjointArray[j] = u_set;
			}
		}
		maxSpanningTreeAdjacencyList[u].push_back(v);
		maxSpanningTreeAdjacencyList[v].push_back(u);
		currentEdges++;
	}

	return;

}

void CalculateMaxSpanningMinDistances(int myProcessId)
{
    //Total paths to t
    vector<int> TotalPaths(n,0);

	//Stores predecessor
	vector<vector<int>> pred(n);

	//Shortest Distance from myProcessId to each other node
	vector<int> dist(n,INT_MAX);

	//Visited list for Queue
	vector<bool> isVisited(n, false);

	//Queue for BFS to mark distances to each node
	queue<int> BFSQueue;
	
	
	//Calculating the distances with BFS
	BFSQueue.push(myProcessId);
	isVisited[myProcessId] = true;
	dist[myProcessId] = 0;
	///mtxPrint << CYANTXT << "Entering the Queue with "<< BFSQueue.front() << " for  " << myProcessId << endl << "\tQueue Empty : " << !BFSQueue.empty() <<endl <<RESET endMtxPrint;//Debug
	
	while(!(BFSQueue.empty()))
	{
		///mtxPrint << CYANTXT << "Entering the Queue Loop with "<< BFSQueue.front() << " for  " << myProcessId << endl <<RESET endMtxPrint;//Debug
		int addChildrenOf = BFSQueue.front();
		BFSQueue.pop();
		int distToNextNode = dist[addChildrenOf]+1;
		auto adjacentsRow = maxSpanningTreeAdjacencyList[addChildrenOf];
		///mtxPrint << CYANTXT << "Popped " << addChildrenOf <<" from the Queue for  " << myProcessId << endl<< RESET endMtxPrint;//Debug
		for(auto adjacent:adjacentsRow)
		{
			//Add minimum distance to next visited Node
			dist[adjacent] = min(dist[adjacent], distToNextNode);

			//If Distance set is equal to our current reaching distance to Next node
			// We are on a shortest path to the node
			// Add the current node as pred to Next node
			if(distToNextNode == dist[adjacent])
			{
				pred[adjacent].push_back(addChildrenOf);
			}

			//Add to queue only if previously not visited
			if(!isVisited[adjacent])
			{
				BFSQueue.push(adjacent);
				isVisited[adjacent] = true;
			}
		}
	}
	///mtxPrint << CYANTXT << "Exiting the Queue for  " << myProcessId << endl << RESET endMtxPrint;//Debug

	//Debug Printing the pred and distances
	printMtx.lock();
	cout << UNDERLINED << endl << " Printing the pred and distances of Max Spanning Tree at "<< myProcessId <<" : " << endl << RESET;
	for(int i=0;i<n;i++)
	{
		auto predecessors = pred[i];
		cout << "For " << i << " : " << dist[i]  << " : \t";
		for(auto j:predecessors)
		{
			cout << j << " ";
		}
		cout << endl;
	}
	cout << endl;
	printMtx.unlock();

    maxSpanningDistanceAddition.lock();
    maxSpanningTreeDistances[myProcessId] = dist;
    maxSpanningDistanceAddition.unlock();
    return;
}

void CalculateDijkstraMinStretchMinDistances(int myProcessId)
{
    //Total paths to t
    vector<int> TotalPaths(n,0);

	//Stores predecessor
	vector<vector<int>> pred(n);

	//Shortest Distance from myProcessId to each other node
	vector<int> dist(n,INT_MAX);

	//Visited list for Queue
	vector<bool> isVisited(n, false);

	//Queue for BFS to mark distances to each node
	queue<int> BFSQueue;
	
	
	//Calculating the distances with BFS
	BFSQueue.push(myProcessId);
	isVisited[myProcessId] = true;
	dist[myProcessId] = 0;
	///mtxPrint << CYANTXT << "Entering the Queue with "<< BFSQueue.front() << " for  " << myProcessId << endl << "\tQueue Empty : " << !BFSQueue.empty() <<endl <<RESET endMtxPrint;//Debug
	
	while(!(BFSQueue.empty()))
	{
		///mtxPrint << CYANTXT << "Entering the Queue Loop with "<< BFSQueue.front() << " for  " << myProcessId << endl <<RESET endMtxPrint;//Debug
		int addChildrenOf = BFSQueue.front();
		BFSQueue.pop();
		int distToNextNode = dist[addChildrenOf]+1;
		auto adjacentsRow = minStretchAdj[addChildrenOf];
		///mtxPrint << CYANTXT << "Popped " << addChildrenOf <<" from the Queue for  " << myProcessId << endl<< RESET endMtxPrint;//Debug
		for(auto adjacent:adjacentsRow)
		{
			//Add minimum distance to next visited Node
			dist[adjacent] = min(dist[adjacent], distToNextNode);

			//If Distance set is equal to our current reaching distance to Next node
			// We are on a shortest path to the node
			// Add the current node as pred to Next node
			if(distToNextNode == dist[adjacent])
			{
				pred[adjacent].push_back(addChildrenOf);
			}

			//Add to queue only if previously not visited
			if(!isVisited[adjacent])
			{
				BFSQueue.push(adjacent);
				isVisited[adjacent] = true;
			}
		}
	}
	///mtxPrint << CYANTXT << "Exiting the Queue for  " << myProcessId << endl << RESET endMtxPrint;//Debug

	//Debug Printing the pred and distances
	printMtx.lock();
	cout << UNDERLINED << endl << " Printing the pred and distances of Dijkstra Min Stretch Tree at "<< myProcessId <<" : " << endl << RESET;
	for(int i=0;i<n;i++)
	{
		auto predecessors = pred[i];
		cout << "For " << i << " : " << dist[i]  << " : \t";
		for(auto j:predecessors)
		{
			cout << j << " ";
		}
		cout << endl;
	}
	cout << endl;
	printMtx.unlock();

    minStretchDistanceAddition.lock();
    ///mtxPrint << "Adding to the dijkstra distance at " << myProcessId << endl; printMtx.unlock();
    minStretchDistanceAll[myProcessId] = dist;
    minStretchDistanceAddition.unlock();
    return;
}


int main()
{
	//Setting iterators
	int i,j,input;

	//Configuring Input File stream
	ifstream fin;
	fin.open("InputPythonComp.txt");

	//Setting Ports for threads
	fin>>NODE_PORT;
	fin >> EDGE_INITIAL_PORT;

	//Input of number of process
	/*cout<<"Enter the number of processes"<<endl;
	fin>>n;
	cout << n << endl << endl;*/

	//Creating adjacency list of processes
	//adj.resize(n);
	totalEdges=0;

	//Making line by line input String Stream
	string adjacentString;
    getline(fin, adjacentString); //Dummy String Input

	//Taking input of edges
	cout<<"<=========  Enter the neighbor vertices corresponding to each vertex =========>"<<endl<<endl;

	i=0;
    int edgeIndex = 0;
	while(getline(fin, adjacentString))
	{
		PortsToAssign.push_back(0);
		adj.push_back({});
		cout<<"Enter neighbors of vertex "<<i<<endl;
		istringstream sin(adjacentString);
        while(sin >> input)
		{
			cout << input << " ";
			if(input>i)
			{
				//Counting ports required for each edge for each process
				PortsToAssign[i]++;
                
                //Edge to Index and back Mapping
                EdgeIndexes.push_back({i,input}); //Not required
                edgeToIndex.insert(make_pair(make_pair(i, input), edgeIndex));
		        indexToEdge.insert(make_pair(edgeIndex, make_pair(i, input)));
                edgeIndex++;

			}
			totalEdges++;
			//Creating adjacency list
			adj[i].push_back(input);
		}
		i++;
		cout << endl;
		////cout << "\t" << i << " is responsible for " << PortsToAssign[i] << " edges" << endl << endl;
	}
	n = i;
	
	//Setting the global pointer of number of edges each node is responsible for

	
	//As the graph is undirected, ach edge is counted twice
	totalEdges = totalEdges/2;
	cout << endl <<"Total edges : " << totalEdges << endl << endl;

	
	//Creating Placeholders for mutexes of each receiver of processes and edges
	vector<mutex> nProcessMtx(n);
	vector<mutex> edgeProcessmtx(totalEdges);
	processRcvMtx.swap(nProcessMtx);
	edgeRcvMtx.swap(edgeProcessmtx);

    //Resizing the distances vectors
    originalDistances.resize(n);
    minStretchDistanceAll.resize(n,vector<int>(n));
    maxSpanningTreeDistances.resize(n,vector<int>(n));


    //Declaration of receive thread and send thread corresponding to the process i
   	/*thread receive_thread[n];
	thread send_thread[n];
    
	//Testing the global edge_Port responsibilty array
	for(i=0;i<n;i++)
	{
		cout << i << " is responsible for " << PortsToAssign[i] << endl;
	}
	cout << endl;

	// Creating receive thread
    for(j=0;j<n;j++)
    {
    	  receive_thread[j] = thread(receiver,j,adj[j],NODE_PORT+2*j+1);  
    }


	//Flushing IO and intoducing time delay to allow receivers to get ready
	printMtx.lock();
	cout.flush();
	printMtx.unlock();
	usleep(10000);

    
	//creating sender thread
	for(j=0;j<n;j++)
	{
		 send_thread[j] = thread(sender,j,adj[j],NODE_PORT+2*j);  
	}

	
	// joining receive thread to the main function
	for(j=0;j<n;j++)
	{
		receive_thread[j].join();
	}

	
	//joining send thread to the main function
	for(j=0;j<n;j++)
	{
		send_thread[j].join();
	}


	cout<<endl<<"Done Creating Edge Graph"<<endl;


	//Trial of Global Edge Graph
	cout <<UNDERLINED<< endl <<" Global Edge Graph : " << endl<<RESET;
	for(i=0;i<totalEdges;i++)
	{
		cout << "head";
		for(auto j:globalEdgeGraph[i])
		{
			cout << "->" << j.u << "+" << j.v;
		}
		cout << endl;
	}


	//Maps the edges to an index
	cout<<"\n Mapping of edges to a number" <<endl;
	for(i=0;i<totalEdges;i++)
	{
		auto q=globalEdgeGraph[i][0];
		edgeToIndex.insert(make_pair(make_pair(q.u,q.v), i));
		indexToEdge.insert(make_pair(i, make_pair(q.u,q.v)));
	}
    */

	//Printing out the Map
	cout << UNDERLINED << endl <<" Map is as follows : " << endl << RESET;
	map<pair<int, int>, int> :: iterator itr;
	for(itr = edgeToIndex.begin(); itr != edgeToIndex.end(); itr++)
	{
		cout<<(itr->first).first<<"+"<<(itr->first).second<<"    "
		<<itr->second<<endl;
	}


	//Converting the graph into a mapped graph
	/*vector<vector<int>> convertedEdgeGraph;
	for(i=0;i<totalEdges;i++)
	{
		vector<int> temp;
		for(auto j:globalEdgeGraph[i])
		{
			itr = edgeToIndex.find(make_pair(j.u,j.v)) ;
			temp.push_back(itr->second);
		}
		convertedEdgeGraph.push_back(temp);
		temp.clear();//Debug Printing the Paths
	}
	convertedEdgeGraphGlobal.swap(convertedEdgeGraph);

	//Printing the global converted graph
	cout << UNDERLINED << endl <<" Global Converted Graph : " << endl <<RESET;
	for(auto i:convertedEdgeGraphGlobal)
	{
		cout << "head" ;
		for(auto j:i)
		{
			cout << " ->" << j;
		}
		cout << endl;
	}
	cout << endl;
    */

	//Vector for storing Centrality
	//vector<float> edgeCentralityMain(totalEdges,0);
	edgeCentralities.resize(totalEdges,0);

    //Edge centralities at Calculating(Sender Side)
	edgeCentralitiesSender.resize(totalEdges,0);

	//Creating threads for calculating and receiving edge centralities
	//thread centralityReceiver[totalEdges];
	thread centralityCalculator[n];

    //Lets do it Globally
	// Creating receiver thread
    /*for(j=0;j<totalEdges;j++)
    {
    	  centralityReceiver[j] = thread(CentralityReceiver,j,convertedEdgeGraphGlobal[j]);  
    }

	
	//Flushing IO and intoducing time delay to allow receivers to get ready
	printMtx.lock();
	cout.flush();
	printMtx.unlock();
	usleep(20000);
    */
    
	//creating sender thread
	for(j=0;j<n;j++)
	{
		//mtxPrint << " Created thread " << j << endl endMtxPrint;
		 centralityCalculator[j] = thread(CentralitySender,j);  
	}

	// joining receive thread to the main function
	/*for(j=0;j<totalEdges;j++)
	{
		centralityReceiver[j].join();
	}*/

	//joining send thread to the main function
	for(j=0;j<n;j++)
	{
		centralityCalculator[j].join();
	}

	//Printing out all edge Centralities
	mtxPrint << endl << UNDERLINED << " Total Edge Centralities : " << endl << RESET endMtxPrint;
	for(i=0;i<totalEdges;i++)
	{
		mtxPrint << i << " : " 
		<< edgeCentralities[i] 
		//<< " = " << edgeCentralitiesSender[i]		//CrossChecking with Sender
		<<endl endMtxPrint;
	}

    
	//Stable sorting the edge centralities
	sortedEdgeCentralityIndexes.resize(totalEdges);
	//Creating a index sequenced vector that will correspond to each centrality at index of value
	iota(sortedEdgeCentralityIndexes.begin(),sortedEdgeCentralityIndexes.end(),0);

	//Stable sorting in decreasing order
	stable_sort(sortedEdgeCentralityIndexes.begin(),
				sortedEdgeCentralityIndexes.end(),
				IndexesGreaterEdgeCentrality
				);

	mtxPrint << endl << UNDERLINED << " Stable Sorted Edge Centralities : " << endl << RESET endMtxPrint;
	for(i=0;i<totalEdges;i++)
	{
		mtxPrint << i << " : " 
		<< sortedEdgeCentralityIndexes[i] << " = " 
		<< edgeCentralities[sortedEdgeCentralityIndexes[i]] 
		<<endl 
		endMtxPrint;
	}
	
	//Maximum Last Occurence
	firstMaxEdgeCentralityIndex = sortedEdgeCentralityIndexes[0];
	int firstMaxEdgeCentrality = edgeCentralities[firstMaxEdgeCentralityIndex];
	
	//For last occurence (O(totalEdges) Time)
	//mtxPrint << "\nIn Adjacent Find : " << endl endMtxPrint;
	auto maxEdgeCentralityPointer = adjacent_find(sortedEdgeCentralityIndexes.begin(),
												  sortedEdgeCentralityIndexes.end(),
												  [](int a, int b) -> bool 
												  {
													  //mtxPrint<< edgeCentralities[a] << " " << edgeCentralities[b] << endl endMtxPrint;
													  return edgeCentralities[a]!=edgeCentralities[b];
													  }
												 );
	
	int lastMaxEdgeCentralityIndex = *maxEdgeCentralityPointer,
		lastMaxEdgeCentrality = edgeCentralities[*maxEdgeCentralityPointer];

	mtxPrint << endl << UNDERLINED << "Max Centrality (Index and Centrality) : " << RESET << endl endMtxPrint;
	mtxPrint <<BOLDED<<"First Occurence :" << RESET << " Index : " << firstMaxEdgeCentralityIndex << "  Centrality : " << firstMaxEdgeCentrality <<endl endMtxPrint;
	mtxPrint <<BOLDED<<"Last  Occurence :" << RESET << " Index : " << lastMaxEdgeCentralityIndex  << "  Centrality : " << lastMaxEdgeCentrality  <<endl endMtxPrint;

	//Adding virtual x' to highest centrality edge 
	//  Positoned at index(starting at 0) n 
	adj.push_back({EdgeIndexes[firstMaxEdgeCentralityIndex][0],EdgeIndexes[firstMaxEdgeCentralityIndex][1]});

	//Printing x' added Global Edge Graph
	printMtx.lock();
	cout << UNDERLINED << endl << " After adding x' indexed at " << n << ", New Global Graph : " << RESET << endl;
	for(int i=0; i<=n; i++)
	{
		cout << BOLDED << i << " : "  << RESET ;
		for(auto j:adj[i])
		{
			cout << " ->" << j ;
		}
		cout << endl;
	}
	printMtx.unlock();


	//Resizing the MinStretch Adj List
	minStretchAdj.resize(n);
	maxSpanningTreeAdjacencyList.resize(n);
	CalculateMinStretch();
	CalculateMaxSpanningTree();

    //Printing All pair shortest distances
	printMtx.lock();
	cout << UNDERLINED << endl << " All Pair Distances : " << RESET << endl;
    cout << "\t" ;
    for(int i=0; i<n; i++)
    {
        cout << " " << BOLDED << UNDERLINED << i << RESET;
    }
    cout << endl;
	for(int i=0; i<n; i++)
	{

		cout << BOLDED << i << " :\t"  << RESET ;
		for(auto j:originalDistances[i])
		{
			cout << " " << j ;
		}
		cout << endl;
	}
	printMtx.unlock();

	//Printing the final min Stretch Adj List
	printMtx.lock();
	cout << UNDERLINED << endl << " Final Min Stretch Adj List : " << RESET << endl;
	for(int i=0; i<n; i++)
	{
		cout << BOLDED << i << " : "  << RESET ;
		for(auto j:minStretchAdj[i])
		{
			cout << " ->" << j ;
		}
		cout << endl;
	}
	printMtx.unlock();

	//Printing the final Max Edge Centrality Spanning Tree Adj List
	printMtx.lock();
	cout << UNDERLINED << endl << " Final Max Edge Centrality Spanning Tree Stretch Adj List : " << RESET << endl;
	for(int i=0; i<n; i++)
	{
		cout << BOLDED << i << " : "  << RESET ;
		for(auto j:maxSpanningTreeAdjacencyList[i])
		{
			cout << " ->" << j ;
		}
		cout << endl;
	}
	printMtx.unlock();

	//Printing the final min Stretch Adj List
	printMtx.lock();
	cout << UNDERLINED << endl << " Final Min Stretch Adj Distances from virtual root x' : " << RESET << endl;
	for(int i=0; i<n; i++)
	{
		cout << BOLDED << i << " : "  << RESET
			 << minStretchDistances[i] << endl;		
	}
	printMtx.unlock();

    //Calculating distances for final Stretch Calculation
    thread minStretchDistanceCalculatorThread[n];
    thread maxSpanningDistanceCalculatorThread[n];
    
    //Calculating and attaching threads
    for(j=0;j<n;j++)
	{
		minStretchDistanceCalculatorThread[j] = thread(CalculateDijkstraMinStretchMinDistances,j);  
        maxSpanningDistanceCalculatorThread[j] = thread(CalculateMaxSpanningMinDistances,j);  
	}

    //Joining the threads after both called so that both distance calculate at same time 
    for(j=0;j<n;j++)
	{
		minStretchDistanceCalculatorThread[j].join();
        maxSpanningDistanceCalculatorThread[j].join();
	}

    //Printing All pair shortest distances Dijkstra
	printMtx.lock();
	cout << UNDERLINED << endl << " All Pair Distances (Dijkstra) : " << RESET << endl;
    cout << "\t" ;
    for(int i=0; i<n; i++)
    {
        cout << " " << BOLDED << UNDERLINED << i << RESET;
    }
    cout << endl;
	for(int i=0; i<n; i++)
	{

		cout << BOLDED << i << " :\t"  << RESET ;
		for(auto j:minStretchDistanceAll[i])
		{
			cout << " " << j ;
		}
		cout << endl;
	}
	printMtx.unlock();


    //Printing All pair shortest distances
	printMtx.lock();
	cout << UNDERLINED << endl << " All Pair Distances (Spanning Tree) : " << RESET << endl;
    cout << "\t" ;
    for(int i=0; i<n; i++)
    {
        cout << " " << BOLDED << UNDERLINED << i << RESET;
    }
    cout << endl;
	for(int i=0; i<n; i++)
	{

		cout << BOLDED << i << " :\t"  << RESET ;
		for(auto j:maxSpanningTreeDistances[i])
		{
			cout << " " << j ;
		}
		cout << endl;
	}
	printMtx.unlock();


    //Calculating Stretch of Both Graphs
    dijkstraStretchAll.resize(n,vector<float>(n));
    maxSpanningTreeStretchAll.resize(n,vector<float>(n));

    //Getting Dijkstra method stretches
    for(int i = 0; i<n; i++)
    {
        for(int j = 0;j < n; j++)
        {
            //Division by zero reulting in nan
            if(i==j)
            {
                dijkstraStretchAll[i][j] = 0;
                continue;
            }

            //Dividing to get the Stretch
            dijkstraStretchAll[i][j] = (float)minStretchDistanceAll[i][j]/(float)originalDistances[i][j];
        }
    }

    //Getting Max Spanning tree Stretch
    for(int i = 0; i<n; i++)
    {
        for(int j = 0;j < n; j++)
        {
            //Division by zero reulting in nan
            if(i==j)
            {
                maxSpanningTreeStretchAll[i][j] = 0;
                continue;
            }

            //Dividing to get the Stretch
            maxSpanningTreeStretchAll[i][j] = (float)maxSpanningTreeDistances[i][j]/(float)originalDistances[i][j];
        }
    }


    //Printing All pair shortest distances
	printMtx.lock();
	cout << UNDERLINED << endl << " All Pair Stretch (Dijkstra) : " << RESET << endl;
    //cout << "" ;
    for(int i=0; i<n; i++)
    {
        cout << "\t" << BOLDED << UNDERLINED << i << RESET;
    }
    cout << endl;
	for(int i=0; i<n; i++)
	{

		cout << BOLDED << i << " :"  << RESET ;
		for(auto j:dijkstraStretchAll[i])
		{
			cout << "\t" << j ;
		}
		cout << endl;
	}
	printMtx.unlock();


    //Printing All pair shortest distances
	printMtx.lock();
	cout << UNDERLINED << endl << " All Pair Stretch (Spanning Tree) : " << RESET << endl;
    //cout << "\t" ;
    for(int i=0; i<n; i++)
    {
        cout << "\t" << BOLDED << UNDERLINED << i << RESET;
    }
    cout << endl;
	for(int i=0; i<n; i++)
	{

		cout << BOLDED << i << " :"  << RESET ;
		for(auto j:maxSpanningTreeStretchAll[i])
		{
			cout << "\t" << j ;
		}
		cout << endl;
	}
	printMtx.unlock();

    //Calculating Final Stretches
    for(int i = 0; i<n; i++)
    {
        for(int j = 0;j < n; j++)
        {
            //Dividing to get the Stretch
            maxSpanningTreeStretch += maxSpanningTreeStretchAll[i][j];
        }
    }
    maxSpanningTreeStretch = maxSpanningTreeStretch/(float)totalEdges;

    //Calculating Final Stretches
    for(int i = 0; i<n; i++)
    {
        for(int j = 0;j < n; j++)
        {
            //Dividing to get the Stretch
            dijkstraStretch += dijkstraStretchAll[i][j];
        }
    }
    dijkstraStretch = dijkstraStretch/(float)totalEdges;

    //Printing out the final Stretches
    printMtx.lock();
    cout <<endl << MAGENTATXT "Dijkstra Method's Stretch Factor : " << BOLDED << dijkstraStretch << RESET << endl;
    cout <<endl << MAGENTATXT "Max Centrality Spanning Tree's Stretch Factor : " << BOLDED << maxSpanningTreeStretch << RESET << endl;
    printMtx.unlock();

    //End Line
    printMtx.lock();
    cout  << endl;
    printMtx.unlock();

	return 0;
}
