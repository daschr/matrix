#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#define MAX_LINEWIDTH 11


int8_t bin2c(char *filename);
int main(int ac, char *as[]){
	if(ac!= 2){
		fprintf(stderr, "Usage: %s [infile]\n", as[0]);
		return 0;
	}
	
	if(!bin2c(as[1]))
		return 1;

	return 0;
}

void normalize_str(const char *str, char *outbuf){
	for(size_t i=0;str[i]!='\0';++i){
		if(	(str[i]>='0' && str[i]<='9') ||
			(str[i]>='a' && str[i]<='z') ||
			(str[i]>='A' && str[i]<='Z'))

			outbuf[i]=str[i];
		else
			outbuf[i]='_';
	}
	
}

int8_t bin2c(char *filename){
	FILE *file=fopen(filename, "r");
	if(file==NULL)
		return 0;
	
	char norm_name[strlen(filename)+1];
	memset(norm_name, 0, strlen(filename)+1);
	normalize_str(filename, norm_name);

	printf("unsigned char %s[]={\n", norm_name);

	unsigned int num_bytes=0, linewidth=0;
	int c=0;
	while((c=fgetc(file))!= '\0' && c != -1){
		printf(" 0x%02x,", c);
		if(linewidth++==MAX_LINEWIDTH){
			putc('\n', stdout);
			linewidth=0;		
		}
		num_bytes++;
	}
	++num_bytes;

	fclose(file);
	printf(" 0x00\n};\n");
	printf("unsigned int %s_len=%u;\n", norm_name, num_bytes);
	
	return 1;
}
