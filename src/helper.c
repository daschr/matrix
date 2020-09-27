#include <stddef.h>

#define ISHEX(X) (('0'<=X && X<='9') || ('a'<=X && X<='f') || ('A'<=X && X<='F'))

int gethval(char e) {
    if('0'<=e && e<='9')
        return e-'0';
    if('a'<=e && e<='f')
        return e-'a'+10;
    if('A'<=e && e<='F')
        return e-'A'+10;
    return 0;
}

char dec_sing(char *e, int *i) {
    *i=1;
	if(e[0] == '+')
		return ' ';

    if(e[0] != '%')
        return e[0];

    if(ISHEX(e[1])) {
        if(ISHEX(e[2])) {
			*i=3;
            return gethval(e[1])*16+gethval(e[2]);
        }
    }

    return '%';
}

int url_decode(char *e, char *buf, size_t bufsize) {
    size_t cpos=0, i=0;
    int s=1;
    while(e[i]!='\0' && cpos<bufsize) {
        buf[cpos++]=dec_sing(e+i, &s);
        i+=s;
    }

    buf[bufsize-1]='\0';
    if(cpos==bufsize) return 0;

    buf[cpos]='\0';
    return 1;
}

