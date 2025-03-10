#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void child1_process(int read_fd, int write_fd) {
    int num;
    while (read(read_fd, &num, sizeof(int)) > 0) {
        num *= num; // Square the number
        write(write_fd, &num, sizeof(int));
    }
    close(read_fd);
    close(write_fd);
    exit(0);
}

void child2_process(int read_fd, int write_fd) {
    int num;
    while (read(read_fd, &num, sizeof(int)) > 0) {
        num += 1; // Add 1
        write(write_fd, &num, sizeof(int));
    }
    close(read_fd);
    close(write_fd);
    exit(0);
}

int main() {
    int pipe1[2], pipe2[2], pipe3[2];
    pid_t child1, child2;
    int num, result;

    // Create pipes
    if (pipe(pipe1) == -1 || pipe(pipe2) == -1 || pipe(pipe3) == -1) {
        perror("pipe");
        exit(1);
    }

    // Fork first child
    if ((child1 = fork()) == -1) {
        perror("fork");
        exit(1);
    }

    if (child1 == 0) {
        // Child 1 process: receives from Parent (pipe1), sends to Child2 (pipe2)
        close(pipe1[1]); // Close unused write end of pipe1
        close(pipe2[0]); // Close unused read end of pipe2
        child1_process(pipe1[0], pipe2[1]);
    }

    // Fork second child
    if ((child2 = fork()) == -1) {
        perror("fork");
        exit(1);
    }

    if (child2 == 0) {
        // Child 2 process: receives from Child1 (pipe2), sends to Parent (pipe3)
        close(pipe2[1]); // Close unused write end of pipe2
        close(pipe3[0]); // Close unused read end of pipe3
        child2_process(pipe2[0], pipe3[1]);
    }

    // Parent process
    close(pipe1[0]); // Close unused read end of pipe1
    close(pipe2[0]); // Parent doesn't use pipe2 directly
    close(pipe2[1]); // Parent doesn't use pipe2 directly
    close(pipe3[1]); // Close unused write end of pipe3

    printf("Enter integers (Ctrl+D to exit):\n");
    while (scanf("%d", &num) == 1) {
        write(pipe1[1], &num, sizeof(int)); // Send to Child1
        read(pipe3[0], &result, sizeof(int)); // Receive from Child2
        printf("Final result: %d\n", result);
    }

    // Close pipes in the parent
    close(pipe1[1]);
    close(pipe3[0]);

    // Wait for children to exit
    waitpid(child1, NULL, 0);
    waitpid(child2, NULL, 0);

    printf("Parent exiting.\n");
    return 0;
}
