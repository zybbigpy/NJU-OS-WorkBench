#include <assert.h>
#include <ctype.h>
#include <dirent.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_PROC_NUM 1024
#define MAX_FILE_ADDR_LEN 512
#define MAX_TREE_DEPTH 20

/*================= GLOBAL RELATED =====================*/

struct {
  int numeric_sort;
  int show_pid;
} global_setting;

int opt;
static const char *optstring = "pnv";
static const struct option long_options[] = {
    {"show_pids", no_argument, NULL, 'p'},
    {"numeric_sort", no_argument, NULL, 'n'},
    {"version", no_argument, NULL, 'v'},
    {NULL, no_argument, NULL, 0}};

/*===================== BUILD PROCESS ======================*/

typedef struct ProcInfo {
  pid_t pid;
  char comm[50];
  char state;
  pid_t ppid;
  pid_t pgrp;
  int level;  // the child level in the pstree
} ProcInfo;   // ref: man proc

ProcInfo sys_porcs[MAX_PROC_NUM];
int num_procs = 0;

int FillSysProcInfo(
    const char *file_addr,
    int *proc_index) {  // the file address is always /proc/[pid]/stat
  FILE *fp = fopen(file_addr, "r");
  if (fp) {
    // fscanf(fp, "%d%s%c%d%d", &sys_porcs[*proc_index].pid,
    //        sys_porcs[*proc_index].comm, &sys_porcs[*proc_index].state,
    //        &sys_porcs[*proc_index].ppid, &sys_porcs[*proc_index].pgrp);
    char buf[MAX_FILE_ADDR_LEN];
    fgets(buf, MAX_FILE_ADDR_LEN, fp);
    //printf("the buf is %s", buf);

    char *lh = buf;
    char *rh = buf;
    while (*lh) {
      if (*lh == '(') break;
      ++lh;
    }
    while (*rh) {
      if (*rh == ')') break;
      ++rh;
    }

    int pid, ppid;
    char state;
    char comm[32];
    
    strncpy(comm, lh, rh-lh);
    sscanf(buf, "%d", &pid);
    sscanf(rh, "%c %d",&state, &ppid);
    printf("Read info [%d %s %c %d]\n", pid, comm, state, ppid);
  } else {
    perror("open file fail \n");
    exit(EXIT_FAILURE);
  }
  fclose(fp);
  (*proc_index)++;
  return 0;
}

pid_t FindPpid(pid_t pid) {
  pid_t ret = 0;
  for (int i = 1; i <= num_procs; ++i) {
    if (sys_porcs[i].pid == pid) ret = sys_porcs[i].ppid;
  }
  return ret;
}

void FillProcsLevel(ProcInfo sys_porcs[]) {
  for (int i = 1; i <= num_procs; ++i) {
    sys_porcs[i].level = 1;
    pid_t ppid = FindPpid(sys_porcs[i].pid);
    while (ppid != 0) {
      ppid = FindPpid(ppid);
      sys_porcs[i].level++;
    }
  }
}

int OpenProcDir(const char *dir_addr) {
  DIR *dir;
  struct dirent *ptr;
  int proc_index = 1;
  dir = opendir(dir_addr);
  if (dir) {
    while ((ptr = readdir(dir)) != NULL) {
      if (isdigit(ptr->d_name[0])) {
        char file_addr[MAX_FILE_ADDR_LEN];
        sprintf(file_addr, "%s%s%s", dir_addr, ptr->d_name, "/stat");
        int ret = FillSysProcInfo(file_addr, &proc_index);
        if (ret) {
          perror("write sys procs array fail. \n");
          exit(EXIT_FAILURE);
        } else {
          if (proc_index == MAX_PROC_NUM) {
            perror("proc array is full!\n");
            exit(EXIT_FAILURE);
          }
        }
      }
    }
  } else {
    perror("open dir fail\n");
    exit(EXIT_FAILURE);
  }
  num_procs = proc_index;
  closedir(dir);
  FillProcsLevel(sys_porcs);
  return 0;
}

/*===================== BUILD PSTREE ======================*/

typedef struct TreeNode {
  ProcInfo *procs;
  struct TreeNode *left_brother;
  struct TreeNode *right_child;
} TreeNode;  // use right-child left-sibling binary tree

TreeNode *FindPrarentNode(TreeNode *root, pid_t ppid) {
  if (root == NULL) return NULL;
  if (root->procs->pid == ppid) return root;
  TreeNode *node = FindPrarentNode(root->left_brother, ppid);
  if (node != NULL) return node;
  node = FindPrarentNode(root->right_child, ppid);
  if (node != NULL) {
    return node;
  } else {
    return NULL;
  }
}

void AddChildNodePid(TreeNode *parent, TreeNode *child) {
  if (parent->right_child == NULL) {
    parent->right_child = child;
  } else {
    TreeNode *ptr = parent->right_child;
    while (ptr->left_brother != NULL) {
      ptr = ptr->left_brother;
    }
    ptr->left_brother = child;
  }
}

void AddChildNodeAlphbat(TreeNode *parent, TreeNode *child) {
  if (parent->right_child == NULL) {
    parent->right_child = child;
  } else {
    TreeNode *ptr = parent->right_child;
    if (strcmp(child->procs->comm, ptr->procs->comm) < 0) {
      parent->right_child = child;
      child->left_brother = ptr;
    } else {
      while (ptr != NULL) {
        if (ptr->left_brother == NULL) {
          ptr->left_brother = child;
          break;
        }
        if (strcmp(ptr->left_brother->procs->comm, child->procs->comm) < 0) {
          ptr = ptr->left_brother;
        } else {
          TreeNode *tmp = ptr->left_brother;
          ptr->left_brother = child;
          child->left_brother = tmp;
          break;
        }
      }
    }
  }
}

int AddChildNode(TreeNode *root, ProcInfo *instance) {
  TreeNode *child = (TreeNode *)malloc(sizeof(TreeNode));
  child->procs = instance;
  child->left_brother = NULL;
  child->right_child = NULL;

  TreeNode *parent = FindPrarentNode(root, instance->ppid);
  if (parent) {
    if (global_setting.numeric_sort) {  // sort by pid
      AddChildNodePid(parent, child);
    } else {  // sort by alphabet
      AddChildNodeAlphbat(parent, child);
    }
    return 0;
  } else {
    printf("the proc info is [pid %d] [comm %s] [ppid %d] \n  ", instance->pid,
           instance->comm, instance->ppid);
    return 1;
  }
}

void BuildPstree(TreeNode *root, ProcInfo sys_porcs[]) {
  for (int i = 1; i != num_procs; ++i) {
    int ret = AddChildNode(root, &(sys_porcs[i]));
    if (ret) {
      perror("Add Child Node fail,cannot find parent node\n");
      exit(EXIT_FAILURE);
    }
  }
}

int rec[MAX_TREE_DEPTH] = {0};

void PrintPstree(TreeNode *root) {
  if (root == NULL) {
    return;
  } else {
    if (root->left_brother) {
      rec[root->procs->level] = 1;
    } else {
      rec[root->procs->level] = 0;
    }
    for (size_t i = 0; i < root->procs->level; i++) {
      if (rec[i] == 0) {
        printf("       ");
      } else {
        printf("|      ");
      }
    }

    if (global_setting.show_pid) {
      printf("+------%d %s\n", root->procs->pid, root->procs->comm);
    } else {
      printf("+------%s\n", root->procs->comm);
    }

    PrintPstree(root->right_child);
    PrintPstree(root->left_brother);
  }
}

void DestroyPstree(TreeNode *root) {  // free the memory of pstree
  if (root == NULL) return;
  DestroyPstree(root->left_brother);
  DestroyPstree(root->right_child);
  free(root);
}

/*================= SHOW FUNCIONS =====================*/

void ShowVersion() {
  printf(
      "BRIEF-Pstree, Version-0.1, copyright@Wangqian Miao, 151242031--NJU \n");
  exit(EXIT_SUCCESS);
}

void ShowUse() {
  printf("How to use? [-p, --show-pids] [-n,--numeric-sort] [-v, --version]\n");
  exit(EXIT_FAILURE);
}

/*================= MAIN   FUNC    =====================*/

int main(int argc, char *argv[]) {
  sys_porcs[0].pid = 0;
  strcpy(sys_porcs[0].comm, "**THE PSTREE ROOT**");
  OpenProcDir("/proc/");

  TreeNode *root = (TreeNode *)malloc(sizeof(TreeNode));
  root->procs = &(sys_porcs[0]);
  root->left_brother = NULL;
  root->right_child = NULL;

  while (1) {
    opt = getopt_long(argc, argv, optstring, long_options, NULL);
    if (opt == -1) break;
    switch (opt) {
      case 'p':
        global_setting.show_pid = 1;
        break;
      case 'n':
        global_setting.numeric_sort = 1;
        break;
      case 'v':
        ShowVersion();
        break;
      default:
        ShowUse();
    }
  }

  BuildPstree(root, sys_porcs);
  // PrintPstree(root);
  DestroyPstree(root);
  exit(EXIT_SUCCESS);
}
