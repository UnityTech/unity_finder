// vim: et ts=4 sw=4 tw=0
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>

typedef struct direntry_s
{
    char *path;
    struct direntry_s *next;
} direntry_t;

static int num_found = 0;

static BOOL is_file_present_in_folder(const char *path, const char *file)
{
    size_t l;
    char *file_path = NULL;
    BOOL res;
    struct stat st;

    l = strlen(path) + strlen (file) + 2;
    file_path = malloc(l);
    _snprintf(file_path, l, "%s\\%s", path, file);

    res = !stat(file_path, &st) && (st.st_mode&(S_IFREG | S_IFDIR));
    
    free(file_path);

    return res;
}

static BOOL is_unity_editor_folder(const char *path)
{
    if (!is_file_present_in_folder(path, "Unity.exe"))
    {
        return FALSE;
    }
    if (!is_file_present_in_folder(path, "Data"))
    {
        return FALSE;
    }
    return TRUE;
}

static void write_unity_info(const char *path, const char *file)
{
    size_t l;
    char *file_path = NULL;
    DWORD  verHandle = 0;
    UINT   size = 0;
    LPBYTE lpBuffer = NULL;
    DWORD  verSize;

    l = strlen(path) + strlen(file) + 2;
    file_path = malloc(l);
    _snprintf(file_path, l, "%s\\%s", path, file);

    verSize = GetFileVersionInfoSize(file_path, &verHandle);

    if (verSize != 0)
    {
        LPSTR verData = malloc (verSize);

        if (GetFileVersionInfo(file_path, verHandle, verSize, verData))
        {
            if (VerQueryValue(verData, "\\", (VOID FAR* FAR*)&lpBuffer, &size))
            {
                if (size)
                {
                    VS_FIXEDFILEINFO *verInfo = (VS_FIXEDFILEINFO *)lpBuffer;
                    if (verInfo->dwSignature == 0xfeef04bd)
                    {
                        char *p = strdup(path);
                        char *pp = p;
                        char esc_chars[] = "\\/";
                        p[strlen(p) - strlen("\\Editor")] = 0;
                        if (num_found != 0)
                        {
                            printf(",\n");
                        }

                        printf ("\t\t{\"path\":\"");

                        // escape
                        while (*pp)
                        {
                            if (strchr (esc_chars, *pp))
                            {
                                putc('\\', stdout);
                            }
                            putc(*pp, stdout);
                            pp++;
                        }

                        printf ("\", \"version\":\"%d.%d.%d.%d\"}",
                            (verInfo->dwFileVersionMS >> 16) & 0xffff,
                            (verInfo->dwFileVersionMS >> 0) & 0xffff,
                            (verInfo->dwFileVersionLS >> 16) & 0xffff,
                            (verInfo->dwFileVersionLS >> 0) & 0xffff
                            );
                        free(p);
                        num_found++;
                    }
                }
            }
        }
        free (verData);
    }
    free(file_path);
}

static BOOL path_ends_with(const char *path, const char *value)
{
    const char *p = path + strlen(path) - strlen(value);
    return p >= path && !_stricmp(p, value);
}

static void find_unity_installs_at_path (const char *path, int level)
{
    WIN32_FIND_DATA fd;
    size_t l;
    char *search_path = NULL;
    HANDLE hSearch;
    direntry_t *list = NULL;

    if (path_ends_with (path, "\\Editor") && is_unity_editor_folder(path))
    {
        write_unity_info (path, "Unity.exe");

        return; // don't search for unity in Unity's subfolders
    }

    l = strlen (path) + 3;
    search_path = malloc (l);

    _snprintf (search_path, l, "%s\\*", path);

    ZeroMemory (&fd, sizeof (fd));
    
    hSearch = FindFirstFile (search_path, &fd);
    if (hSearch != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (strcmp (fd.cFileName, ".")
                && strcmp (fd.cFileName, "..")
                && (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                && !(fd.dwFileAttributes&(FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM|FILE_ATTRIBUTE_TEMPORARY)))
            {
                if (level >= 1 || _stricmp (fd.cFileName, "windows"))
                {
                    size_t l = strlen (fd.cFileName) + strlen (path) + 2;
                    char *file_path = malloc (l);
                    direntry_t *de = calloc (1, sizeof (direntry_t));

                    _snprintf (file_path, l, "%s\\%s", path, fd.cFileName);

                    de->path = file_path;
                    de->next = list;
                    list = de;
                }
            }
        } while (FindNextFile (hSearch, &fd));
    }
    FindClose (hSearch);

    free (search_path);

    while (list)
    {
        direntry_t *next = list->next;

        find_unity_installs_at_path (list->path, level+1);

        free (list->path);
        free (list);
        list = next;
    }
}

static void find_unity_installs ()
{
    DWORD drives;
    int i;

    printf("{\n\t\"unity_installs\":[\n");
    
    drives = GetLogicalDrives();
    for (i = 0; i < 32; i++)
    {
        if (drives&(1 << i))
        {
            UINT type;
            char drivename[3];
            drivename[0] = 'A' + i;
            drivename[1] = ':';
            drivename[2] = 0;
            type = GetDriveType(drivename);
            if (type == DRIVE_FIXED)
            {
                find_unity_installs_at_path(drivename, 0);
            }
        }
    }
    printf("\n\t]\n}\n");
}

int main (int argc, char *argv[])
{
    find_unity_installs ();
    return 0;
}
