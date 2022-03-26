#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>

#include <wordexp.h>
#include <ndbm.h>

#define EXIT_SUCCESS_NO_JUMP 2

const char* DB_FNAME = "~/.warp";
DBM* DB = NULL;

int warp_db_open(int open_flags, mode_t file_mode)
{
    wordexp_t exp_result;
    int wordexp_err = wordexp(DB_FNAME, &exp_result, 0);
    if (wordexp_err != 0) {
        fprintf(stderr, "Failed to find warp database: '%s'\n", DB_FNAME);
        return (wordexp_err << 1) | 0;
    }
    DB = dbm_open(exp_result.we_wordv[0], open_flags, file_mode);
    wordfree(&exp_result);
    if (DB == NULL) {
        fprintf(stderr, "Failed to open warp database: '%s'\n", DB_FNAME);
        return (errno << 1) | 1;
    } else {
        return 0;
    }
}

void warp_db_close(void) {
    dbm_close(DB);
}

int warp_db_find(const char* warp_point, datum* location) {
    datum key = {
        .dptr = (void*)warp_point,
        .dsize = strnlen(warp_point, 255) + 1
    };

    *location = dbm_fetch(DB, key);
    if (location->dptr == NULL) {
        fprintf(stderr, "Unable to find warp point for location '%s'\n", warp_point);
        return 1;
    }

    return 0;
}

int warp_db_store(const char* warp_point, const char* path, int store_mode) {
    datum key = {
        .dptr = (void*)warp_point,
        .dsize = strnlen(warp_point, 255) + 1
    };

    datum value = {
        .dptr = (void*)path,
        .dsize = strnlen(path, 4095) + 1
    };

    return dbm_store(DB, key, value, store_mode);
}

int warp_db_remove(const char* warp_point, bool quiet) {
    datum key = {
        .dptr = (void*)warp_point,
        .dsize = strnlen(warp_point, 255) + 1
    };

    bool exists = dbm_fetch(DB, key).dptr != NULL;
    int remove_err = dbm_delete(DB, key);
    if (!exists && remove_err != 0) {
        if (quiet) {
            return 0;
        } else {
            return abs(remove_err);
        }
    } else {
        return remove_err;
    }
}

bool warp_should_help(int argc, const char* argv[]) {
    for (int i = 0; i < argc; i++) {
        if (strncmp(argv[i], "-h", 3) == 0 || strncmp(argv[i], "--help", 7) == 0) {
            return true;
        }
    }
    return false;
}

void warp_help_list(void) {
    puts("list: show all the known warp points.");
    puts("\nUsage:");
    puts(" list [-p]");
    puts("\nArguments:");
    puts(" -p --paths   Show the paths alongside the keys.");
    puts(" -h --help    Show this help and stop.");
}

int warp_list(int argc, const char* argv[]) {
    if (warp_should_help(argc, argv)) {
        warp_help_list();
        return EXIT_SUCCESS_NO_JUMP;
    }

    bool show_targets = false;
    for (int i = 2; i < argc && !show_targets; i++) {
        if (strncmp(argv[i], "-p", 3) == 0 || strncmp(argv[i], "--paths", 8) == 0) {
            show_targets = true;
        }
    }

    int open_err = warp_db_open(O_RDWR | O_CREAT, 0660);
    if (open_err != 0) {
        return EXIT_FAILURE;
    }

    for (datum key = dbm_firstkey(DB); key.dptr != NULL; key = dbm_nextkey(DB)) {
        if (show_targets) {
            datum location;
            int location_err = warp_db_find((char *)key.dptr, &location);
            if (location_err != 0) {
                warp_db_close();
                return EXIT_FAILURE;
            }
            printf("%s\t%s\n", (char *)key.dptr, (char *)location.dptr);
        } else {
            printf("%s\n", (char *)key.dptr);
        }
    }

    warp_db_close();
    return EXIT_SUCCESS_NO_JUMP;
}

void warp_help_jump(void) {
    puts("jump: output the associated location for a warp point to then jump.");
    puts("\nUsage:");
    puts(" jump [name] [--no-prompt]");
    puts("\nArguments:");
    puts(" name         The name of the warp point to jump to.");
    puts("              If not given, will prompt for the name.");
    puts(" --no-prompt  If the name is not given as an argument, fail rather than prompt.");
    puts(" -h --help    Show this help and stop.");
}

int warp_jump(int argc, const char* argv[]) {
    if (warp_should_help(argc, argv)) {
        warp_help_jump();
        return EXIT_SUCCESS_NO_JUMP;
    }

    bool prompt = true;
    int path_arg = -1;
    for(int i = 2; i < argc && (prompt || path_arg == -1); i++) {
        if (strncmp(argv[i], "--no-prompt", 12) == 0) {
            prompt = false;
        } else {
            path_arg = i;
        }
    }

    char* warp_point = calloc(sizeof(char), 256);
    if (path_arg == -1 && prompt) {
        printf("Warp point name: ");
        fgets(warp_point, 255, stdin);
        warp_point[strcspn(warp_point, "\n")] = '\0';
    } else if (path_arg == -1 && !prompt) {
        fprintf(stderr, "Invalid warp point.\n");
        return EXIT_FAILURE;
    } else {
        strncpy(warp_point, argv[path_arg], 255);
    }

    int open_err = warp_db_open(O_RDWR | O_CREAT, 0660);
    if (open_err != 0) {
        return (open_err << 1) | 0;
    }

    datum location;
    int location_err = warp_db_find(warp_point, &location);
    if (location_err != 0) {
        return EXIT_FAILURE;
    }

    printf("%s\n", (char*)location.dptr);

    free(warp_point);
    warp_db_close();
    return EXIT_SUCCESS;
}

void warp_help_dir(void) {
    puts("dir: show the target directory for a warp point.");
    puts("\nUsage:");
    puts(" dir [name]");
    puts("\nArguments:");
    puts(" name         The name of the warp point to jump to.");
    puts("              If not given, will prompt for the name.");
    puts(" --no-prompt  If the name is not given as an argument, fail rather than prompt.");
    puts(" -h --help    Show this help and stop.");
}

int warp_dir(int argc, const char* argv[]) {
    if (warp_should_help(argc, argv)) {
        warp_help_dir();
        return EXIT_SUCCESS_NO_JUMP;
    }

    int retn = warp_jump(argc, argv);
    if (retn != EXIT_SUCCESS) {
        return retn;
    }
    return EXIT_SUCCESS_NO_JUMP;
}

void warp_help_set(void) {
    puts("set: create a new warp point to jump to.");
    puts("\nUsage:");
    puts(" set [name] [path] [-f]");
    puts("\nArguments:");
    puts(" name         The name of the warp point to jump to.");
    puts("              If not given, will prompt for the name.");
    puts(" path         The directory to which the warp point will jump.");
    puts("              If not given, uses the current working directory.");
    puts(" --no-prompt  If name or path are not given as arguments, fail rather than prompt.");
    puts(" -f --force   Set this warp point, even if one of the same name already exists.");
    puts(" -h --help    Show this help and stop.");
}

int warp_set(int argc, const char* argv[]) {
    if (warp_should_help(argc, argv)) {
        warp_help_set();
        return EXIT_SUCCESS_NO_JUMP;
    }

    bool prompt = true;
    bool force = false;
    int point_arg = -1;
    int path_arg = -1;
    for (int i = 2; i < argc && (path_arg == -1 || point_arg == -1 || !force || prompt); i++) {
        if (strncmp(argv[i], "--no-prompt", 12) == 0) {
            prompt = false;
        }
        else if (strncmp(argv[i], "-f", 3) == 0 || strncmp(argv[i], "--force", 8) == 0) {
            force = true;
        } else if (point_arg == -1) {
            point_arg = i;
        } else {
            path_arg = i;
        }
    }

    char* warp_point = calloc(sizeof(char), 256);
    char* warp_path;

    if (point_arg == -1 && prompt) {
        printf("Warp point name: ");
        fgets(warp_point, 255, stdin);
        warp_point[strcspn(warp_point, "\n")] = '\0';
    } else if (point_arg == -1 && !prompt) {
        fprintf(stderr, "Invalid warp point name.\n");
        return EXIT_FAILURE;
    } else {
        strncpy(warp_point, argv[point_arg], 255);
    }

    if (path_arg == -1 && prompt) {
        warp_path = getcwd(NULL, 0);
        if (warp_path == NULL) {
            if (errno == ERANGE) {
                fprintf(stderr, "Current directory path too long\n");
            } else {
                fprintf(stderr, "Failure reading current directory\n");
            }
            free(warp_point);
            return EXIT_FAILURE;
        }
    } else if (path_arg == -1 && !prompt) {
        fprintf(stderr, "Invalid warp point name.\n");
        free(warp_point);
        return EXIT_FAILURE;
    } else {
        wordexp_t exp_result;
        int wordexp_err = wordexp(argv[path_arg], &exp_result, 0);
        if (wordexp_err != 0) {
            fprintf(stderr, "Invalid directory path: '%s'\n", argv[path_arg]);
            free(warp_point);
            return (wordexp_err << 1) | 0;
        }
        warp_path = calloc(sizeof(char), 4096);
        strncpy(warp_path, exp_result.we_wordv[0], 4096);
        wordfree(&exp_result);
    }

    warp_db_open(O_RDWR | O_CREAT, 0660);
    int store_error =
        warp_db_store(warp_point, warp_path, force ? DBM_REPLACE : DBM_INSERT);
    warp_db_close();

    free(warp_point);
    free(warp_path);

    if (store_error > 0) {
      fprintf(stderr,
              "Warp point already exists; use a new name, or --force.\n");
      return EXIT_FAILURE;
    } else if (store_error < 0) {
      fprintf(stderr, "Error storing new warp point.\n");
      return EXIT_FAILURE;
    } else {
      return EXIT_SUCCESS_NO_JUMP;
    }
}

void warp_help_remove(void) {
    puts("remove: remove an existing warp point.");
    puts("\nUsage:");
    puts(" remove [name] [-q]");
    puts("\nArguments:");
    puts(" name         The warp point to remove, or fail if it does not exist.");
    puts("              If not given, will prompt for the name.");
    puts(" --no-prompt  If the name is not given as an argument, fail rather than prompt.");
    puts(" -q --quiet   If the warp point being removed does not exist, still succeed.");
    puts(" -h --help    Show this help and stop.");

}

int warp_remove(int argc, const char* argv[]) {
    if (warp_should_help(argc, argv)) {
        warp_help_remove();
        return EXIT_SUCCESS_NO_JUMP;
    }

    bool prompt = true;
    bool quiet = false;
    int point_arg = -1;
    for (int i = 2; i < argc && (point_arg == -1 || !quiet || prompt); i++) {
        if (strncmp(argv[i], "--no-prompt", 12) == 0) {
            prompt = false;
        } else if (strncmp(argv[i], "-q", 3) == 0 || strncmp(argv[i], "--quiet", 8) == 0) {
            quiet = true;
        } else {
            point_arg = i;
        }
    }

    char* warp_point = calloc(sizeof(char), 256);

    if (point_arg == -1 && prompt) {
        printf("Warp point name: ");
        fgets(warp_point, 255, stdin);
        warp_point[strcspn(warp_point, "\n")] = '\0';
    } else if (point_arg == -1 && !prompt) {
        fprintf(stderr, "Invalid warp point name.\n");
        return EXIT_FAILURE;
    } else {
        strncpy(warp_point, argv[point_arg], 255);
    }

    warp_db_open(O_RDWR | O_CREAT, 0660);
    int remove_error =
        warp_db_remove(warp_point, quiet);
    warp_db_close();
    free(warp_point);

   if (remove_error > 0 && !quiet) {
      fprintf(stderr, "Warp point does not exist; consider '--quiet'.\n");
      return EXIT_FAILURE;
    } else if (remove_error < 0) {
      fprintf(stderr, "Error removing warp point.\n");
      return EXIT_FAILURE;
    } else {
      return EXIT_SUCCESS_NO_JUMP;
    }
}

int warp_help(int argc, const char* argv[]) {
    if (argc < 3) {
        printf("%s: jump to a previously saved location\n\n", argv[0]);
        puts("Commands:");
        puts("  help    Show this help information");
        if (argc == 2) {
            puts("  help [tool]   Show more help about a particular tool.");
        }
        puts("  list    List the known warp points");
        puts("  jump    Jump* to a specified warp point");
        puts("  set     Set a new warp point");
        puts("  remove  Remove a known warp point");
        puts("  dir     Show the directory associated with a warp point\n");
        printf("* %s cannot jump itself; it will output the directory to jump to.\n\n", argv[0]);
        printf("%s will exit with return code %d if the command is 'jump' and the\n", argv[0], EXIT_SUCCESS);
        printf("warp point is known; if there is an error it will return exit code %d; and if the\n", EXIT_FAILURE);
        printf("tool succeeds but this should not result in a jump, it returns exit code %d.\n", EXIT_SUCCESS_NO_JUMP);
        return EXIT_SUCCESS_NO_JUMP;
    }

    if (strncmp(argv[2], "l", 2) == 0 || strncmp(argv[2], "list", 5) == 0) {
        warp_help_list();
    } else if (strncmp(argv[2], "j", 2) == 0 || strncmp(argv[2], "jump", 5) == 0) {
        warp_help_jump();
    } else if (strncmp(argv[2], "s", 2) == 0 || strncmp(argv[2], "set", 4) == 0) {
        warp_help_set();
    } else if (strncmp(argv[2], "r", 2) == 0 || strncmp(argv[2], "remove", 7) == 0) {
        warp_help_remove();
    } else if (strncmp(argv[2], "d", 2) == 0 || strncmp(argv[2], "dir", 4) == 0) {
        warp_help_dir();
    } else {
        fprintf(stderr, "Cannot help with unknown warp tool: '%s'.\n", argv[2]);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS_NO_JUMP;

}

int main(int argc, const char* argv[]) {
    if (argc < 2) {
        return warp_help(argc, argv);
    }

    if (strncmp(argv[1], "h", 2) == 0 || strncmp(argv[1], "help", 5) == 0 || strncmp(argv[1], "-h", 3) == 0 || strncmp(argv[1], "--help", 7) == 0) {
        return warp_help(argc, argv);
    } else if (strncmp(argv[1], "l", 2) == 0 || strncmp(argv[1], "list", 5) == 0) {
        return warp_list(argc, argv);
    } else if (strncmp(argv[1], "j", 2) == 0 || strncmp(argv[1], "jump", 5) == 0) {
        return warp_jump(argc, argv);
    } else if (strncmp(argv[1], "s", 2) == 0 || strncmp(argv[1], "set", 4) == 0) {
        return warp_set(argc, argv);
    } else if (strncmp(argv[1], "r", 2) == 0 || strncmp(argv[1], "remove", 7) == 0) {
        return warp_remove(argc, argv);
    } else if (strncmp(argv[1], "d", 2) == 0 || strncmp(argv[1], "dir", 4) == 0) {
        return warp_dir(argc, argv);
    } else {
        fprintf(stderr, "Unknown warp tool: '%s'\n", argv[1]);
        return EXIT_FAILURE;
    }
}
