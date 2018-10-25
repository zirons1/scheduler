//Zachary Irons
//Matt Stout

#include <cstdio>
#include <iostream>
#include <stdlib.h>
#include "Node.h"

using namespace std;

//Node constructor
Node::Node(bool head, Node* next){
	this->head = head;
	this->next = next;
	this->completed = false;
	this->bankers = false;
	this->rejected = false;
}

//Adds a Node to the front of a list
void add_front(Node* head, Node* n){
	Node* temp = new Node(false, NULL);
	temp->next = head->next;
	head->next = n;
	n->next = temp->next;
}

//Adds a Node to the end of a list
void add_end(Node* head, Node* n){
	if (head->next == NULL){
		head->next = n;
		n->next = NULL;
	}else{
		Node *temp = head;
		while(temp->next != NULL){
			temp = temp->next;
		}
		temp->next = n;
		n->next = NULL;
	}

}

//Removes a Node to a list
Node* remove(Node* head, int job_num){	
	Node* temp = head;
	Node* prev = NULL;
	
	if(head->job_num == job_num){
		head = head->next;
		return temp;
	}
	while(temp != NULL) {
		if (temp->job_num == job_num) {
			prev->next = temp->next;
			temp->next = NULL;
			return temp;
		}
		prev = temp;
		temp = temp->next;
	}

	return NULL;
}
