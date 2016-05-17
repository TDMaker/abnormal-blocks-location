#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>

#define block_max 1024
#define security_level 512
#define level_a 4
#define level_b 16

char *mapped_mem1 = NULL, *mapped_mem2 = NULL;
long long int flength1 = 0, flength2 = 0;
long* arr_add = NULL;
int final_result[block_max / level_b];
int final_count = 0;
typedef struct node
{
	char data[security_level + 1];
	int number;
	struct node* node1;
	struct node* node2;
	struct node* node3;
	struct node* node4;
}mynode;

int totalNode(int a, int b)
{
	int result = 0;
	while(a / b > 0)
	{
		result += a;
		a /= b;
	}
	return result + 1;
}

void treeMerge(mynode* tree1, mynode* tree2)
{
	if(strcmp(tree1->data, tree2->data) != 0)
	{
		if(tree1->node1 == NULL)
		{
			final_result[final_count++] = tree1->number;
			//printf("%d\n", tree1->number);
		}
		else
		{
			treeMerge(tree1->node1, tree2->node1);
			treeMerge(tree1->node2, tree2->node2);
			treeMerge(tree1->node3, tree2->node3);
			treeMerge(tree1->node4, tree2->node4);
		}
	}
}

void treeVisiting(mynode* tree)
{
	if(tree->node1 == NULL)
	{
		printf("%d\n", tree->number);
		printf("%s\n", tree->data);
	}
	else
	{
		treeVisiting(tree->node1);
		treeVisiting(tree->node2);
		treeVisiting(tree->node3);
		treeVisiting(tree->node4);
		printf("%d\n", tree->number);
		printf("%s\n", tree->data);
	}
	return;
}

node* treeGen(const char* filename)
{
	FILE* sourcefile;
	char tmp_data[security_level];
	if((sourcefile = fopen(filename, "r+")) == NULL)
	{
		return NULL;
	}
	int number_count = 1;
	long* work_cp = arr_add;
	long* sub_work_cp = arr_add;
	node* result_cp = NULL;
	for(int m = block_max / level_b; m > 0; m /= level_a)
	{
		for(int n = 0; n < m; n++)
		{
			memset(tmp_data, 0, sizeof(tmp_data));
			fscanf(sourcefile,"%s",tmp_data);
			mynode* tmp_node = (mynode*)malloc(sizeof(mynode));
			memset(&(tmp_node->data), 0, security_level + 1);
			strcpy(tmp_node->data, tmp_data);
			*(work_cp++) = (long)tmp_node;
			if(m == block_max / level_b)
			{
				tmp_node->number = number_count++;
				tmp_node->node1 = NULL;
				tmp_node->node2 = NULL;
				tmp_node->node3 = NULL;
				tmp_node->node4 = NULL;
			}
			else
			{
				tmp_node->number = -1;
				tmp_node->node1 = (mynode*)(*(sub_work_cp++));
				tmp_node->node2 = (mynode*)(*(sub_work_cp++));
				tmp_node->node3 = (mynode*)(*(sub_work_cp++));
				tmp_node->node4 = (mynode*)(*(sub_work_cp++));
			}
			result_cp = tmp_node;
		}
	}
	//printf("finalcp = %d\n", result_cp);
	return result_cp;
}

void getMappedMem1()
{
	void * start_addr = 0;
	int fd1 = open("database/verifier", O_RDONLY, S_IRUSR | S_IWUSR);
	if (fd1 == -1)
	{
		printf("File database/verifier open failed.\n");
		return;
	}
	flength1 = lseek(fd1, 0, SEEK_END);
	mapped_mem1 = (char *) mmap(start_addr, flength1, PROT_READ, MAP_PRIVATE, fd1, 0);
	if (close(fd1) == -1)
	{
		printf("File database/verifier closed failed.\n");
	}
	return;
}
void getMappedMem2()
{
	void * start_addr = 0;
	int fd2 = open("database/server", O_RDONLY, S_IRUSR | S_IWUSR);
	if (fd2 == -1)
	{
		printf("File database/server open failed.\n");
		return;
	}
	flength2 = lseek(fd2, 0, SEEK_END);
	mapped_mem2 = (char *) mmap(start_addr, flength2, PROT_READ, MAP_PRIVATE, fd2, 0);
	if (close(fd2) == -1)
	{
		printf("File database/server closed failed.\n");
	}
	return;
}

int main(void)
{
	int totalnode = 0;
	struct timeval start1, start2;
	gettimeofday(&start1, NULL);
	totalnode = totalNode(block_max / level_b, level_a);
	arr_add = (long*)malloc(totalnode * sizeof(long));
	mynode* tree1 = treeGen("database/servers_sort");
	mynode* tree2 = treeGen("database/verifire_sort");
	//treeVisiting(tree1);
	//treeVisiting(tree2);
	treeMerge(tree1, tree2);
	getMappedMem1();
	getMappedMem2();
	
	FILE *fs7;
	if ((fs7 = fopen("database/allcompare", "w")) == NULL)
		perror("open");
	int i = 0;
	if(final_result[0] == 0)
	{
		printf("Completely the same\n");
	}
	else
	{
		char tmp_buffer1[security_level + 17];
		char tmp_buffer2[security_level + 17];
		char output_ptr1[64];
		char output_ptr2[64];
		int cell_length = security_level + 16;
		while(final_result[i++] != 0)
		{
			for(int j = 0; j < level_b; j++)
			{
				memset(tmp_buffer1, 0, sizeof(tmp_buffer1));
				memset(tmp_buffer2, 0, sizeof(tmp_buffer2));
				memcpy(tmp_buffer1, mapped_mem1 + cell_length * (final_result[i - 1] - 1) * level_b + j * cell_length, cell_length);
				memcpy(tmp_buffer2, mapped_mem2 + cell_length * (final_result[i - 1] - 1) * level_b + j * cell_length, cell_length);
				if(strcmp(tmp_buffer1, tmp_buffer2) != 0)
				{
					memset(output_ptr1, 0 , sizeof(output_ptr1));
					memset(output_ptr2, 0 , sizeof(output_ptr2));
					memcpy(output_ptr1, tmp_buffer1, strchr(tmp_buffer1, ':') - tmp_buffer1);
					sprintf(output_ptr2, "%d%c", atoi(output_ptr1), ',');
					fwrite(output_ptr2, strlen(output_ptr2), 1, fs7);
				}
			}
		}
	}
	fclose(fs7);
	if (munmap(mapped_mem1, flength1) == -1)
	{
		perror("munmap");
		return(1);
	}
	if (munmap(mapped_mem2, flength2) == -1)
	{
		perror("munmap");
		return(1);
	}
	gettimeofday(&start2, NULL);
	unsigned long long int timeuse = 1000000 * (start2.tv_sec - start1.tv_sec) + start2.tv_usec - start1.tv_usec;
	printf("time: %lld s,%lld ms,%lld us\n", timeuse / (1000 * 1000), timeuse / 1000, timeuse);
	return(0);
}