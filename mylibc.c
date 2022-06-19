#include <string.h>
#include <stdarg.h>

#include "mylibc.h"
#include "fs.h"

myFILE *myfopen(const char *pathname, const char *mode) {
    myFILE *file = (myFILE *)malloc(sizeof(myFILE));
    if (file == NULL) {
        printf("ERROR: the file is empty");
        exit(0);
    }
    file->id = myopen(pathname,0);
    if (strlen(mode) > 2) {
        printf("ERROR: mode is illegal");
        exit(0);
    }
    strcpy(file->mode, mode);
    if (file->mode[0] == 'w')
       inodes[file->id].size = 0;
    else if (file->mode[0] == 'a')
        mylseek(file->id, 0, SEEK_END);
    return file;
}
int myfclose(myFILE *stream) {
    if (myclose(stream->id) == 0) {
        free(stream);
        return 0;
    }
    free(stream);
    return 1;
}
size_t myfread(void * ptr, size_t size, size_t nmemb, myFILE * stream) {
    if ((stream->mode[0] == 'r' && stream->mode[1] == '+') || stream->mode[0] == 'r') {
        size_t s = my_open_file[stream->id].pos;
        size_t e = myread(stream->id, ptr, nmemb * size);
        return s - e;
    }
    return 0;
}
size_t myfwrite(const void * ptr, size_t size, size_t nmemb, myFILE *stream) {
    if (stream->mode[0] == 'a' || (stream->mode[0] == 'r' && stream->mode[1] == '+') || stream->mode[0] == 'w') {
        size_t s = my_open_file[stream->id].pos;
        size_t e = mywrite(stream->id, ptr, size * nmemb);
        return e - s;
    }
    perror("mode is illegal");
    exit(0);
}
int myfseek(myFILE *stream, long offset, int whence) {
    mylseek(stream->id, offset, whence);
    return 0;
}
int myfscanf(myFILE * stream, const char * format, ...) {
    va_list args;
    va_start(args, format);
    int len = strlen(format);
    int index = 0;
    for (size_t i = 0; i < len; i++) {
        if (format[i] == '%') {
            if (format[i + 1] == 'd') {
                myfread(va_arg(args, void *), sizeof(int), 1, stream);
                index++;
            } else if (format[i + 1] == 'f') {
                myfread((float*)va_arg(args, void *), sizeof(float), 1, stream);
                index++;
            } else if (format[i + 1] == 'c') {
                myfread(va_arg(args, void *), sizeof(char), 1, stream);
                index++;
            }
        }
    }
    return index;
}
int myfprintf(myFILE *stream, const char *format, ...) {
    va_list args;
    va_start(args, format);
    int len = strlen(format);
    int index = 0;
    for (size_t i = 0; i < len; i++) {
        if (format[i] == '%') {
            if (format[i + 1] == 'd') {
                int d = va_arg(args, int);
                myfwrite(&d, sizeof(int), 1, stream);
                index++;
            } else if (format[i + 1] == 'f') {
                float f = va_arg(args, double);
                myfwrite(&f, sizeof(float), 1, stream);
                index++;
            } else if (format[i + 1] == 'c') {
                char c = (va_arg(args, int));
                myfwrite(&c, sizeof(char), 1, stream);
                index++;
            }
            i++;
        } else {
            while (format[i] != '%' && i < len) {
                char ch = format[i];
                myfwrite(&ch, sizeof(char), 1, stream);
                i++;
            }
        }
    }
    return index;
}
