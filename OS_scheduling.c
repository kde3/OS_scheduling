

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


typedef struct PCB
{
	int pid;
	float priority;							//(Since alpha is a floating point number, it is declared as a float type)
	int arrival_time;
	int burst_time;

	struct PCB* next;
} PCB;


typedef struct QUEUE_HEADER					//for Ready Queue
{
	PCB* head;								//It points to the first PCB in the queue.
	PCB* tail;
} QUEUE_HEADER;


//================<  global variable declaration  >================


FILE* OutFile;								//output file

int completion_time;						//Total process end time
int process_end_time[10];					//End time of each process
int response_time[10];						//response time for each process
int cpu_usage_time[10];						//CPU usage time for each process

int waiting_time_for_aging[10];				//Array to store waiting time for aging
float original_priority[10];				//Original priority


//================<  function declaration  >================


void initPCB(char* input_fileName, PCB** arr);											//A function that reads the input file, creates a PCB, and puts it in the job queue
void sortArray(PCB** arr);																//A function that sorts the job queue in order of arrival time.
void swap(PCB** arr, int a, int b);                                                 	//swap function
void move_to_readyQueue(PCB** arr, QUEUE_HEADER* readyQueue, int time, int* front);		//A function to move from job queue to ready queue
void performance_result(PCB** arr);										//performance results function

void FCFS(PCB** arr);													//First Come First Serve (FCFS) 
void RR(PCB** arr, char* a);											//Round Robin (RR)
void Preemptive_Priority(PCB** arr, char* a);							//Preemptive Priority Scheduling with Aging

void arrangement(QUEUE_HEADER* readyQueue);								//A function that puts the PCB with the highest priority in front
void aging(QUEUE_HEADER* readyQueue, float alpha);						//calculate aging function
void Update_wating_time_for_aging(QUEUE_HEADER* readyQueue);			//Update wating time for aging


//================<  function definition  >================


void initPCB(char* input_fileName, PCB** arr)
{
	FILE* inputFile;

	inputFile = fopen(input_fileName, "r");				//Open in reading format.
	if (inputFile == NULL) printf("File Could Not Be Opened\n");



	//Read and store in PCB, and save the address of PCB in arr.
	int i;
	for (i = 0; i < 10; i++)
	{
		PCB* pcb = (PCB*)malloc(sizeof(PCB));			//PCB creation
		arr[i] = pcb;									//store address of PCB

		fscanf(inputFile, "%d %f %d %d", &(pcb->pid), &(pcb->priority), &(pcb->arrival_time), &(pcb->burst_time));		//Read the file and save it to the PCB

		pcb->next = NULL;								//initialization next
		response_time[i] = -1;							//initialization response time(* arr index =  pid - 1)
		cpu_usage_time[i] = pcb->burst_time;			//store cpu_usage_time of each process
		original_priority[i] = pcb->priority;			//store original_priority
	}

	fclose(inputFile);									//close file

	sortArray(arr);										//Sort array by arrival time
}


void sortArray(PCB** arr)								//sorting array function
{
	int min;											//Variable to store the array index of the smallest arrival_time


	int i, j;
	for (i = 0; i < 9; i++)
	{
		min = i;

		for (j = i + 1; j < 10; j++) if (arr[min]->arrival_time > arr[j]->arrival_time) min = j;		//find minimum arrival_time

		swap(arr, i, min);								//swapping
	}
}


void swap(PCB** arr, int a, int b)						//swap function
{
	PCB* temp = arr[a];
	arr[a] = arr[b];
	arr[b] = temp;
}



//function to move(linking) from job queue to ready queue
void move_to_readyQueue(PCB** arr, QUEUE_HEADER* readyQueue, int time, int* front)
{

	while (time == (arr[*front])->arrival_time)				//If the time is the same, put the PCB in the ready queue.
	{

		if (readyQueue->head == NULL)						//If head is null, put the PCB in the ready queue first
		{
			readyQueue->head = arr[*front];
			readyQueue->tail = arr[*front];
		}
		else
		{
			(readyQueue->tail)->next = arr[*front];			//connect at the end
			readyQueue->tail = arr[*front];					//tail change
		}

		printf("<time %d> [new arrival] process %d\n", time, (readyQueue->tail)->pid);			//Prints a message
		fprintf(OutFile, "<time %d> [new arrival] process %d\n", time, (readyQueue->tail)->pid);

		*front = *front + 1;								//Since one is out, the number is +1.
		if (*front == 10) break;
	}
}


//A function that puts the highest priority in the head
void arrangement(QUEUE_HEADER* readyQueue)
{
	PCB* ptr = readyQueue->head;
	PCB* prev_max = NULL;
	PCB* after_max = NULL;
	PCB* max = ptr;													//ptr == head


	if (ptr != NULL)
	{
		while (ptr->next != NULL)
		{
			if (max->priority < (ptr->next)->priority)				//Find the number with the highest priority.
			{
				prev_max = ptr;
				max = prev_max->next;
				after_max = max->next;
			}

			ptr = ptr->next;
		}


		if (max == readyQueue->head);
		else if (after_max == NULL)
		{

			if ((readyQueue->head)->next == max)
			{
				readyQueue->head = max;
				max->next = prev_max;
				prev_max->next = NULL;
				readyQueue->tail = prev_max;
			}
			else
			{
				(readyQueue->tail)->next = (readyQueue->head)->next;
				(readyQueue->head)->next = NULL;
				prev_max->next = readyQueue->head;
				readyQueue->head = readyQueue->tail;
				readyQueue->tail = prev_max->next;
			}

		}
		else
		{
			prev_max->next = readyQueue->head;
			max->next = (readyQueue->head)->next;
			(readyQueue->head)->next = after_max;
			readyQueue->head = max;
		}

	}

}


void aging(QUEUE_HEADER* readyQueue, float alpha)
{
	PCB* ptr = readyQueue->head;

	while (ptr != NULL)
	{
		//update priority
		ptr->priority = original_priority[(ptr->pid) - 1] + (alpha * waiting_time_for_aging[(ptr->pid) - 1]);

		ptr = ptr->next;
	}
}


void Update_wating_time_for_aging(QUEUE_HEADER* readyQueue)
{
	PCB* ptr = readyQueue->head;

	while (ptr != NULL)
	{
		waiting_time_for_aging[(ptr->pid) - 1]++;		//Waiting time is increased one by one only in ReadyQ.

		ptr = ptr->next;
	}
}



//++++++++++++++			Preemptive Priority Scheduling with Aging			++++++++++++++
void Preemptive_Priority(PCB** arr, char* a)
{
	QUEUE_HEADER readyQueue;

	readyQueue.head = NULL;
	readyQueue.tail = NULL;

	PCB* cpu = NULL;

	int time = 0;
	int front = 0;						//Index display of job queue

	float alpha = atof(a);				//alpha for aging


	printf("Scheduling: Preemptive Priority Scheduling with Aging\n");
	printf("==========================================\n");

	fprintf(OutFile, "Scheduling: Preemptive Priority Scheduling with Aging\n");
	fprintf(OutFile, "==========================================\n");


	while (1)
	{
		//if front is 10, job queue is empty
		if (front < 10) move_to_readyQueue(arr, &readyQueue, time, &front);	//enter PCB in ready Q


		//A function that links the PCB with the highest priority to the head
		if (readyQueue.head != NULL) arrangement(&readyQueue);


		if ((cpu != NULL) && (readyQueue.head != NULL))				//cpu isn't NULL
		{
			if ((readyQueue.head)->priority > cpu->priority)			//If ready Q has a higher priority
			{
				printf("------------------------------- (Context-Switch)\n");		//Context-Switch
				fprintf(OutFile, "------------------------------- (Context-Switch)\n");

				waiting_time_for_aging[(cpu->pid) - 1] = 0;			//Waiting time reset
				cpu->priority = original_priority[(cpu->pid) - 1];	//Change to original priority.


				(readyQueue.tail)->next = cpu;						//Sent back to ReadyQ.
				readyQueue.tail = cpu;
				(readyQueue.tail)->next = NULL;

				cpu = NULL;											//cpu is NULL
			}
		}

		//update waiting time for aging
		Update_wating_time_for_aging(&readyQueue);

		//Aging
		aging(&readyQueue, alpha);


		//---- running state(CPU) ----
		if (readyQueue.head == NULL && cpu == NULL)						//if ready Q is empty,
		{
			printf("<time %d> ---- system is idle ----\n", time);		//system is idle
			fprintf(OutFile, "<time %d> ---- system is idle ----\n", time);
		}
		else
		{
			if (cpu == NULL)												//if cpu is NULL,
			{
				cpu = readyQueue.head;									//Allocate CPU from ReadyQ
				readyQueue.head = cpu->next;

				if (response_time[cpu->pid - 1] == -1) response_time[cpu->pid - 1] = time - cpu->arrival_time;		//Find the response time.
			}



			if (cpu->burst_time != 1)													//if not the last run
			{
				printf("<time %d> process %d is running\n", time, cpu->pid);			//running
				fprintf(OutFile, "<time %d> process %d is running\n", time, cpu->pid);

				cpu->burst_time--;														//cpu burst_time - 1
			}
			else																		//If the CPU execution time is 1, it is the last execution.
			{
				printf("<time %d> process %d is finished\n", time, cpu->pid);			//finish
				fprintf(OutFile, "<time % d> process % d is finished\n", time, cpu->pid);

				process_end_time[cpu->pid] = time + 1;									//Enter the ending time (since it ended time + 1).
				free(cpu);
				cpu = NULL;																//free cpu



				if (readyQueue.head != NULL)												//if ready Queue is not empty,
				{
					printf("------------------------------- (Context-Switch)\n");		//context switching
					fprintf(OutFile, "------------------------------- (Context-Switch)\n");
				}
				else																	//head is NULL = last process
				{
					if (front == 10 && cpu == NULL)			//ready queue, job queue and cpu is empty.
					{


						time++;								//*(Since the last process runs to time 35 and all processes are terminated on the 36, it need to add +1.)
						printf("<time %d> all processes finish\n", time);
						printf("=========================\n");

						fprintf(OutFile, "<time %d> all processes finish\n", time);
						fprintf(OutFile, "=========================\n");


						completion_time = time;				//Stores the end time of the entire process.

						performance_result(arr);			//print performance results

						break;								//Terminate execution by escaping the infinite loop with break;.
					}

				}

			}


		}

		time++;				//change time
	}
}



//++++++++++++++++	<RR>	++++++++++++++++++
void RR(PCB** arr, char* a)
{
	QUEUE_HEADER readyQueue;

	readyQueue.head = NULL;
	readyQueue.tail = NULL;

	PCB* cpu = NULL;

	int time = 0;										//time
	int front = 0;

	int time_quantum = atoi(a);							//Convert the received time quantum into a number and insert it.
	int cnt_time_quantum;								//Variables that process only as much as time quantum (count variables)

	printf("Scheduling: RR\n");
	printf("==========================================\n");

	fprintf(OutFile, "Scheduling: RR\n");
	fprintf(OutFile, "==========================================\n");

	while (1)
	{
		if (front < 10) move_to_readyQueue(arr, &readyQueue, time, &front);


		if ((cpu != NULL) && (readyQueue.head != NULL))
		{
			if (cnt_time_quantum == 0)
			{
				printf("------------------------------- (Context-Switch)\n");
				fprintf(OutFile, "------------------------------- (Context-Switch)\n");


				(readyQueue.tail)->next = cpu;
				readyQueue.tail = cpu;
				(readyQueue.tail)->next = NULL;

				cpu = NULL;
			}
		}


		//---- running state(CPU) ----
		if (readyQueue.head == NULL && cpu == NULL)
		{
			printf("<time %d> ---- system is idle ----\n", time);
			fprintf(OutFile, "<time %d> ---- system is idle ----\n", time);
		}
		else
		{
			if (cpu == NULL)
			{
				cpu = readyQueue.head;
				readyQueue.head = cpu->next;

				cnt_time_quantum = time_quantum;									//Time quantum newly assigned.

				if (response_time[cpu->pid - 1] == -1) response_time[cpu->pid - 1] = time - cpu->arrival_time;
			}


			if (cpu->burst_time != 1)
			{
				printf("<time %d> process %d is running\n", time, cpu->pid);
				fprintf(OutFile, "<time %d> process %d is running\n", time, cpu->pid);

				cpu->burst_time--;
				cnt_time_quantum--;
			}
			else
			{
				printf("<time %d> process %d is finished\n", time, cpu->pid);
				fprintf(OutFile, "<time %d> process %d is finished\n", time, cpu->pid);

				process_end_time[cpu->pid] = time + 1;
				free(cpu);
				cpu = NULL;




				if (readyQueue.head != NULL)
				{
					printf("------------------------------- (Context-Switch)\n");		//context switching
					fprintf(OutFile, "------------------------------- (Context-Switch)\n");
				}
				else
				{
					if (front == 10 && cpu == NULL)
					{


						time++;
						printf("<time %d> all processes finish\n", time);
						printf("=========================\n");

						fprintf(OutFile, "<time %d> all processes finish\n", time);
						fprintf(OutFile, "=========================\n");


						completion_time = time;

						performance_result(arr);					//print performance results

						break;
					}

				}

			}

		}

		time++;
	}
}


//++++++++++++++++	<FCFS>	++++++++++++++++++
void FCFS(PCB** arr)
{
	QUEUE_HEADER readyQueue;

	readyQueue.head = NULL;
	readyQueue.tail = NULL;

	PCB* cpu = NULL;

	int time = 0;
	int front = 0;


	printf("Scheduling: FCFS\n");
	printf("==========================================\n");

	fprintf(OutFile, "Scheduling: FCFS\n");
	fprintf(OutFile, "==========================================\n");


	while (1)
	{
		if (front < 10) move_to_readyQueue(arr, &readyQueue, time, &front);


		//--running state(CPU)--
		if (readyQueue.head == NULL)
		{
			printf("<time %d> ---- system is idle ----\n", time);
			fprintf(OutFile, "<time %d> ---- system is idle ----\n", time);
		}
		else
		{
			if (cpu == NULL)
			{
				cpu = readyQueue.head;
				readyQueue.head = cpu->next;

				if (response_time[cpu->pid - 1] == -1) response_time[cpu->pid - 1] = time - cpu->arrival_time;
			}


			if (cpu->burst_time != 1)																									//마지막 실행이 아니라면
			{
				printf("<time %d> process %d is running\n", time, cpu->pid);
				fprintf(OutFile, "<time %d> process %d is running\n", time, cpu->pid);

				cpu->burst_time--;																									//cpu 사용시간을 하나 줄인다.
			}
			else																																		//cpu 실행시간이 1이면 마지막 실행이니
			{
				printf("<time %d> process %d is finished\n", time, cpu->pid);
				fprintf(OutFile, "<time %d> process %d is finished\n", time, cpu->pid);

				process_end_time[cpu->pid] = time + 1;
				free(cpu);
				cpu = NULL;																													//cpu를 비워준다.


				if (readyQueue.head != NULL)
				{
					printf("------------------------------- (Context-Switch)\n");
					fprintf(OutFile, "------------------------------ - (Context - Switch)\n");

				}
				else
				{
					if (front == 10 && cpu == NULL)
					{
						time++;

						printf("<time %d> all processes finish\n", time);
						printf("=========================\n");

						fprintf(OutFile, "<time %d> all processes finish\n", time);
						fprintf(OutFile, "=========================\n");

						completion_time = time;

						performance_result(arr);

						break;
					}

				}

			}

		}

		time++;				//change time
	}
}

//++++++++++++++++++++++++++++++++++



//performance_result function
void performance_result(PCB** arr)
{
	float avg_cpu_usage = 0;
	float avg_waiting_time = 0;
	float avg_response_time = 0;
	float avg_turnaround_time = 0;

	int i;

	//Avarage cpu usage
	for (i = 0; i < 10; i++) avg_cpu_usage += cpu_usage_time[i];
	//printf("각 cpu사용시간의 총합 = [%f]\n", avg_cpu_usage);
	avg_cpu_usage = avg_cpu_usage / completion_time * 100;

	//Avarage waiting time
	for (i = 0; i < 10; i++) avg_waiting_time += (process_end_time[i] - cpu_usage_time[i]);
	avg_waiting_time /= 10;

	//Avarage response time
	for (i = 0; i < 10; i++) avg_response_time += response_time[i];
	avg_response_time /= 10;


	//Avarage turnaround time
	for (i = 0; i < 10; i++) avg_turnaround_time += process_end_time[i];
	avg_turnaround_time /= 10;


	printf("Avarage cpu usage : %.2f %\n", avg_cpu_usage);
	printf("Avarage waiting time : %.1f\n", avg_waiting_time);
	printf("Avarage response time : %.1f\n", avg_response_time);
	printf("Avarage turnaround time : %.1f\n", avg_turnaround_time);

	fprintf(OutFile, "Avarage cpu usage : %.2f %\n", avg_cpu_usage);
	fprintf(OutFile, "Avarage waiting time : %.1f\n", avg_waiting_time);
	fprintf(OutFile, "Avarage response time : %.1f\n", avg_response_time);
	fprintf(OutFile, "Avarage turnaround time : %.1f\n", avg_turnaround_time);
}



//================<  main  >================


int main(int argc, char* argv[])									//argc = 5
{
	OutFile = fopen(argv[2], "w");									//Open as output file writing format
	PCB* jobQueue[10];												//Create job queue (the address of the PCB will be entered)


	initPCB(argv[1], jobQueue);										//argv[1] = input file name


	//int i;
	//for(i = 0; i < 10; i++) printf("%d %f %d %d\n", jobQueue[i]->pid, jobQueue[i]->priority, jobQueue[i]->arrival_time, jobQueue[i]->burst_time);

	int choice;
	printf("Choose one of three algorithms\n\n");

	printf("1) First Come First Serve(FCFS)\n");
	printf("2) Round Robin(RR)\n");
	printf("3) Preemptive Priority Scheduling with Aging\n");

	scanf("%d", &choice);

	switch (choice)
	{
	case 1:

		FCFS(jobQueue);
		break;

	case 2:

		RR(jobQueue, argv[3]);									//time quantum
		break;

	case 3:

		Preemptive_Priority(jobQueue, argv[4]);					//alpha
		break;
	}


	fclose(OutFile);

	return 0;
}
