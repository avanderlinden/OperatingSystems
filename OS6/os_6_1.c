// 
// os_assignment.c
//
// This is the file you will be working in. You will have to complete the (partial) implementation
// given below. Any functions you have to complete are clearly marked as such.
//
// Good luck!
//

#define FUSE_USE_VERSION 31

#include <fuse.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdlib.h>
#include <assert.h>
#include "os_dfs.c"

dir_t* root_dir;

FILE * logfile;

void printlog( const char* str, ... );

/**
 * Here, we fill DFS with a number of directories and files.
 * Add, remove and rename some files and directories, if you wish.
 * 
 * Note how there is a top-level directory root_dir. You will have to use this directory.
 */
void create_dfs() {
    // Define some directories
    root_dir = create_dir("Root");
    dir_t* dir_1 = create_dir("Files");
    dir_t* dir_2 = create_dir("Pictures");
    dir_t* dir_3 = create_dir("Important files");

    // Define some files
    file_t* file_0 = create_file("Readme.txt", "Welcome to the DFS.");
    file_t* file_1 = create_file("Budget.txt", "We're all out of money!");
    file_t* file_2 = create_file("Hello.txt", "Hello there!");
    file_t* file_3 = create_file("Hello2.txt", "Hello there!");
    file_t* file_4 = create_file("some_image", "I am an image");
    file_t* file_5 = create_file("some_other_image", "I too am an image");

    // Add everything together
    add_dir_to(root_dir, dir_1);
    add_dir_to(root_dir, dir_2);
    add_dir_to(dir_1, dir_3);
    add_file_to(root_dir, file_0);
    add_file_to(dir_1, file_2);
    add_file_to(dir_1, file_3);
    add_file_to(dir_2, file_4);
    add_file_to(dir_2, file_5);
    add_file_to(dir_3, file_1);
}

/**
 * This function must be completed for the assignments!
 * 
 * readdir() lists files/directories available in a specific directory
 * - path: directory which we are listing
 * - buff: buffer to fill with data
 * - fill: a function to help fill buffer
 * - off: not used
 * - fi: not used
 * Returns either 0 or an appropriate error code
 */
static int os_readdir(const char *path, void *buff, fuse_fill_dir_t fill, off_t off, struct fuse_file_info *fi) {
    // Example: this is how you find the required directory.
    // This could be NULL though! You should return an error if this is so, in other functions as well
    dir_t* dir = find_dir(root_dir, path);
    if (dir == NULL) {
        return -ENOENT; // Error: no entity
    }

    // Example: this is how you add a folder "Documents" to the buffer
    // although this "Documents" folder does not exist.
    fill(buff, ".", NULL, 0);
    fill(buff, "..", NULL, 0);

    // Example: this is how you could iterate over all files in dir
    // and it's almost the same for directories
    for (unsigned int i = 0; i < dir->files->length; i++) {
        file_t* ptr = (file_t *) get_p(dir->files, i);
        fill(buff, ptr->name, NULL, 0 );
        // Do something with this file
    }

    for (unsigned int i = 0; i < dir->dirs->length; i++) {
        dir_t* ChildDir = (dir_t *) get_p(dir->dirs, i);
        fill(buff, ChildDir->name, NULL, 0 );
        // Do something with this file
    }

    // Returning 0 means all-ok.
    return 0;
}

/**
 * This function must be completed for the assignments!
 * 
 * mkdir() creates a directory.
 * - path: path leading to the new directory
 * Returns 0 if successful, appropriate error otherwise.
 */
static int os_mkdir(const char* path, mode_t mode) {

    printlog("LOG:[mkdir] mkdir path: %s\n", path);


    dir_t* parent = NULL;
    int delPos = 0;
    char dirName[80];
    int size = strlen(path);
    for(int i = size-1; i>=0; i--){
        if( path[i] == '/' ){
            delPos = i;
            break;
        }
    }

    printlog("LOG:[mkdir] delPos: %d\n", delPos);

    memcpy(dirName, path+(delPos+1), size-(delPos+1));
    dirName[size-(delPos+1)] = '\0';

    printlog("LOG:[mkdir] dirname: %s\n", dirName);

    if(delPos == 0){
        parent = find_dir(root_dir, "/");
    }
    else {
        char wholeRoot[80];
        char root[80];
        int rSize = 0;
        int rDelPos = 0;

        memcpy(wholeRoot, path, delPos);
        wholeRoot[delPos] = '\0';

        printlog("LOG:[mkdir] wholeRoot: %s\n", wholeRoot);

        rSize = strlen(wholeRoot);

        for(int i = rSize-1; i>=0; i--){
            if( wholeRoot[i] == '/' ){
                rDelPos = i;
            }
        }
        printlog("LOG:[mkdir] rDelPos: %d\n", rDelPos);

        memcpy(root, wholeRoot+rDelPos, rSize-rDelPos);
        root[rSize-rDelPos] = '\0';

        printlog("LOG:[mkdir] root: %s\n", root);

        parent = find_dir(root_dir, root);
    }

    if (parent == NULL) {
        printlog("ERROR:[mkdir] root dir not found \n");
        return -ENOENT; // Error: no entity
    }

    dir_t* child = create_dir(dirName);
    add_dir_to(parent, child);

    return 0;
}

/**
 * This function must be completed for the assignments!
 * 
 * rmdir() removes a directory (and its contents).
 * - path: path leading to the directory.
 * Returns 0, appropriate error otherwise.
 */
static int os_rmdir(const char* path) {

    printlog("LOG:[rmdir] delete directory %s\n", path);

    dir_t* target = find_dir(root_dir, path);
    if (target == NULL) {
        printlog("ERROR:[rmdir] dir not found %s\n", path);
        return -ENOENT; // Error: no entity
    }

    destroy_dir(target);

    return 0;
}

/**
 * This function must be completed for the assignments!
 * 
 * read() reads (part of, aka a 'chunk' of) a file
 * - path: file to be read
 * - buff: buffer to fill with chunk
 * - size: size of the chunk to be read
 * - off: offset from where chunk starts
 * - fi: not used
 * returns number of bytes read
 */
static int os_read(const char* path, char* buff, size_t size, off_t off, struct fuse_file_info* fi) {

    printlog("LOG:[read] read file %s\n", path);

    dir_t* parent = NULL;
    int delPos = 0;
    char fileName[80];
    int pSize = strlen(path);
    for(int i = pSize-1; i>=0; i--){
        if( path[i] == '/' ){
            delPos = i;
            break;
        }
    }

    printlog("LOG:[read] delPos: %d\n", delPos);

    memcpy(fileName, path+(delPos+1), pSize-(delPos+1));
    fileName[pSize-(delPos+1)] = '\0';

    printlog("LOG:[read] file name: %s\n", fileName);


    if(delPos == 0){
        parent = find_dir(root_dir, "/");
    }
    else {
        char wholeRoot[80];
        char root[80];
        int rSize = 0;
        int rDelPos = 0;

        memcpy(wholeRoot, path, delPos);
        wholeRoot[delPos] = '\0';

        printlog("LOG:[read] wholeRoot: %s\n", wholeRoot);

        rSize = strlen(wholeRoot);

        for(int i = rSize-1; i>=0; i--){
            if( wholeRoot[i] == '/' ){
                rDelPos = i;
            }
        }
        printlog("LOG:[read] rDelPos: %d\n", rDelPos);

        memcpy(root, wholeRoot+rDelPos, rSize-rDelPos);
        root[rSize-rDelPos] = '\0';

        printlog("LOG:[read] root: %s\n", root);

        parent = find_dir(root_dir, root);
    }

    if (parent == NULL) {
        printlog("ERROR:[read] root dir not found \n");
        return -ENOENT; // Error: no entity
    }


    file_t* filePtr = NULL;
    for (unsigned int i = 0; i < parent->files->length; i++) {
        file_t* ptr = (file_t *) get_p(parent->files, i);
        if (memcmp(ptr->name, fileName, strlen(fileName)) == 0){
            filePtr = ptr;
        }
    }

    if (filePtr == NULL) {
        printlog("ERROR:[read] file not found in root dir\n");
        return -ENOENT; // Error: no entity
    }

    printlog("LOG:[read] Contents: \n%s\n", filePtr->contents);


    size_t len = strlen(filePtr->contents);

    if (off < len) {
        if (off + size > len){
            size = len - off;
        }
        memcpy(buff, filePtr->contents+off, size);
    }
    else {
        size = 0;
    }
    return size;
}

/**
 * This function must be completed for the assignments!
 * 
 * write() writes (part of, aka a 'chunk' of) a file.
 * - path: file to be written to
 * - buff: buffer with data to write
 * - size: size of the chunk to be written
 * - off: offset from where chunk should start
 * - fi: not used
 */
static int os_write(const char* path, const char* buff, size_t size, off_t off, struct fuse_file_info* fi) {

    printlog("LOG:[write] read file %s\n", path);

    dir_t* parent = NULL;
    int delPos = 0;
    char fileName[80];
    int pSize = strlen(path);
    for(int i = pSize-1; i>=0; i--){
        if( path[i] == '/' ){
            delPos = i;
            break;
        }
    }

    printlog("LOG:[write] delPos: %d\n", delPos);

    memcpy(fileName, path+(delPos+1), pSize-(delPos+1));
    fileName[pSize-(delPos+1)] = '\0';

    printlog("LOG:[write] file name: %s\n", fileName);


    if(delPos == 0){
        parent = find_dir(root_dir, "/");
    }
    else {
        char wholeRoot[80];
        char root[80];
        int rSize = 0;
        int rDelPos = 0;

        memcpy(wholeRoot, path, delPos);
        wholeRoot[delPos] = '\0';

        printlog("LOG:[write] wholeRoot: %s\n", wholeRoot);

        rSize = strlen(wholeRoot);

        for(int i = rSize-1; i>=0; i--){
            if( wholeRoot[i] == '/' ){
                rDelPos = i;
            }
        }
        printlog("LOG:[write] rDelPos: %d\n", rDelPos);

        memcpy(root, wholeRoot+rDelPos, rSize-rDelPos);
        root[rSize-rDelPos] = '\0';

        printlog("LOG:[write] root: %s\n", root);

        parent = find_dir(root_dir, root);
    }

    if (parent == NULL) {
        printlog("ERROR:[write] root dir not found \n");
        return -ENOENT; // Error: no entity
    }


    file_t* filePtr = NULL;
    for (unsigned int i = 0; i < parent->files->length; i++) {
        file_t* ptr = (file_t *) get_p(parent->files, i);
        if (memcmp(ptr->name, fileName, strlen(fileName)) == 0){
            filePtr = ptr;
        }
    }

    if (filePtr == NULL) {
       printlog("ERROR:[write] file not found in root dir\n");
       return -ENOENT; // Error: no entity
   }

    printlog("LOG:[write] Contents: \n  %s\n", buff);

    memcpy(filePtr->contents + off, buff, size);

    return size;
}

/**
 * This function must be completed for the assignments!
 * 
 * Create a file at specified path, with given mode.
 * - path: path leading to the file.
 * - mode: not used.
 * - fi: not used.
 */
static int os_create(const char * path, mode_t mode, struct fuse_file_info* fi) {

    printlog("LOG:[create] read file %s\n", path);

    dir_t* parent = NULL;
    int delPos = 0;
    char fileName[80];
    int pSize = strlen(path);
    for(int i = pSize-1; i>=0; i--){
        if( path[i] == '/' ){
            delPos = i;
            break;
        }
    }

    printlog("LOG:[create] delPos: %d\n", delPos);

    memcpy(fileName, path+(delPos+1), pSize-(delPos+1));
    fileName[pSize-(delPos+1)] = '\0';

    printlog("LOG:[create] file name: %s\n", fileName);


    if(delPos == 0){
        parent = find_dir(root_dir, "/");
    }
    else {
        char wholeRoot[80];
        char root[80];
        int rSize = 0;
        int rDelPos = 0;

        memcpy(wholeRoot, path, delPos);
        wholeRoot[delPos] = '\0';

        printlog("LOG:[create] wholeRoot: %s\n", wholeRoot);

        rSize = strlen(wholeRoot);

        for(int i = rSize-1; i>=0; i--){
            if( wholeRoot[i] == '/' ){
                rDelPos = i;
            }
        }
        printlog("LOG:[create] rDelPos: %d\n", rDelPos);

        memcpy(root, wholeRoot+rDelPos, rSize-rDelPos);
        root[rSize-rDelPos] = '\0';

        printlog("LOG:[create] root: %s\n", root);

        parent = find_dir(root_dir, root);
    }

    if (parent == NULL) {
        printlog("ERROR:[create] root dir not found \n");
        return -ENOENT; // Error: no entity
    }


    file_t* filePtr = NULL;
    for (unsigned int i = 0; i < parent->files->length; i++) {
        file_t* ptr = (file_t *) get_p(parent->files, i);
        if (memcmp(ptr->name, fileName, strlen(fileName)) == 0){
            filePtr = ptr;
        }
    }

    if (filePtr != NULL) {
       printlog("ERROR:[create] file exists\n");
       return -ENOENT; // Error: no entity
    }


    filePtr = create_file(fileName, "");
    add_file_to(parent, filePtr);

    return 0;
}

/**
 * This function must be completed for the assignments!
 * 
 * unlink() unlinks (deletes) a file.
 * - path: path leading to the file
 * Returns 0 if successful, appropriate error otherwise.
 */
static int os_unlink(const char* path) {
    printlog("LOG:[unlink] read file %s\n", path);

    dir_t* parent = NULL;
    int delPos = 0;
    char fileName[80];
    int pSize = strlen(path);
    for(int i = pSize-1; i>=0; i--){
       if( path[i] == '/' ){
           delPos = i;
           break;
       }
    }

    printlog("LOG:[unlink] delPos: %d\n", delPos);

    memcpy(fileName, path+(delPos+1), pSize-(delPos+1));
    fileName[pSize-(delPos+1)] = '\0';

    printlog("LOG:[unlink] file name: %s\n", fileName);


    if(delPos == 0){
       parent = find_dir(root_dir, "/");
    }
    else {
       char wholeRoot[80];
       char root[80];
       int rSize = 0;
       int rDelPos = 0;

       memcpy(wholeRoot, path, delPos);
       wholeRoot[delPos] = '\0';

       printlog("LOG:[unlink] wholeRoot: %s\n", wholeRoot);

       rSize = strlen(wholeRoot);

       for(int i = rSize-1; i>=0; i--){
           if( wholeRoot[i] == '/' ){
               rDelPos = i;
           }
       }
       printlog("LOG:[unlink] rDelPos: %d\n", rDelPos);

       memcpy(root, wholeRoot+rDelPos, rSize-rDelPos);
       root[rSize-rDelPos] = '\0';

       printlog("LOG:[unlink] root: %s\n", root);

       parent = find_dir(root_dir, root);
    }

    if (parent == NULL) {
       printlog("ERROR:[unlink] root dir not found \n");
       return -ENOENT; // Error: no entity
    }


    file_t* filePtr = NULL;
    for (unsigned int i = 0; i < parent->files->length; i++) {
       file_t* ptr = (file_t *) get_p(parent->files, i);
       if (memcmp(ptr->name, fileName, strlen(fileName)) == 0){
           filePtr = ptr;
       }
    }

    if (filePtr == NULL) {
      printlog("ERROR:[unlink] file does not exists\n");
      return -ENOENT; // Error: no entity
    }

    destroy_file(filePtr);

    return 0;
}


/**
 * You do not have to modify this for the assignments.
 * 
 * getattr() gets file/directory attributes.
 * - path: file or directory path for which attributes are requested.
 * - st: struct of type stat, to be filled with file/directory attributes.
 * returns 0 or appropriate error code.
 */
static int os_getattr(const char *path, struct stat *st) {
    // Uncertain whether its a file or a directory, so we just look for both
    file_t* file = find_file(root_dir, path);
    dir_t* dir = find_dir(root_dir, path);

    if (file != NULL) {
        st->st_mode = S_IFREG | 0444; // Requested item is (probably) a regular file
        st->st_nlink = 1; // Number of hard links: a file has at least one
        st->st_size = strlen(file->contents);
    } else if (dir != NULL) {
        st->st_mode = S_IFDIR | 0755;
        st->st_nlink = 2 + dir->dirs->length;
    } else {
        return -ENOENT;
    }

    return 0;
}

/**
 * You do not have to modify this for the assignments.
 * 
 * truncate() resizes a file by offset.
 * - path: file location.
 * - off: the offset to resize by.
 * Returns 0 or appropriate error.
 */
static int os_truncate(const char* path, off_t off) { 
    file_t* file = find_file(root_dir, path);
    if (file == NULL) {
        return -ENOENT;
    }

    size_t cur_length = strlen(file->contents);
    if (off > cur_length) {
        size_t new_length = cur_length + off;
        file->contents = realloc(file->contents, new_length);
        memset(file->contents + cur_length, ' ', off);
    } else {
        size_t new_length = cur_length - off;
        file->contents = realloc(file->contents, new_length);
    }

    return 0;
}



/**
 * You do not have to modify these for the assignments.
 * 
 * These are some stub functions that FUSE must see, but do not modify our file system.
 */
static int os_setxattr(const char* a, const char* b, const char* c, size_t d, int e) { return 0; }
static int os_chmod(const char* a, mode_t b) { return 0; }
static int os_chown(const char* a, uid_t b, gid_t c) { return 0; }
static int os_utimens(const char* a, const struct timespec tv[2]) { return 0; }

/**
 * You do not have to modify these for the assignments.
 * 
 * Here we define the list of operations FUSE has access to.
 * If operations are not defined here, FUSE considers these 'not implemented'
 * and will not allow you to call them!
 */
static struct fuse_operations operations = {
    .getattr = os_getattr,
    .readdir = os_readdir,
    .read = os_read,
    .mkdir = os_mkdir,
    .rmdir = os_rmdir,
    .write = os_write,
    .setxattr = os_setxattr,
    .truncate = os_truncate,
    .chmod = os_chmod,
    .chown = os_chown,
    .utimens = os_utimens,
    .create = os_create,
    .unlink = os_unlink
};


void printlog( const char* str, ... ){
    va_list argp;

    logfile = fopen("dfs_log.txt", "a");

    va_start(argp, str);
    vfprintf(logfile, str, argp);
    va_end(argp);

    fclose(logfile);
}

/**
 * Main function.
 */
int main (int argc, char *argv[]) {


    printlog("Create DFS\n");

    // Setup dumb file system
    create_dfs();



    // Pass arguments on to FUSE.
	return fuse_main(argc, argv, &operations, NULL);
}
