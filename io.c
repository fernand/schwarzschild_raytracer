void getFileContents(const char* path, char* contents, int* len) {
    FILE* file = fopen(path, "r");
    fseek(file, 0, SEEK_END);
    int fsize = ftell(file);
    if(fsize > *len) {
        printf("getFileContents was given a buffer which is too small for the file size\n");
        printf("Necessary file size is %d\n", fsize);
        exit(-1);
    }
    *len = fsize;
    fseek(file, 0, SEEK_SET);
    fread(contents, fsize, 1, file);
    fclose(file);
};
