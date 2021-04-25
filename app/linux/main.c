/**
    N(o) S(pecial) G(ame) B(oy) E(mulator)
    Copyright (C) 2021  Noeliel

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
**/


#include <signal.h>
#include <stdlib.h>
#include <pthread.h>

#include <nsgbe.h>

char *rompath;
char *biospath;

extern int gui_main(int argc, char **argv);

char *get_bios_path()
{
    return biospath;
}

char *get_rom_path()
{
    return rompath;
}

long file_read(byte **buffer, char *path)
{
    FILE *fbuf = fopen(path, "r");
    
    if (!fbuf)
    {
        printf("Error trying to open file: %s\n", path);
        return 0;
    }
    
    fseek(fbuf, 0, SEEK_END);
    long fsize = ftell(fbuf);
    rewind(fbuf);
    
    *buffer = malloc(fsize);
    if (!fread(*buffer, fsize, 1, fbuf))
    {
        printf("Error trying to read file: %s\n", path);
        return 0;
    }
    
    fclose(fbuf);

    return fsize;
}

int file_write(byte *buffer, char *path)
{

}

static void catch_exit(int signal_num)
{
    write_battery();    
    exit(0);
}

int main(int argc, char **argv) {

    pthread_t core_thread;
    pthread_attr_t core_thread_attributes;

    if (argc != 2)
        exit(0);

    if (signal(SIGTERM, catch_exit) == SIG_ERR) {
        printf("Failed to set up SIGTERM handler.\n");
        return EXIT_FAILURE;
    }

    if (signal(SIGINT, catch_exit) == SIG_ERR) {
        printf("Failed to set up SIGINT handler.\n");
        return EXIT_FAILURE;
    }

    if (signal(SIGABRT, catch_exit) == SIG_ERR) {
        printf("Failed to set up SIGABRT handler.\n");
        return EXIT_FAILURE;
    }
    
    rompath = argv[1];

#define LAUNCH_WITH_GUI 1
#ifdef LAUNCH_WITH_GUI
    pthread_attr_init(&core_thread_attributes);
    pthread_create(&core_thread, &core_thread_attributes, system_run, NULL);
    gui_main(1, argv);
#else
    system_run();
#endif

    return 0;
}
