#include <unistd.h>

int main() {
    char buffer[1024];
    getcwd(buffer, sizeof(buffer));
    getcwd(buffer, sizeof(buffer));
    getcwd(buffer, sizeof(buffer));
    getcwd(buffer, sizeof(buffer));
    getcwd(buffer, sizeof(buffer));
    return 0;
}