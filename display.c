#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void ) {
    write(STDOUT_FILENO, "Programme affichage lancé\n", 27);
    char buffer[50];
    scanf("%s", buffer);
    printf("J'affiche : %s\n", buffer);
    return EXIT_SUCCESS;
}