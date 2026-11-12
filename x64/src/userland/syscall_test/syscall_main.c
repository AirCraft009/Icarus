#include <string.h>
#include <stdlib.h>

int main(){
    char buf[12] = "hello world";
    buf[11] = '\0';
    char buf2[12] = "";
    memcpy(buf, buf2, 12);
    return strlen(buf);
}