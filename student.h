#ifndef _STUDENT_H_
#define _STUDENT_H_

#define RECORD_FILE_NAME    "student.dat"
#define INDEX_FILE_NAME     "student.idx"
#define MAX_RECORD_SIZE     127 //including 7 delimeters

typedef struct _Student
{
	char id[11];		//학번 1
	char name[21];		//이름 2
	char addr[31];	    //주소 5
	char year[2];		//학년 4
	char dept[20];		//학과 3
    char phone[16];		//전화번호 6
	char email[26];		//이메일 주소 7
} STUDENT;

#endif
