//Zachary Irons
//Matt Stout

#include <stdlib.h>
#include <string>
using namespace std;

//Node class
//Holds all the information about the job
class Node {
public:
	bool head;
	Node* next;
	int job_num;
	int priority;
	int memory;
	int devices;
	int alloc_devs;
	int req_devs;
	bool dev_req_met;
	int arr_time;
	int run_time;
	int remain_time;
	int time_completed;	
	bool completed;
	bool rejected;
	bool bankers;

	Node (bool head, Node* next);
};

void add_front(Node* head, Node* n);
void add_end(Node* head, Node* n);
Node* remove(Node* head, int job_num);
