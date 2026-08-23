#include <unistd.h>
#include <sys/syscall.h>

int main (int argc, char *argv[])
{
    syscall(900);
    return 0;
}
