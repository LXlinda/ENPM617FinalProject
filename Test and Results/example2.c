#include <stdio.h>

int foo(int y)
{
    int p;
    p = y+1;
    return p;
}

void print_int(int x)
{
    printf("%d \n", x);
    return;
}

int main()
{
    int a;
    a = foo(2);
    printf("a = ");
    print_int(a);
    return 0;
}
