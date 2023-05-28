// A simple file system using FUSE
// store data in memory
#define FUSE_USE_VERSION 31
#include "socket_client.h"
#include "socket_server.h"
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/xattr.h>
#include <unistd.h>
#define MAX_FILE_NUM 100
#define MAX_FILE_NAME 100
#define MAX_FILE_SIZE 1000
#define MAX_FILE_CONTENT 10000
#define CHAT_CONFIG_FILE "/.chat_config"
#define CHAT_CONTENT_FILE "/.chat_content"
#define CHAT_CONFIG_FILE_INDEX 1
#define CLIENT_MODE 0
#define SERVER_MODE 1
#define DEBUG_MODE
#ifdef DEBUG_MODE
#define debug_print(fmt, ...)                                                  \
  do {                                                                         \
    fprintf(stderr, fmt, __VA_ARGS__);                                         \
  } while (0)
#else
#define debug_print(fmt, ...)                                                  \
  do {                                                                         \
  } while (0)
#endif

// file struct to store file information in memory
struct file {
  char name[MAX_FILE_NAME];
  char content[MAX_FILE_CONTENT];
  int size;
  int isDir;
  int isDeleted;
  mode_t mode;
  int uid, gid;
};

// file system struct to store file system information in memory
struct file_system {
  struct file files[MAX_FILE_NUM];
  int file_num;
  int chat_support_flag; // 0: not support, 1: support
  int chat_mode;         // 0: client, 1: server
  int chat_file_index;
  char chat_buffer[BUFFER_SIZE];
};

// global variable to store file system information

struct file_system fs;

// print debug information in fs.files

void print_fs() {
  debug_print("file_num %d\n", fs.file_num);
  for (int i = 0; i < MAX_FILE_NUM; i++) {
    if (fs.files[i].isDeleted == 0) {
      debug_print("file %d, name %s, size %d, isDir %d, isDeleted %d\n", i,
                  fs.files[i].name, fs.files[i].size, fs.files[i].isDir,
                  fs.files[i].isDeleted);
    }
  }
}

// implement a function to distinguish file and directory

int is_dir(const char *path) {
  // all directory end with '/'
  if (path[strlen(path) - 1] == '/') {
    return 1;
  }
  return -1;
}

// implement a function to judge whether a file is subjected to a directory

int is_subdir(const char *name, const char *path) {
  // name: file name "path1/name"
  // path: directory name
  // if path1==path, return 1
  // if name==path return 0
  debug_print("[is_subdir]name %s, path %s\n", name, path);
  if (strcmp(name, path) == 0) {
    return 0;
  }
  // copy name and path
  char *name_copy = (char *)malloc(sizeof(char) * strlen(name));
  char *path_copy = (char *)malloc(sizeof(char) * strlen(path));
  strcpy(name_copy, name);
  strcpy(path_copy, path);
  // if name ends with '/', remove it
  if (name_copy[strlen(name_copy) - 1] == '/') {
    name_copy[strlen(name_copy) - 1] = '\0';
    debug_print("remove / from name, name %s\n", name);
  }
  // if path does not end with '/', add it
  if (path_copy[strlen(path_copy) - 1] != '/') {
    strcat(path_copy, "/");
    debug_print("add / to path, path %s\n", path);
  }
  char *p = strrchr(name_copy, '/');
  if (p == NULL) {
    return 0;
  }
  ++p;
  char *dir_name = (char *)malloc(sizeof(char) * (p - name_copy));
  strncpy(dir_name, name_copy, p - name_copy);
  dir_name[p - name_copy + 1] = '\0';
  if (strcmp(dir_name, path_copy) == 0) {
    debug_print("yes %d\n", 1);
    return 1;
  }
  debug_print("no %d\n", 0);
  return 0;
}

// implement a function to find the parent directory of a file or directory

char *find_parent_dir(const char *path) {
  char *p = strrchr(path, '/');
  if (p == NULL) {
    return NULL;
  }
  char *dir_name = (char *)malloc(sizeof(char) * (p - path));
  strncpy(dir_name, path, p - path);
  dir_name[p - path] = '\0';
  return dir_name;
}

int compare_file_name(const char *name, const char *path) {
  int len_a = strlen(name);
  int len_b = strlen(path);
  if (len_b >= len_a && strncmp(name, path, len_b - 1) == 0) {
    return 1;
  }
  return 0;
}

// find file by name

int find_file(const char *name) {
  for (int i = 0; i < MAX_FILE_NUM; i++) {
    if (fs.files[i].isDeleted == 0) {
      // if fs.files[i].name == name or fs.files[i].name[:-1] == name return i
      int len = strlen(fs.files[i].name);
      debug_print("name %s, fs.files[i].name %s, len %d\n", name,
                  fs.files[i].name, len);
      if (compare_file_name(name, fs.files[i].name)) {
        return i;
      }
    }
  }
  return -1;
}

// find empty file

int find_empty_file() {
  for (int i = 0; i < MAX_FILE_NUM; i++) {
    if (fs.files[i].isDeleted == 1) {
      return i;
    }
  }
  return -1;
}

// get file size

int get_file_size(char *name) {
  int index = find_file(name);
  if (index == -1) {
    return -1;
  }
  return fs.files[index].size;
}

// get file content

char *get_file_content(char *name) {
  int index = find_file(name);
  if (index == -1) {
    return NULL;
  }
  return fs.files[index].content;
}

// add file

int add_file(const char *name, char *content, int size, mode_t mode) {
  int index = find_empty_file();
  if (index == -1) {
    return -1;
  }
  char *name_copy = (char *)malloc(sizeof(char) * strlen(name));
  strcpy(name_copy, name);
  // if name ends with '/', remove it
  if (name_copy[strlen(name_copy) - 1] == '/') {
    name_copy[strlen(name_copy) - 1] = '\0';
    debug_print("remove \\ , name %s\n", name_copy);
  }
  strcpy(fs.files[index].name, name_copy);
  strcpy(fs.files[index].content, content);
  fs.files[index].size = size;
  fs.files[index].isDeleted = 0;
  fs.files[index].isDir = 0;
  fs.file_num++;
  struct fuse_context *context = fuse_get_context();
  if (context != NULL) {
    fs.files[index].uid = context->uid;
    fs.files[index].gid = context->gid;
  } else {
    perror("get context error");
    exit(1);
  }
  fs.files[index].mode = mode;
  debug_print("add file %s, size %d, content %s\n", name_copy, size, content);
  if (strcmp(name_copy, CHAT_CONTENT_FILE) == 0) {
    // if the file is chat content file, set the index
    fs.chat_file_index = index;
  }
  return 0;
}

// add directory

int add_dir(const char *name, mode_t mode) {
  int index = find_empty_file();
  if (index == -1) {
    return -1;
  }
  // add a directory end with '/'
  strcpy(fs.files[index].name, name);
  if (fs.files[index].name[strlen(fs.files[index].name) - 1] != '/') {
    strcat(fs.files[index].name, "/");
  }
  fs.files[index].isDir = 1;
  fs.files[index].isDeleted = 0;
  fs.file_num++;
  struct fuse_context *context = fuse_get_context();
  if (context != NULL) {
    fs.files[index].uid = context->uid;
    fs.files[index].gid = context->gid;
  } else {
    perror("get context error");
    exit(1);
  }
  fs.files[index].mode = mode;
  debug_print("add dir %s\n", name);
  return 0;
}

// delete file

int delete_file(const char *name) {
  int index = find_file(name);
  if (index == -1) {
    return -1;
  }
  if (fs.files[index].isDir == 1) {
    // return an error if the file is a directory
    return -1;
  }
  fs.files[index].isDeleted = 1;
  fs.file_num--;
  return 0;
}

// delete directory

int delete_dir(const char *name) {
  int index = find_file(name);
  if (index == -1) {
    return -1;
  }
  if (fs.files[index].isDir == 0) {
    // return an error if the file is not a directory
    return -1;
  }
  fs.files[index].isDeleted = 1;
  fs.file_num--;
  // delete every file and directory in the directory
  for (int i = 0; i < MAX_FILE_NUM; i++) {
    if (fs.files[i].isDeleted == 0 &&
        is_subdir(fs.files[i].name, fs.files[index].name) == 1) {
      if (fs.files[i].isDir == 1) {
        delete_dir(fs.files[i].name);
      } else {
        delete_file(fs.files[i].name);
      }
    }
  }
  return 0;
}

// implement a function to check if there is new message in the socket

int update_content() {
  int index = fs.chat_file_index, res = 0;
  //clear the buffer
  memset(fs.chat_buffer, 0, sizeof(fs.chat_buffer));
  if (fs.chat_mode == CLIENT_MODE) {
    // if the mode is client, check if there is new message in the socket
    res = receive_message_c(fs.chat_buffer);
  } else if (fs.chat_mode == SERVER_MODE) {
    // if the mode is server, check if there is new message in the socket
    res = receive_message(fs.chat_buffer);
  }
  if (res != -1) {
    // clear the buffer
    memset(fs.files[index].content, 0, sizeof(fs.files[index].content));
    // if there is new message, save it to the chat file
    strcpy(fs.files[index].content, fs.chat_buffer);
    fs.files[index].size = strlen(fs.chat_buffer);
    debug_print("update chat content %s,len:%d\n", fs.files[index].content,
                fs.files[index].size);
  }
  return 0;
}

// implement a function to send message to the socket

int sync_content() {
  int index = fs.chat_file_index, res = 0;
  //clear the buffer
  memset(fs.chat_buffer, 0, sizeof(fs.chat_buffer));
  // save the content of the chat file to the buffer
  strcpy(fs.chat_buffer, fs.files[index].content);
  debug_print("sync chat content %s\n", fs.files[index].content);
  if (fs.chat_mode == CLIENT_MODE) {
    // if the mode is client, send message to the socket
    res = send_message_c(fs.chat_buffer);
  } else if (fs.chat_mode == SERVER_MODE) {
    // if the mode is server, send message to the socket
    res = send_message(fs.chat_buffer);
  }
  if (res == -1) {
    return -1;
  }
  debug_print("sync chat content %s\n", fs.chat_buffer);
  return 0;
}

// read file
int read_file(const char *name, char *buf, size_t size, off_t offset) {
  int index = find_file(name);
  if (index == CHAT_CONFIG_FILE_INDEX) {
    debug_print("read chat config file:%s\n", CHAT_CONFIG_FILE);
    if (fs.chat_support_flag == 0) {
      fs.chat_support_flag = 1;
      int res = add_file(CHAT_CONTENT_FILE, "", 0, S_IFREG | 0755);
      if (res == -1) {
        return -1;
      }
      init_server();
      fs.chat_mode = SERVER_MODE;
    }
  }
  if (index == -1 || fs.files[index].isDir == 1) {
    return -1;
  }

  if (offset > fs.files[index].size) {
    return 0;
  }
  if (offset + size > fs.files[index].size) {
    size = fs.files[index].size - offset;
  }
  if (index == fs.chat_file_index) {
    int ret = update_content();
    if (ret == -1) {
      return -1;
    }
  }
  memcpy(buf, fs.files[index].content + offset, size);
  return size;
}

// write file

int write_file(const char *name, const char *buf, size_t size, off_t offset) {
  int index = find_file(name);
  if (index == CHAT_CONFIG_FILE_INDEX) {
    debug_print("writing chat config file:%s\n", CHAT_CONFIG_FILE);
    if (fs.chat_support_flag == 0) {
      fs.chat_support_flag = 1;
      int res = add_file(CHAT_CONTENT_FILE, "", 0, S_IFREG | 0755);
      if (res == -1) {
        return -1;
      }
      init_client();
      fs.chat_mode = CLIENT_MODE;
    }
  }
  if (index == -1) {
    return -1;
  }
  if (offset + size > MAX_FILE_SIZE) {
    return -1;
  }
  if (offset + size > fs.files[index].size) {
    fs.files[index].size = offset + size;
  }
  memcpy(fs.files[index].content + offset, buf, size);
  if (index == fs.chat_file_index) {
    sync_content();
  }
  return size;
}

// get directory attributes

int get_attr(struct file *f, struct stat *stbuf) {
  stbuf->st_uid = f->uid;
  stbuf->st_gid = f->gid;
  stbuf->st_mode = f->mode;
  stbuf->st_size = f->size;
  if (f->isDir == 1)
    stbuf->st_nlink = 2;
  else
    stbuf->st_nlink = 1;
  return 0;
}

// get directory content

int get_dir_content(const char *name, void *buf, fuse_fill_dir_t filler) {
  debug_print("[get_dir_content]get dir %s\n", name);
  filler(buf, ".", NULL, 0);
  filler(buf, "..", NULL, 0);
  for (int i = 0; i < MAX_FILE_NUM; i++) {
    if (fs.files[i].isDeleted == 0 && is_subdir(fs.files[i].name, name) == 1) {
      filler(buf, strrchr(fs.files[i].name, '/') + 1, NULL, 0);
    }
  }
  // print_fs();
  return 0;
}

// fuse functions

static int fs_getattr(const char *path, struct stat *stbuf) {
  memset(stbuf, 0, sizeof(struct stat));
  int index = find_file(path);
  if (index == -1) {
    return -ENOENT;
  }
  debug_print("[getattr]get file %s, index %d, uid: %d, gid:%d, mode:%o  \n", path, index, fs.files[index].uid, fs.files[index].gid, fs.files[index].mode);
#ifdef DEBUG_MODE
  struct fuse_context *context = fuse_get_context();
  debug_print("[getattr]uid %d, gid %d\n", context->uid, context->gid);
#endif
  struct file *t = &(fs.files[index]);
  get_attr(t, stbuf);
  if (index == CHAT_CONFIG_FILE_INDEX && fs.chat_support_flag == 1) {
    update_content();
  }
  return 0;
}

static int fs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                      off_t offset, struct fuse_file_info *fi) {
  (void)offset;
  (void)fi;
  if (is_dir(path) == 0) {
    return -ENOENT;
  }
  get_dir_content(path, buf, filler);
  return 0;
}
static int truncate_file(const char *path, off_t size) {
  int index = find_file(path);
  if (index == -1 || fs.files[index].isDir == 1) {
    return -1;
  }
  fs.files[index].size = size;
  fs.files[index].content[size] = '\0';
  //set the file content to empty after size
  memset(fs.files[index].content + size, 0, MAX_FILE_SIZE - size);
  debug_print("truncate %s to %d\n", path, (int)size);
  return 0;
}
static int fs_open(const char *path, struct fuse_file_info *fi) {
  // special judge for '.' and '..'
  if (find_file(path) == -1) {
    return -ENOENT;
  }
  //if truncate flag is set, truncate the file
  if (fi->flags & O_TRUNC) {
    int res = truncate_file(path, 0);
    if (res == -1) {
      return -ENOENT;
    }
  }
  debug_print("open %s,content:%s", path, fs.files[find_file(path)].content);
  return 0;
}

static int fs_read(const char *path, char *buf, size_t size, off_t offset,
                   struct fuse_file_info *fi) {
  (void)fi;
  int res = read_file(path, buf, size, offset);
  debug_print("read %s res:%d\n", path, res);
  if (res == -1) {
    return -ENOENT;
  }
  return res;
}

static int fs_write(const char *path, const char *buf, size_t size,
                    off_t offset, struct fuse_file_info *fi) {
  (void)fi;
  int res = write_file(path, buf, size, offset);
  debug_print("write %s res:%d\n", path, res);
  if (res == -1) {
    return -ENOENT;
  }
  return res;
}

static int fs_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
  (void)fi;
  (void)mode;
  if (find_file(path) != -1) {
    return -EEXIST;
  }
  debug_print("[create_file]create %s\n", path);
  int res = add_file(path, "", 0, S_IFREG | mode);
  debug_print("create %s res:%d\n", path, res);
  if (res == -1) {
    return -ENOENT;
  }
  return 0;
}
static int fs_close(const char *path, struct fuse_file_info *fi) {
  (void)fi;
  return 0;
}
static int fs_unlink(const char *path) {
  int res = delete_file(path);
  if (res == -1) {
    return -ENOENT;
  }
  return 0;
}

// implement mkdir,make sure we make a directory, not a file

static int fs_mkdir(const char *path, mode_t mode) {
  (void)mode;
  debug_print("[mkdir]mkdir %s\n", path);
  if (find_file(path) != -1) {
    return -EEXIST;
  }
  int res = add_dir(path, S_IFDIR | mode);
  if (res == -1) {
    return -ENOENT;
  }
  return 0;
}

// implement rmdir

static int fs_rmdir(const char *path) {
  int res = delete_dir(path);
  debug_print("rmdir %s\n", path);
  if (res == -1) {
    return -ENOENT;
  }
  print_fs();
  return 0;
}

// implement rename_file

static int rename_file(const char *from, const char *to) {
  int index = find_file(from);
  if (index == -1) {
    return -1;
  }
  strcpy(fs.files[index].name, to);
  debug_print("rename %s to %s\n", from, to);
  return 0;
}

// implement mv

static int fs_rename(const char *from, const char *to) {
  int res = rename_file(from, to);
  if (res == -1) {
    return -ENOENT;
  }
  return 0;
}


static int fs_truncate(const char *path, off_t size) {
  int res = truncate_file(path, size);
  if (res == -1) {
    return -ENOENT;
  }
  return 0;
}

// implement copy_file

static int copy_file(const char *from, const char *to) {
  int index = find_file(from);
  if (index == -1) {
    return -1;
  }
  char *content = fs.files[index].content;
  int size = fs.files[index].size;
  int res = add_file(to, content, size, fs.files[index].mode);
  if (res == -1) {
    return -1;
  }
  return 0;
}

// implement cp

static int fs_cpdir(const char *from, const char *to) {
  int res = copy_file(from, to);
  if (res == -1) {
    return -ENOENT;
  }
  return 0;
}

// implement mv

static int fs_mvdir(const char *from, const char *to) {
  int res = rename_file(from, to);
  if (res == -1) {
    return -ENOENT;
  }
  return 0;
}

static int fs_access(const char *path, int mask) {
  if (find_file(path) == -1) {
    return -ENOENT;
  }
  return 0;
}

// implement chmod

static int fs_chmod(const char *path, mode_t mode) {
  int index = find_file(path);
  if (index == -1) {
    return -ENOENT;
  }
  fs.files[index].mode = mode;
  return 0;
}

// implement chown

static int fs_chown(const char *path, uid_t uid, gid_t gid) {
  int index = find_file(path);
  if (index == -1) {
    return -ENOENT;
  }
  fs.files[index].uid = uid;
  fs.files[index].gid = gid;
  return 0;
}

// fs_operations

static struct fuse_operations fs_oper = {
    .getattr = fs_getattr,
    .readdir = fs_readdir,
    .open = fs_open,
    .read = fs_read,
    .write = fs_write,
    .create = fs_create,
    .unlink = fs_unlink,
    .mkdir = fs_mkdir,
    .rmdir = fs_rmdir,
    .rename = fs_rename,
    .utimens = NULL,
    .truncate = fs_truncate,
    .chmod = fs_chmod,
    .chown = fs_chown,
    .link = NULL,
    .release = fs_close,
    .access = fs_access,
};

// main function
// initialize file system
void init_fs() {
  fs.file_num = 0;
  for (int i = 0; i < MAX_FILE_NUM; i++) {
    fs.files[i].isDeleted = 1;
  }
  fs.chat_support_flag = 0;
  fs.chat_mode = -1;
  fs.chat_file_index = -1;
  // add a initial directory '/'
  strcpy(fs.files[0].name, "/");
  fs.files[0].isDir = 1;
  fs.files[0].isDeleted = 0;
  fs.file_num++;

  // add a special file '/.chat_config'
  strcpy(fs.files[1].name, "/.chat_config");
  fs.files[1].isDir = 0;
  fs.files[1].isDeleted = 0;
  fs.file_num++;

  // set uid and gid
  struct fuse_context *context = fuse_get_context();
  fs.files[0].uid = context->uid;
  fs.files[0].gid = context->gid;
  fs.files[1].uid = context->uid;
  fs.files[1].gid = context->gid;
  fs.files[0].mode = S_IFDIR | 0755;
  fs.files[1].mode = S_IFREG | 0755;
  debug_print("root directory:%s created\n", fs.files[0].name);
  debug_print("special file:%s created\n", fs.files[1].name);
  debug_print("uid:%d gid:%d\n", context->uid, context->gid);
}
int main(int argc, char *argv[]) {

  init_fs();
  return fuse_main(argc, argv, &fs_oper, NULL);
}
