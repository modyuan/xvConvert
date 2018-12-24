#include <stdio.h>
#include <string.h>
#include <math.h>
#include <malloc.h>


// reference: http://www.cnblogs.com/lsjwq/archive/2011/05/15/2046716.html

int XV_Convert(const char *src_file) {
    int mn;
    const char *ts = NULL;
    const char *video_type[7] = {"rmvb", "wmv", "flv", "avi", "mp4", "mpg", "mkv"};
    const char val[7] = {46, 48, 70, 82, 0, 0, 26};

    char *fb = (char *) malloc(sizeof(char) * 0x200011);

    FILE *fin = fopen(src_file, "rb");
    if (fin == NULL) {
        fprintf(stderr,"can not open file %s\n",src_file);
        return -1;
    }

    //get size of file
    fseek(fin, 0, SEEK_END);
    long l = ftell(fin);

    fseek(fin,0,SEEK_SET);
    fread(fb,1,0x200000,fin);
    char peek[5];
    fread(peek, 1, 4, fin);

    //check the real type of video
    int type=0;
    mn = ('R'-peek[1] + 0x100) & 0xFF;
    if (((peek[2]+mn) & 0xFF) == 'M' && ((peek[3]+mn) & 0xFF ) == 'F') {
        type = 1;
        goto ok;
    }
    mn = (0x26-peek[1] + 0x100) & 0xFF;
    if (((peek[2]+mn) & 0xFF) == 0xB2 && ((peek[3]+mn) & 0xFF) == 0x75) {
        type = 2;
        goto ok;
    }
    mn = ('L'-peek[1] + 0x100) & 0xFF;
    if (((peek[2]+mn) & 0xFF) == 'V') {
        type = 3;
        goto ok;
    }
    mn = ('I'-peek[1] + 0x100) & 0xFF;
    if (((peek[2]+mn) & 0xFF) == 'F' && ((peek[3]+mn) & 0xFF) == 'F') {
        type = 4;
        goto ok;
    }
    mn = (-peek[1] + 0x100) & 0xFF;
    if (((peek[2]+mn) & 0xFF) == 0) {
        type = 5;
        goto ok;
    }
    mn = (-peek[1] + 0x100) & 0xFF;
    if (((peek[2]+mn) & 0xFF) == 1 && ((peek[3]+mn) & 0xFF) == 0xBA) {
        type = 6;
        goto ok;
    }
    mn = (0x45-peek[1] + 0x100) & 0xFF;
    if (((peek[2]+mn) & 0xFF) == 0xDF && ((peek[3]+mn) & 0xFF) == 0xA3) {
        type = 7;
        goto ok;
    }

ok:
    //Unknown video type
    if (type == 0) {
        fclose(fin);
        printf("convert failed: unknown video type.\n");
        free(fb);
        return -1;
    }

    ts = video_type[type - 1];


    // open dest file, remove .xv, append real extension.
    char dst_file[255] = {0};
    strcpy(dst_file, src_file);
    size_t name_length = strlen(dst_file);
    if (name_length > 2
        && dst_file[name_length - 3] == '.'
        && dst_file[name_length - 2] == 'x'
        && dst_file[name_length - 1] == 'v') {
        char *p = &dst_file[name_length - 1];
        *p = '\0'; p--;
        *p = '\0'; p--;
        *p = '\0';
    }
    strcat(dst_file, ".");
    strcat(dst_file, ts);
    FILE *fout = fopen(dst_file, "wb");


    peek[0] = val[type - 1];
    for (int j = 1; j <= 3; j++) {
        peek[j] = (peek[j] + mn) & 0xFF;
    }
    fwrite(peek, 1, 4, fout);
    fread(fb, 1, 0x3FC, fin);
    for (int j = 1; j <= 0x3FC; j++) {
        fb[j - 1] = (fb[j - 1] + mn) & 0xFF;
    }
    fwrite(fb, 1, 0x3FC, fout);

    //write other
    while (ftell(fin) <= l) {

        size_t rl = fread(fb, 1, 0x40000, fin);
        if (rl == 0) break;;
        fwrite(fb, 1, rl, fout);

    }

    fclose(fin);
    fclose(fout);
    free(fb);

    return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("usage: convert xunlei xv video format to other format.\n");
        printf("    %s xv_file1.xv xv_file1.xv ...\n", argv[0]);
        return 0;
    }
    int result;
    printf("converting...\n");
    for (int i = 1; i < argc; i++) {
        if (result = XV_Convert(argv[i])) {
            return result;
        }
    }
    printf("converted %d files.\n",argc-1);
    return 0;
}

