// vim: et ts=4 sw=4 tw=0
#import <Foundation/Foundation.h>
#include <sys/stat.h>

static const char *unity_bundle_ids[] = {
    "com.unity3d.UnityEditor5.x",
    "com.unity3d.UnityEditor4.x",
    "com.unity3d.UnityEditor3.x",
    NULL
};

static NSString *get_bundle_version (const char *bundle_path) {
    NSString *plistPath = [[NSString stringWithUTF8String:bundle_path] stringByAppendingString:@"/Contents/Info.plist"];
    struct stat st;
    memset (&st, 0, sizeof (st));
    int err = stat ([plistPath UTF8String], &st);
    if (0 > err || !S_ISREG(st.st_mode)) {
        return nil;
    }

    NSDictionary *plist = [NSDictionary dictionaryWithContentsOfFile:plistPath];
    if (!plist) {
        return nil;
    }

    NSString *version = plist[@"CFBundleVersion"];
    return version;
}

static BOOL is_bundle_a_unity_installation (const char *bundle_path) {
    const char *folders[] = {
        "PlaybackEngines",
        "Unity Bug Reporter.app",
        "MonoDevelop.app",
        NULL
    };

    for (int i = 0; folders[i]; i++) {
        char path[PATH_MAX];
        snprintf (path, sizeof (path), "%s/../%s", bundle_path, folders[i]);
        struct stat st;
        memset (&st, 0, sizeof (st));
        int res = stat (path, &st);
        if (!res && S_ISDIR (st.st_mode)) {
            return YES;
        }
    }
    return NO;
}

static void find_unity_bundles_for_id (const char *bundle_id, NSMutableArray *installs) {
    char cmd[PATH_MAX];
    snprintf (cmd, sizeof (cmd), "mdfind \"kMDItemCFBundleIdentifier == '%s'\"", bundle_id);
    FILE *fp = popen(cmd, "r");
    if (!fp) {
        return;
    }
    else {
        char buffer[1000];
        while (fgets(buffer, sizeof(buffer), fp)) {
            // trim whitespace
            char *p = buffer + strlen (buffer) - 1;
            while (p >= buffer) {
                if (*p != '\n' && *p != '\r') {
                    break;
                }
                *p-- = 0;
            }
            NSString *version = get_bundle_version (buffer);
            if (!version) {
                continue;
            }
            NSDictionary *install = [NSDictionary dictionaryWithObjectsAndKeys:[NSString stringWithUTF8String: buffer], @"path", version, @"version", nil];
            [installs addObject:install];
        }
        fclose(fp);
    }
}

static void find_unity_bundles (void) {
    NSMutableArray *installs = [[NSMutableArray alloc] init];

    for (int i = 0; unity_bundle_ids[i]; i++) {
        find_unity_bundles_for_id (unity_bundle_ids[i], installs);
    }
    NSDictionary *dict = [NSDictionary dictionaryWithObjectsAndKeys:installs, @"unity_installs", nil];

    NSError *err = nil;
    NSData *dt = [NSJSONSerialization dataWithJSONObject:dict options:0 error:&err];

    NSString *json = [[NSString alloc] initWithData:dt encoding:NSUTF8StringEncoding];
    printf ("%s\n", [json UTF8String]);
}

int main(int argc, const char * argv[]) {
    @autoreleasepool {
        find_unity_bundles ();
    }
    return 0;
}
