//Zachary Irons
//Matt Stout

#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <string>
#include <cstring>
#include <cmath>
#include <vector>
#include <sstream>
#include "Node.h"

using namespace std;

//simulating: 	true when simulation is running
//				false when simulation complete
bool simulating = true;

//system_time:		the time of the current simulation
//instruction_time:	the time of the current instruction
//instruction_num:	the number of the current instruction
int system_time = 0;
int instruction_time;
int instruction_num = 0;

//input_complete:	true when instructions are completed
//					false when there are more instructions
//num_instructions:	the number of total instructions
bool input_complete = false;
int num_instructions;

//memory:			the total memory of the system
//avail_mem:		the available memory of the system
//devices:			the total number of devices
//avail_devs:		the number of available devices
//quantum:			the size of the quantum
//quantum_slice:	the current amount of time into the quantum
int memory = 0;
int avail_mem = 0;
int devices = 0;
int avail_devs = 0;
int quantum = 0;
int quantum_slice = 0;

//function declaration
int get_instruction_time(string input);
void instruction_decode(string input, Node *job_list, Node *submit, Node *wait, Node *hold1, Node *hold2, Node *ready, Node *run, Node *complete);
void display(Node *job_list, Node *submit, Node *hold1, Node *hold2, Node *ready, Node *run, Node *wait, Node *complete);
void make_system(vector<string> array);
void make_job(vector<string> array, Node *job_list, Node *submit);
void make_request(vector<string> array, Node *job_list, Node *cpu, Node *rq, Node *wq);
void release(vector<string> array, Node *job_list, Node *cpu, Node *wq, Node *rq);
void update_sq(Node *job_list, Node *sq, Node *hq1, Node *hq2);
void update_wq(Node *job_list, Node *wq, Node *rq);
void update_hq1(Node *job_list, Node *hq1, Node *rq);
void update_hq2(Node *job_list, Node *hq2, Node *rq);
void update_rq(Node *job_list, Node *rq, Node *cpu);
void update_cpu(Node *job_list, Node *wq, Node *hq1, Node *hq2, Node *rq, Node *cpu, Node *cq);
bool bankers(Node* rq, int req_devs);
void update_job_completion(Node *job_list, Node *job, bool comp);
void update_job_alloc_devs(Node* job_list, Node* job, int alloc_devs);
void update_job_time_completed(Node* job_list, Node* job, int time_completed);
void update_job_rejection(Node* job_list, Node* job, bool rej);

int main () {
	string line;
	string instruction_list[100];
	ifstream my_file ("test.txt");
	
	if (my_file.is_open()) {
		int i = 0;
		
		while (getline(my_file, line)) {
			instruction_list[i] = line;
			i++;
		}
		num_instructions = i;
		my_file.close();
	} else {
		cout << "That file does not exist.";
		return 0;
	}

	//creation of all the queues
	Node *job_list = new Node(true, NULL);
	Node *sq = new Node(true, NULL);
	Node *hq1 = new Node(true, NULL);
	Node *hq2 = new Node(true, NULL);
	Node *rq = new Node(true, NULL);
	Node *wq = new Node(true, NULL);
	Node *cpu = new Node(true, NULL);
	Node *cq = new Node(true, NULL);
	
	while (simulating) {
		string current = instruction_list[instruction_num];

		if (!input_complete) {
			instruction_time = get_instruction_time(current);
		}

		if (system_time >= instruction_time) {
			
			instruction_decode(current, job_list, sq, wq, hq1, hq2, rq, cpu, cq);
			instruction_num++;
			
			if (instruction_num >= num_instructions) {
				input_complete = true;
			}
		}

		update_sq(job_list, sq, hq1, hq2);

		update_wq(job_list, wq, rq);

		update_hq1(job_list, hq1, rq);
		update_hq2(job_list, hq2, rq);

		update_rq(job_list, rq, cpu);

		update_cpu(job_list, wq, hq1, hq2, rq, cpu, cq);

		if (quantum_slice == quantum) {
			quantum_slice = 0;
			if (rq->next != NULL) {
				Node *n1 = remove(cpu, cpu->next->job_num);
				add_end(rq, n1);
				Node *n2 = remove(rq, rq->next->job_num);
				add_end(cpu, n2);
			}
		}
		system_time++;

		if (cpu->next == NULL && input_complete) {
			simulating = false;
		}
	}
	return 1;
}

//get_instruction_time: gets the time of the current instruction
int get_instruction_time(string line){
	vector<string> array;
	std::size_t pos = 0, found;
	while((found = line.find_first_of(' ', pos)) != std::string::npos){
		array.push_back(line.substr(pos, found-pos));
		pos = found+1;
	}
	array.push_back(line.substr(pos));

	vector<string>:: iterator i;
	int y = 0;
	int time;

	for(i = array.begin(); i!=array.end(); ++i){
		if (y == 1){
			istringstream buffer(*i);
			buffer >> time;
		}
		y++;
	}
	return time;
}

//instruction_decode: determines the type of instruction and executes accordingly
void instruction_decode(string line, Node *job_list, Node *sq, Node *wq, Node *hq1, Node *hq2, Node *rq, Node *cpu, Node *cq){	
	vector<string> array;
	std::size_t pos = 0, found;
	while((found = line.find_first_of(' ', pos)) != std::string::npos){
		array.push_back(line.substr(pos, found-pos));
		pos = found+1;
	}
	array.push_back(line.substr(pos));

	vector<string>:: iterator i;
	int y = 0;
	char instruction = 'X';
	
	i = array.begin();
	
	istringstream buffer (*i);
	buffer >> instruction;
	
	array.erase(i);
	
	if (instruction == 'C') {
		make_system(array);
	} else if (instruction == 'A') {
		make_job(array, job_list, sq);
	} else if (instruction == 'Q') {
		make_request(array, job_list, cpu, rq, wq);
	} else if (instruction == 'L') {
		release(array, job_list, cpu, wq, rq);
	} else if (instruction == 'D') {
		display(job_list, sq, hq1, hq2, rq, cpu, wq, cq);
	}
}

//display: outputs the contents of the system to a JSON file
void display(Node *job_list, Node *sq, Node *hq1, Node *hq2, Node *rq, Node *cpu, Node *wq, Node *cq) {	
	
	ofstream json_file;
	
	string file_name = "json_output_";
	
	ostringstream time;
	time << system_time;
	string time_string = time.str();
	
	file_name += time_string;
	file_name += ".json";
	
	char* file_time = (char*) file_name.c_str();
	
	json_file.open(file_time);
	
	json_file << "{\"readyq\": [";
	
    Node* temp = rq;
    while(temp->next != NULL){
        json_file << temp->next->job_num;
        temp = temp->next;
		
		if (temp->next != NULL){
			json_file  << ", ";
		}
    }
    json_file << "], ";

    json_file << "\"current_time\": " << system_time;
    json_file << ", \"total_memory\": " << memory;
    json_file << ", \"available_memory\": " << avail_mem;
    json_file << ", \"total_devices\": " << devices;
    
    temp = cpu;
	if (cpu->next != NULL){
		json_file << ", \"running\": ";
		json_file << temp->next->job_num;
	}
	
    json_file << ", \"submitq\": [";
    temp = sq;
    while(temp->next != NULL){
        json_file << temp->next->job_num;
        temp = temp->next;
		
		if (temp->next != NULL){
			json_file  << ", ";
		}
    }
    json_file << "]";
	
    json_file << ", \"holdq2\": [";
    temp = hq2;
    while(temp->next != NULL){
        json_file << temp->next->job_num;
        temp = temp->next;
		
		if (temp->next != NULL){
			json_file  << ", ";
		}
    }
    json_file << "]";
	
    json_file << ", \"job\": [";
    temp = job_list;
    while(temp->next != NULL){
		
		if(!temp->next->rejected){
			json_file << "{" << "\"arrival time\": " << temp->next->arr_time;
			if(!temp->next->completed){
				json_file << ", \"devices_allocated\": " << temp->next->alloc_devs;
			}
			json_file << ", \"id\": " << temp->next->job_num;
			json_file << ", \"remaining_time\": " << temp->next->remain_time;
			if(temp->next->completed){
				json_file << ", \"completion_time\": " << temp->next->time_completed;
			}
			if (temp->next != NULL && !temp->rejected){
				json_file << "}, ";
			}else{
				json_file << "}";
			}
		}		
		temp = temp->next;
    }
    json_file << "]";  
    
    json_file << ", \"holdq1\": [";
    temp = hq1;
    while(temp->next != NULL){
        json_file << temp->next->job_num;
        temp = temp->next;
		
		if (temp->next != NULL){
			json_file  << ", ";
		}
    }
    json_file << "]";

    json_file << ", \"available_devices\": " << avail_devs;
    json_file << ", \"quantum\": " << quantum;


    json_file << ", \"completeq\": [";
    temp = cq;
    while(temp->next != NULL){
        json_file << temp->next->job_num;
        temp = temp->next;
		
		if (temp->next != NULL){
			json_file  << ", ";
		}
    }
    json_file << "]";

    json_file << ", \"waitq\": [";
    temp = wq;
    while(temp->next != NULL){
        json_file << temp->next->job_num << " ,";
        temp = temp->next;
		
		if (temp->next != NULL){
			json_file  << " ,";
		}
    }
    json_file << "]}" << endl;
}

//make_system: initializes the system to the instruction
void make_system(vector<string> array){	
 	vector<string>:: iterator i;
	int y = 0;
	int m, d, q;

	for(i = array.begin(); i!=array.end(); ++i){
		if (y == 1){
			istringstream buffer((*i).substr(2));
			buffer >> m;
			memory = m;
			avail_mem = m;
		}else if (y == 2){
			istringstream buffer((*i).substr(2));
			buffer >> d;
			devices = d;
			avail_devs = d;
		}else if (y == 3){
			istringstream buffer((*i).substr(2));
			buffer >> q;
			quantum = q;
		}
		y++;
	}
}

//make_job: creates a job according to the instruction
void make_job(vector<string> array, Node *job_list, Node *sq) {	
	vector<string>:: iterator i;
	int y = 0;
	int j;
	int m;
	int d;
	int r;
	int p;
	
	Node *job = new Node(false, NULL);
	Node *copy = new Node(false, NULL);
	job->arr_time = instruction_time;
	copy->arr_time = instruction_time;

	for(i = array.begin(); i!=array.end(); ++i){
		if (y == 1){
			istringstream buffer((*i).substr(2));
			buffer >> j;
			job->job_num = j;
			copy->job_num = j;
		}else if (y == 2){
			istringstream buffer((*i).substr(2));
			buffer >> m;
			job->memory = m;
			copy->memory = m;
		}else if (y == 3){
			istringstream buffer((*i).substr(2));
			buffer >> d;
			job->devices = d;
			copy->devices = d;
			job->alloc_devs = 0;
			copy->alloc_devs = 0;
		}else if (y == 4){
			istringstream buffer((*i).substr(2));
			buffer >> r;
			job->run_time = r;
			copy->run_time = r;
			job->remain_time = r;
			copy->remain_time = r;
		}else if (y == 5){
			istringstream buffer((*i).substr(2));
			buffer >> p;			
			job->priority = p;
			copy->priority = p;
		}
		y++;
	}
	add_end(sq, job);
	add_end(job_list, copy);
}

//make_request:								creates a request according to the instruction
void make_request(vector<string> array, Node *job_list, Node *cpu, Node *rq, Node *wq) {	
	vector<string>:: iterator i;
	int j = 0;
	int d = 0;
	int y = 0;
	
	quantum_slice = 0;

	for(i = array.begin(); i!=array.end(); ++i){
		if (y == 1){
			istringstream buffer((*i).substr(2));
			buffer >> j;
		}else if (y == 2){
			istringstream buffer((*i).substr(2));
			buffer >> d;
		}
		y++;
	}	
	cpu->next->req_devs = d;
	
	if (j == cpu->next->job_num && cpu->next->req_devs <= (cpu->next->devices - cpu->next->alloc_devs)){
		if (bankers(rq, d)){
			cpu->next->dev_req_met = true;
			cpu->next->alloc_devs += d;
			
			update_job_alloc_devs(job_list, cpu->next, cpu->next->alloc_devs);
			
			avail_devs -= d;
			
			Node *n1 = remove(cpu, cpu->next->job_num);
			add_end(rq, n1);
			
			Node *n2 = remove(rq, rq->next->job_num);
			add_end(cpu, n2);
		}else{
			cpu->next->dev_req_met = false;
			Node *n1 = remove(cpu, cpu->next->job_num);
			add_end(wq, n1);

			Node *n2 = remove(rq, rq->next->job_num);
			add_end(cpu, n2);
		}
	}
}

//release: attempts to release devices according to the instruction
void release(vector<string> array, Node *job_list, Node *cpu, Node *wq, Node *rq) {
	quantum_slice = 0;
	int j = 0;
	int d = 0;
	
	vector<string>:: iterator i;
	int y = 0;

	for(i = array.begin(); i!=array.end(); ++i){
		if (y == 1){
			istringstream buffer((*i).substr(2));
			buffer >> j;
		}else if (y == 2){
			istringstream buffer((*i).substr(2));
			buffer >> d;
		}
		y++;
	}
	
	if (j == cpu->next->job_num && cpu->next->dev_req_met && cpu->next->alloc_devs >= d){
		avail_devs += d;
		cpu->next->alloc_devs -= d;
		
		update_job_alloc_devs(job_list, cpu->next, cpu->next->alloc_devs);

		update_wq(job_list, wq, rq);
	}
}

//update_sq: updates the submit queue
void update_sq(Node *job_list, Node *sq, Node *hq1, Node *hq2){
	if (sq->next != NULL) {
		Node *temp = sq;
		while(temp != NULL){
			if(temp->head == false){
				if(temp->devices > devices || temp->memory > memory ){
					remove(sq, temp->job_num);
					temp->rejected = true;
					update_job_rejection(job_list, temp, true);
				}
				Node *n = remove(sq, temp->job_num);
				if (n != NULL) {
					if(n->priority == 1){
						add_front(hq1, n);
					}else{
						add_front(hq2, n);
					}
				}
			}
			temp = temp->next;
		}
	}
}

//update_wq: updates the wait queue
void update_wq(Node *job_list, Node *wq, Node *rq) {
	if (wq->next != NULL) {
		Node *temp = wq;
		while (temp != NULL) {
			if (temp->head == false) {
				if (temp->devices <= avail_devs) {
					Node *n = remove(wq, temp->job_num);
					add_end(rq, n);
				}
			}
			temp = temp->next;
		}
	}
}

//update_hq1: update hold queue 1
void update_hq1(Node *job_list, Node *hq1, Node *rq) {
	if (hq1->next != NULL) {
		Node *temp = hq1;
		int shortest_job_num = 0;
		int shortest_job_time = 999; 
		
		while (temp != NULL) {
			if (temp->head == false) {
				if (temp->memory <= avail_mem && temp->devices <= avail_devs) {
					if (temp->run_time < shortest_job_time) {
						shortest_job_time = temp->run_time;
						shortest_job_num = temp->job_num;
					}
				}
			}
			temp = temp->next;
		}
		if (shortest_job_num != 0){
			Node *n = remove(hq1, shortest_job_num);
			add_end(rq, n);
			avail_mem -= n->memory;
		}
	}
}

//update_hq2: updates hold queue 2
void update_hq2(Node *job_list, Node *hq2, Node *rq) {
	if (hq2->next != NULL) {
		Node *temp = hq2;
		
 		while (temp->next != NULL) {
			temp = temp->next;
		}
		
		if (temp->memory <= avail_mem && temp->devices <= avail_devs) {
			Node *n = remove(hq2, temp->job_num);
			add_end(rq, n);
			avail_mem = avail_mem - n->memory;
		}
	}
}

//update_rq: updates the ready queue
void update_rq(Node *job_list, Node *rq, Node *cpu) {
	if (rq->next != NULL) {
		if (cpu->next == NULL) {
			Node *n = remove(rq, rq->next->job_num);
			add_front(cpu, n);
			quantum_slice = 0;
		}
	}
}

//update_cpu: updates the cpu
void update_cpu(Node *job_list, Node *wq, Node *hq1, Node *hq2, Node *rq, Node *cpu, Node *cq) {
	if (cpu->next != NULL) {
		quantum_slice++;
		cpu->next->remain_time--;

		Node *temp = job_list;
		while (temp->job_num != cpu->next->job_num) {
			temp = temp->next;
		}
		temp->remain_time--;

		if (cpu->next->remain_time == 0) {
			cpu->next->time_completed = system_time + 1;
			
			update_job_time_completed(job_list, cpu->next, cpu->next->time_completed);
			
			avail_mem += cpu->next->memory;

			if (cpu->next->dev_req_met) {
				avail_devs += cpu->next->alloc_devs;
			}

			Node *temp = job_list;
			while (temp->job_num != cpu->next->job_num) {
				temp = temp->next;
			}
			temp->time_completed = system_time + 1;

			Node *transfer = remove(cpu, cpu->next->job_num);
			add_end(cq, transfer);

			update_job_completion(job_list, transfer, true);
			
			quantum_slice = 0;

			update_wq(job_list, wq, rq);
			update_hq1(job_list, hq1, rq);
			update_hq2(job_list, hq2, rq);
			update_rq(job_list, rq, cpu);
		}
	}
}

//bankers: Banker's Algorithm
bool bankers(Node* rq, int req_devs){
	int avail_devs_init = avail_devs;
	
	avail_devs_init -= req_devs;
	int avail_devs_fin = avail_devs_init;
	
	Node* temp = rq;
	while(avail_devs_init != avail_devs_fin){
		avail_devs_init = avail_devs_fin;
		
		while(temp->next != NULL){		
			int needed_devs = temp->devices - temp->alloc_devs;
			if(needed_devs < avail_devs_fin && !temp->bankers){
				temp->bankers = true;
				avail_devs_fin += temp->alloc_devs;
			}
			temp = temp->next;
		}
	}
	
	bool ret = true;
	temp = rq;
	while(temp->next != NULL){
		ret = ret && temp->bankers;
		temp->bankers = false;
		
		temp = temp->next;
	}
	
	return ret;
}

//update_job_completion: updates whether a job is completed
void update_job_completion(Node* job_list, Node* job, bool comp) {
	Node *temp = job_list;
	while (temp != NULL) {
		if(temp->job_num == job->job_num) {
			temp->completed = comp;
		}
		temp = temp->next;
	}
}

//update_job_alloc_devs: updates the number of allocate devices
void update_job_alloc_devs(Node* job_list, Node* job, int alloc_devs){
	Node *temp = job_list;
	while (temp != NULL) {
		if(temp->job_num == job->job_num) {
			temp->alloc_devs = alloc_devs;
		}
		temp = temp->next;
	}
}

//update_job_time_completed: updates the time a job completes
void update_job_time_completed(Node* job_list, Node* job, int time_completed){
	Node *temp = job_list;
	while (temp != NULL) {
		if(temp->job_num == job->job_num) {
			temp->time_completed = time_completed;
		}
		temp = temp->next;
	}
}

//update_job_rejection: updates whether a job is rejection
void update_job_rejection(Node* job_list, Node* job, bool rej){
	Node *temp = job_list;
	while (temp != NULL) {
		if(temp->job_num == job->job_num) {
			temp->rejected = rej;
		}
		temp = temp->next;
	}
}