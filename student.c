#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "student.h"

FILE * idx;

void pack(char *recordbuf, const STUDENT *s); // 구조체를 문자열로

void unpack(const char *recordbuf, STUDENT *s); // 문자열을 구조체로

void readRecord(FILE *fp, char *recordbuf, int rn); 

void add(FILE *fp, const STUDENT *s);

int search(FILE *fp, const char *keyval);

void delete(FILE *fp, const char *keyval);

void printRecord(const STUDENT *s, int n);

int main(int argc, char *argv[])
{
	int i;
	FILE *fp;  // 학생 레코드 파일의 파일 포인터
	STUDENT std;
	short record_num = 0,byteoffset; // record 개수와 byteoffset
	char recordbuf[MAX_RECORD_SIZE];

	memset((char *)recordbuf,0,MAX_RECORD_SIZE);

	if(argc < 3)
	{
		fprintf(stderr,"usage : %s [OPTION] [ARGUMENT1] ...\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	if(access(RECORD_FILE_NAME,F_OK) != 0) 
	{
		if((fp = fopen(RECORD_FILE_NAME,"w")) == NULL) // record file
		{
			fprintf(stderr,"fopen error\n");
			exit(EXIT_FAILURE);
		}

		record_num = -1;
		fwrite(&record_num,sizeof(short),1,fp);
		fflush(fp);
		fclose(fp);
	}
	if(access(INDEX_FILE_NAME,F_OK) != 0)
	{
		if((idx = fopen(INDEX_FILE_NAME,"w")) == NULL) // index file
		{
			fprintf(stderr,"fopen error\n");
			exit(EXIT_FAILURE);
		}

		record_num = 0;
		fwrite(&record_num,sizeof(short),1,idx);
		fflush(idx);
		fclose(idx);
	}

	if((fp = fopen(RECORD_FILE_NAME,"r+")) == NULL) // record file
	{
		fprintf(stderr,"fopen error\n");
		exit(EXIT_FAILURE);
	}
	if((idx = fopen(INDEX_FILE_NAME,"r+")) == NULL) // index file
	{
		fprintf(stderr,"fopen error\n");
		exit(EXIT_FAILURE);
	}


	if(!strcmp(argv[1],"-a")) // add
	{
		if(argc != 9) // id,name,dept,year,addr,phone,email
		{
			fprintf(stderr,"%s -a [ARGUMENT1] [ARGUMENT2] ... [ARGUMENT7]\n", argv[0]);
			exit(EXIT_FAILURE);
		}
		memset(&std,0,sizeof(STUDENT)); 

		strcpy(std.id,argv[2]);
		strcpy(std.name,argv[3]);
		strcpy(std.dept,argv[4]);
		strcpy(std.year,argv[5]);
		strcpy(std.addr,argv[6]);
		strcpy(std.phone,argv[7]);
		strcpy(std.email,argv[8]);
	
		add(fp,&std);
	}
	else if(!strcmp(argv[1],"-d")) // delete - first fit 
	{
		if(argc != 3)
		{
			fprintf(stderr,"%s -d [ARGUMENT]\n", argv[0]);
			exit(EXIT_FAILURE);
		}
		delete(fp,argv[2]);
	}
	else if(!strcmp(argv[1],"-s")) // search
	{
		if(argc != 3)
		{
			fprintf(stderr,"%s -s [ARGUMENT]\n", argv[0]);
			exit(EXIT_FAILURE);
		}
		search(fp,argv[2]);
	}
	else
	{
		fprintf(stderr,"usage : %s [OPTION] [ARGUMENT1] ...\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	fclose(fp);
	fclose(idx);
	return 1;
}

void printRecord(const STUDENT *s, int n) // print a record
{
	int i;

	for(i = 0; i < n; i++)
		printf("%s|%s|%s|%s|%s|%s|%s\n", s[i].id, s[i].name, s[i].dept, s[i].year, s[i].addr, s[i].phone, s[i].email);
}

void pack(char *recordbuf, const STUDENT *s) // 구조체를 문자열로
{
	memset((char *)recordbuf,0,MAX_RECORD_SIZE);

	strcat(recordbuf,s->id);
	strcat(recordbuf,"|");
	strcat(recordbuf,s->name);
	strcat(recordbuf,"|");
	strcat(recordbuf,s->dept);
	strcat(recordbuf,"|");
	strcat(recordbuf,s->year);
	strcat(recordbuf,"|");
	strcat(recordbuf,s->addr);
	strcat(recordbuf,"|");
	strcat(recordbuf,s->phone);
	strcat(recordbuf,"|");
	strcat(recordbuf,s->email);
	strcat(recordbuf,"|");
}

void unpack(const char *recordbuf, STUDENT *s) // 문자열을 구조체로
{
	char * tok;
	char temp[MAX_RECORD_SIZE];

	memset((STUDENT *)s,0,sizeof(STUDENT));
	memset((char *)temp,0,MAX_RECORD_SIZE);
	strcpy(temp,recordbuf);

	tok = strtok(temp,"|");
	strcpy(s->id,tok);
	tok = strtok(NULL,"|");
	strcpy(s->name,tok);
	tok = strtok(NULL,"|");
	strcpy(s->dept,tok);
	tok = strtok(NULL,"|");
	strcpy(s->year,tok);
	tok = strtok(NULL,"|");
	strcpy(s->addr,tok);
	tok = strtok(NULL,"|");
	strcpy(s->phone,tok);
	tok = strtok(NULL,"|");
	strcpy(s->email,tok);
	tok = strtok(NULL,"|");
}

void readRecord(FILE *fp, char *recordbuf, int rn) // rn에 해당하는 레코드를 읽어서 recordbuf에 저장
{
	int i;
	short offset;
	short index = 0;
	short next = 0,max;

	fseek(idx,0,SEEK_SET);
	fread(&index,sizeof(short),1,idx);
	max = index;

	if(max < rn)
	{
		fprintf(stderr,"record number is bigger than real number of records.\n");
		return;
	}
	for(i = 0; i <= rn; i++)
		fread(&index,sizeof(short),1,idx);
	if(max > rn+1)
	{
		fread(&next,sizeof(short),1,idx);
		fseek(fp,index,SEEK_SET);
		fread((char *)recordbuf,sizeof(char),next-index,fp);
	}
	else
	{
		fseek(fp,0,SEEK_END);
		offset = (short)ftell(fp);
		fseek(fp,index,SEEK_SET);
		fread((char *)recordbuf,sizeof(char),offset-index,fp);
	}
}

void add(FILE *fp, const STUDENT *s) // "-a"
{
	short offset,prev,current;
	short header,record_num,len,pos;
	char recordbuf[MAX_RECORD_SIZE];

	fseek(fp,0,SEEK_SET); 
	fread(&header,sizeof(short),1,fp);

	fseek(idx,0,SEEK_SET);
	fread(&record_num,sizeof(short),1,idx); // 레코드의 개수
	fseek(idx,0,SEEK_SET);
	pack(recordbuf,s);

	if(header == -1) // 삭제 레코드가 없음
	{
		fseek(fp,0,SEEK_END);
		offset = (short)ftell(fp); 
		fwrite((char *)recordbuf,sizeof(char),strlen(recordbuf),fp);

		record_num++; // 레코드 개수 증가
		fwrite(&record_num,sizeof(short),1,idx);
		fseek(idx,0,SEEK_END);
		fwrite(&offset,sizeof(short),1,idx);
	}
	else // 삭제 레코드가 있음
	{
		_Bool check = 0;
		current = 0;
		prev = header;
		fseek(fp,header,SEEK_SET);
		fseek(fp,1,SEEK_CUR); // * 마크 넘기기
		fread(&pos,sizeof(short),1,fp); // 삭제 레코드의 offset을 읽기
		fread(&len,sizeof(short),1,fp); // len 따오기
		
		if(pos == -1) // 삭제 레코드가 1개
		{
			if(strlen(recordbuf) <= len) // 써도 됨
			{
				fseek(fp,header,SEEK_SET);
				fwrite((char *)recordbuf,sizeof(char),strlen(recordbuf),fp);
				header = pos;
				fseek(fp,0,SEEK_SET);
				fwrite(&header,sizeof(short),1,fp);
				check = 1;
			}
		}


		while(prev != -1) // 삭제 레코드 검색 loop
		{
			if(strlen(recordbuf) <= len) // 써도 됨
			{
				fseek(fp,prev,SEEK_SET);
				fwrite((char *)recordbuf,sizeof(char),strlen(recordbuf),fp);
				fseek(fp,current,SEEK_SET);
				if(current == 0) // header
					fwrite(&pos,sizeof(short),1,fp);
				else
				{
					char ch = '*';
					fwrite(&ch,sizeof(char),1,fp);
					fwrite(&pos,sizeof(short),1,fp);
				}
				check = 1;
				break;
			}
			else
			{
				current = prev;
				prev = pos;

				fseek(fp,pos,SEEK_SET);
				fseek(fp,1,SEEK_CUR);
				fread(&pos,sizeof(short),1,fp);
				fread(&len,sizeof(short),1,fp);
			}
		}

		if(!check) // 삭제 레코드에 맞는 게 없을 때
		{
			fseek(fp,0,SEEK_END);
			offset = (short)ftell(fp);
			fwrite((char *)recordbuf,sizeof(char),strlen(recordbuf),fp);
	
			record_num++; // 레코드 개수 증가
			fwrite(&record_num,sizeof(short),1,idx);
			fseek(idx,0,SEEK_END);
			fwrite(&offset,sizeof(short),1,idx); // offset 추가
		}
	}
}

int search(FILE *fp, const char *keyval) // "-s"
{
	int ret;
	short i,record_num;
	char recordbuf[MAX_RECORD_SIZE];

	fseek(idx,0,SEEK_SET);
	fread(&record_num,sizeof(short),1,idx);

	for(i = 0; i < record_num; i++)
	{
		STUDENT s;
		memset((char *)recordbuf,0,MAX_RECORD_SIZE);
		readRecord(fp,recordbuf,i);
		if(recordbuf[0] == '*')
			continue;
		unpack(recordbuf,&s);

		if(!strcmp(s.id,keyval))
		{
			printRecord(&s,1);
			return i;
		}
	}
	return -1;
}

void delete(FILE *fp, const char *keyval) // "-d"
{
	int count = 0;
	short i,record_num,header,before,after,len;
	char recordbuf[MAX_RECORD_SIZE];

	fseek(fp,0,SEEK_SET);
	fseek(idx,0,SEEK_SET);
	fread(&header,sizeof(short),1,fp);
	fread(&record_num,sizeof(short),1,idx);

	i = 0;
	while(i < record_num)
	{
		STUDENT s;
		fread(&before,sizeof(short),1,idx);
		fread(&after,sizeof(short),1,idx);
		count++;
		
		if(count == record_num)
		{
			fseek(fp,0,SEEK_END);
			after = (short)ftell(fp);
		}

		len = after - before;
		memset((char *)recordbuf,0,MAX_RECORD_SIZE);
		fseek(fp,before,SEEK_SET);
		fread((char *)recordbuf,sizeof(char),len,fp);
		
		if(recordbuf[0] == '*')
		{
			fseek(idx,-2,SEEK_CUR);
			i++;
			continue;
		}
		unpack(recordbuf,&s);
		
		if(!strcmp(s.id,keyval))
		{
			char ch = '*';
			short temp = header;
			
			fseek(fp,before,SEEK_SET);
			fwrite(&ch,sizeof(char),1,fp);
			fwrite(&temp,sizeof(short),1,fp);
			fwrite(&len,sizeof(short),1,fp);
			fseek(fp,0,SEEK_SET);
			fwrite(&before,sizeof(short),1,fp);
			break;
		}
		else 
			fseek(idx,-2,SEEK_CUR);
		i++;
	}
}
