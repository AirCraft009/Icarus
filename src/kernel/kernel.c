void kmain(void) {
    const char *str = "!!ICARUS!!";
    char *videoptr = (char*) 0xb80000;


    while (*str != '\0') {
        *videoptr = *str;
        videoptr++;
        str++;
    }

    while (1){}
}

