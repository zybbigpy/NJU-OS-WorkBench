#include <assert.h>
#include <ctype.h>
#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_PROC_NUM 1000
#define MAX_FILE_ADDR_LEN 300

/*===================== BUILD PROCESS ======================*/

/*   the process information is in /proc/[pid]/stat.
**   ref: man 5 proc
*/

typedef struct ProcInfo {
  pid_t pid;
  char comm[50];
  char state[4];
  pid_t ppid;
  pid_t pgrp;
} ProcInfo;

ProcInfo sys_porcs[MAX_PROC_NUM];
int num_procs = 0;

/*
** the file address is always /proc/[pid]/stat.
*/
int FillSysProcInfo(const char *file_addr, int *proc_index) {
  FILE *fp = fopen(file_addr, "r");
  if (fp) {
    fscanf(fp, "%d%s%s%d%d", &sys_porcs[*proc_index].pid,
           sys_porcs[*proc_index].comm, sys_porcs[*proc_index].state,
           &sys_porcs[*proc_index].ppid, &sys_porcs[*proc_index].pgrp);
    printf("process info is %d, %s, %s, %d\n", sys_porcs[*proc_index].pid,
           sys_porcs[*proc_index].comm, sys_porcs[*proc_index].state,
           sys_porcs[*proc_index].ppid);

  } else {
    perror("open file fail \n");
    return 1;
  }
  fclose(fp);
  (*proc_index)++;
  return 0;
}

int OpenProcDir(const char *dir_addr) {
  DIR *dir;
  struct dirent *ptr;
  int proc_index = 1;
  dir = opendir(dir_addr);
  if (dir) {
    while ((ptr = readdir(dir)) != NULL) {
      if (isdigit(ptr->d_name[0])) {
        char file_addr[300];
        sprintf(file_addr, "%s%s%s", dir_addr, ptr->d_name, "/stat");
        int ret = FillSysProcInfo(file_addr, &proc_index);
        if (ret) {
          perror("write sys procs array fail. \n");
          return 1;
        } else {
          if (proc_index == MAX_PROC_NUM) {
            perror("proc array is full!\n");
            return 1;
          }
        }
      }
    }
  } else {
    perror("open dir fail\n");
    return 1;
  }
  num_procs = proc_index;
  closedir(dir);
  return 0;
}

/*===================== BUILD PSTREE ======================*/

/*
** use brother-child tree structure to represent the tree is a good idea
**
*/
typedef struct TreeNode {
  ProcInfo *procs;
  struct TreeNode *left_brother;
  struct TreeNode *right_child;
} TreeNode;

TreeNode *FindPrarentNode(TreeNode *root, pid_t ppid) {
  if (root == NULL)
    return NULL;
  if (root->procs->pid == ppid)
    return root;
  TreeNode *node = FindPrarentNode(root->left_brother, ppid);
  if (node != NULL)
    return node;
  node = FindPrarentNode(root->right_child, ppid);
  if (node != NULL) {
    return node;
  } else {
    return NULL;
  }
}

int AddChildNode(TreeNode *root, ProcInfo *instance) {
  TreeNode *child = (TreeNode *)malloc(sizeof(TreeNode));
  child->procs = instance;
  child->left_brother = NULL;
  child->right_child = NULL;
  TreeNode *parent = FindPrarentNode(root, instance->ppid);
  if (parent) {
    if (parent->right_child == NULL) {
      parent->right_child = child;
    } else {
      TreeNode *ptr = parent->right_child;
      while (ptr->left_brother != NULL) {
        ptr = ptr->left_brother;
      }
      ptr->left_brother = child;
    }
    return 0;
  } else {
    return 1;
  }
}

void BuildPstree(TreeNode *root, ProcInfo sys_porcs[]) {
  for (int i = 1; i != num_procs + 1; ++i) {
    int ret = AddChildNode(root, &(sys_porcs[i]));
    if (ret) {
      perror("Add Child Node fail\n");
      exit(EXIT_FAILURE);
    }
  }
}

void PrintPstree(TreeNode *root) {}

/*================= FUNCTION RELATED =====================*/
// int sorted = 0;
void PrintVersion() {
  printf(" Welcome, this is a simple pstree, version 0.1 \n");
}

int main(int argc, char *argv[]) {
  sys_porcs[0].pid = 0;
  OpenProcDir("/proc/");
  TreeNode *root = (TreeNode *)malloc(sizeof(TreeNode));
  root->procs = &(sys_porcs[0]);
  root->left_brother = NULL;
  root->right_child = NULL;

  BuildPstree(root, sys_porcs);
  printf(" the number of process is %d \n", num_procs);
  return 0;
}
