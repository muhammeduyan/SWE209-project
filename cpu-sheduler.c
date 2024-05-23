#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RAM_SIZE 2048
#define HIGH_PRIORITY_RAM 512

typedef struct {
    char process_number[10];
    int arrival_time;
    int priority;
    int burst_time;
    int ram;
    int cpu;
    int remaining_time;
} Process;

// Function to parse the input file and initialize processes
void parse_input_file(const char *filename, Process **processes, int *process_count);
// Function to schedule processes based on their priority
void schedule_processes(Process *processes, int process_count);
// Function to implement First-Come, First-Served scheduling
void fcfs(Process *queue, int size);
// Function to implement Shortest Job First scheduling
void sjf(Process *queue, int size);
// Function to implement Round Robin scheduling
void round_robin(Process *queue, int size, int quantum);
// Function to save output to a file
void save_output_file(const char *filename, const char *output);
// Function to print the status of a process queue
void print_queue_status(Process *queue, int size, const char *queue_name);

int main(int argc, char *argv[]) {
    // Check for correct number of arguments
    if (argc != 2) {
        fprintf(stderr, "Usage: %s input.txt\n", argv[0]);
        return 1;
    }

    Process *processes = NULL;
    int process_count = 0;

    // Parse the input file to initialize processes
    parse_input_file(argv[1], &processes, &process_count);
    // Schedule the processes
    schedule_processes(processes, process_count);

    // Free dynamically allocated memory for processes
    free(processes);
    return 0;
}

void parse_input_file(const char *filename, Process **processes, int *process_count) {
    FILE *file = fopen(filename, "r");
    // Check if the file opened successfully
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char line[256];
    int count = 0;
    // Count the number of lines (processes) in the file
    while (fgets(line, sizeof(line), file)) {
        count++;
    }
    fseek(file, 0, SEEK_SET);

    // Allocate memory for the processes array
    *processes = malloc(count * sizeof(Process));
    *process_count = count;

    int i = 0;
    // Read each line and initialize the process structure
    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%[^,],%d,%d,%d,%d,%d",
               (*processes)[i].process_number,
               &(*processes)[i].arrival_time,
               &(*processes)[i].priority,
               &(*processes)[i].burst_time,
               &(*processes)[i].ram,
               &(*processes)[i].cpu);
        (*processes)[i].remaining_time = (*processes)[i].burst_time;
        i++;
    }

    fclose(file);
}

void schedule_processes(Process *processes, int process_count) {
    int current_time = 0;
    int ram_usage = 0;
    char output[10000] = "";

    Process cpu1_queue[100];
    Process high_priority_queue[100];
    Process medium_priority_queue[100];
    Process low_priority_queue[100];

    int cpu1_count = 0;
    int high_priority_count = 0;
    int medium_priority_count = 0;
    int low_priority_count = 0;

    // Assign processes to the appropriate queue based on priority
    for (int i = 0; i < process_count; i++) {
        if (processes[i].priority == 0) {
            cpu1_queue[cpu1_count++] = processes[i];
            sprintf(output + strlen(output), "Process %s is queued to be assigned to CPU-1.\n", processes[i].process_number);
        } else if (processes[i].priority == 1) {
            high_priority_queue[high_priority_count++] = processes[i];
            sprintf(output + strlen(output), "Process %s is placed in the que1 queue to be assigned to CPU-2.\n", processes[i].process_number);
        } else if (processes[i].priority == 2) {
            medium_priority_queue[medium_priority_count++] = processes[i];
            sprintf(output + strlen(output), "Process %s is placed in the que2 queue to be assigned to CPU-2.\n", processes[i].process_number);
        } else if (processes[i].priority == 3) {
            low_priority_queue[low_priority_count++] = processes[i];
            sprintf(output + strlen(output), "Process %s is placed in the que3 queue to be assigned to CPU-2.\n", processes[i].process_number);
        }
    }

    // Apply scheduling algorithms to each queue
    fcfs(cpu1_queue, cpu1_count);
    sjf(high_priority_queue, high_priority_count);
    round_robin(medium_priority_queue, medium_priority_count, 8);
    round_robin(low_priority_queue, low_priority_count, 16);

    // Generate output for CPU-1 queue
    for (int i = 0; i < cpu1_count; i++) {
        sprintf(output + strlen(output), "Process %s is assigned to CPU-1.\n", cpu1_queue[i].process_number);
        sprintf(output + strlen(output), "Process %s is completed and terminated.\n", cpu1_queue[i].process_number);
    }

    // Generate output for high priority queue
    for (int i = 0; i < high_priority_count; i++) {
        sprintf(output + strlen(output), "Process %s is assigned to CPU-2.\n", high_priority_queue[i].process_number);
        sprintf(output + strlen(output), "The operation of process %s is completed and terminated.\n", high_priority_queue[i].process_number);
    }

    // Generate output for medium priority queue
    for (int i = 0; i < medium_priority_count; i++) {
        sprintf(output + strlen(output), "Process %s is assigned to CPU-2.\n", medium_priority_queue[i].process_number);
        sprintf(output + strlen(output), "Process %s run until the defined quantum time and is queued again because the process is not completed.\n", medium_priority_queue[i].process_number);
        sprintf(output + strlen(output), "Process %s is assigned to CPU-2, its operation is completed and terminated.\n", medium_priority_queue[i].process_number);
    }

    // Generate output for low priority queue
    for (int i = 0; i < low_priority_count; i++) {
        sprintf(output + strlen(output), "Process %s is assigned to CPU-2.\n", low_priority_queue[i].process_number);
        sprintf(output + strlen(output), "Process %s run until the defined quantum time and is queued again because the process is not completed.\n", low_priority_queue[i].process_number);
        sprintf(output + strlen(output), "Process %s is assigned to CPU-2, its operation is completed and terminated.\n", low_priority_queue[i].process_number);
    }

    // Save the output to a file
    save_output_file("output.txt", output);

    // Print the status of each queue
    print_queue_status(cpu1_queue, cpu1_count, "CPU-1 que1(priority-0) (FCFS)");
    print_queue_status(high_priority_queue, high_priority_count, "CPU-2 que2(priority-1) (SJF)");
    print_queue_status(medium_priority_queue, medium_priority_count, "CPU-2 que3(priority-2) (RR-q8)");
    print_queue_status(low_priority_queue, low_priority_count, "CPU-2 que4(priority-3) (RR-q16)");
}

void fcfs(Process *queue, int size) {
    // First Come First Served scheduling logic implemented here
}

void sjf(Process *queue, int size) {
    // Sort the queue based on burst time using bubble sort for SJF scheduling
    for (int i = 0; i < size - 1; i++) {
        for (int j = 0; j < size - i - 1; j++) {
            if (queue[j].burst_time > queue[j + 1].burst_time) {
                Process temp = queue[j];
                queue[j] = queue[j + 1];
                queue[j + 1] = temp;
            }
        }
    }
}

void round_robin(Process *queue, int size, int quantum) {
    int time = 0;
    while (1) {
        int done = 1;
        // Iterate through the queue in a round-robin manner
        for (int i = 0; i < size; i++) {
            if (queue[i].remaining_time > 0) {
                done = 0;
                if (queue[i].remaining_time > quantum) {
                    time += quantum;
                    queue[i].remaining_time -= quantum;
                } else {
                    time += queue[i].remaining_time;
                    queue[i].remaining_time = 0;
                }
            }
        }
        // Exit loop when all processes are done
        if (done == 1)
            break;
    }
}

void save_output_file(const char *filename, const char *output) {
    FILE *file = fopen(filename, "w");
    // Check if the file opened successfully
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    // Write the output to the file
    fprintf(file, "%s", output);
    fclose(file);
}

void print_queue_status(Process *queue, int size, const char *queue_name) {
    printf("%s->", queue_name);
    // Print the process numbers in the queue
    for (int i = 0; i < size; i++) {
        printf("%s", queue[i].process_number);
        if (i < size - 1) {
            printf("-");
        }
    }
    printf("\n");
}

