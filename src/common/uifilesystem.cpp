#include "uifilesystem.h"
#include <algorithm>
#ifdef _WIN32
# include <shlwapi.h>
# define tchar wchar_t
# define tstrcpy wcscpy
# define PATH_MAX MAX_PATH
#else
# include <sys/stat.h>
# include <fcntl.h>
# include <fts.h>
# include <dirent.h>
# define tchar char
# define tstrcpy strcpy
#endif


bool UIFileSystem::fileExists(const tstring &filePath)
{
#ifdef _WIN32
    DWORD attr = ::GetFileAttributes(filePath.c_str());
    return (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY));
#else
    struct stat st;
    return lstat(filePath.c_str(), &st) == 0 && S_ISREG(st.st_mode);
#endif
}

bool UIFileSystem::dirExists(const tstring &dirName)
{
#ifdef _WIN32
    DWORD attr = ::GetFileAttributes(dirName.c_str());
    return (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY));
#else
    struct stat st;
    return lstat(dirName.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
#endif
}

bool UIFileSystem::dirIsEmpty(const tstring &dirName)
{
#ifdef _WIN32
    return PathIsDirectoryEmpty(dirName.c_str());
#else
    DIR *dir = opendir(dirName.c_str());
    if (!dir)
        return (errno == ENOENT);

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            closedir(dir);
            return false;
        }
    }
    closedir(dir);
    return true;
#endif
}

// CRITICAL: An incorrect implementation or misuse of this function
// may lead to failed file operations, missing resources, or inconsistent filesystem state.
bool UIFileSystem::makePath(const tstring &path, size_t root_offset)
{
    size_t len = path.length();
    if (len == 0)
        return false;
#ifdef _WIN32
    if (CreateDirectoryW(path.c_str(), NULL) != 0 || GetLastError() == ERROR_ALREADY_EXISTS)
#else
    if (mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == 0 || errno == EEXIST)
#endif
        return true;
    if (len >= PATH_MAX || root_offset >= len)
        return false;

    tchar buf[PATH_MAX];
    tstrcpy(buf, path.c_str());
    if (buf[len - 1] == '/'
#ifdef _WIN32
            || buf[len - 1] == '\\'
#endif
        )
        buf[len - 1] = '\0';

    tchar *it = buf + root_offset;
    while (1) {
        while (*it != '\0' && *it != '/'
#ifdef _WIN32
                   && *it != '\\'
#endif
              )
            it++;
        tchar tmp = *it;
        *it = '\0';
#ifdef _WIN32
        if (CreateDirectoryW(buf, NULL) == 0 && GetLastError() != ERROR_ALREADY_EXISTS) {
#else
        if (mkdir(buf, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0 && errno != EEXIST) {
#endif
            *it = tmp;
            return false;
        }
        if (tmp == '\0')
            break;
        *it++ = tmp;
    }
    return true;
}

// CRITICAL: An incorrect implementation or misuse of this function
// may lead to failed file operations, missing resources, or inconsistent filesystem state.
bool UIFileSystem::copyFile(const tstring &oldFile, const tstring &newFile)
{
#ifdef _WIN32
    return CopyFile(oldFile.c_str(), newFile.c_str(), FALSE);
#else
    struct stat st;
    if (lstat(oldFile.c_str(), &st) != 0)
        return false;

    char buf[BUFSIZ];
    int fd_src = -1, fd_dst = -1, n_read = 0;
    if ((fd_src = open(oldFile.c_str(), O_RDONLY)) < 0)
        return false;

    if ((fd_dst = creat(newFile.c_str(), 0666)) < 0) {
        close(fd_src);
        return false;
    }
    if (fchmod(fd_dst, st.st_mode) != 0) {
        close(fd_src);
        close(fd_dst);
        return false;
    }

    while ((n_read = read(fd_src, buf, sizeof(buf))) > 0) {
        if (write(fd_dst, buf, n_read) != n_read) {
            close(fd_src);
            close(fd_dst);
            return false;
        }
    }
    close(fd_src);
    if (close(fd_dst) != 0)
        return false;

    return n_read == 0;
#endif
}

// CRITICAL: An incorrect implementation or misuse of this function
// may lead to failed file operations, missing resources, or inconsistent filesystem state.
bool UIFileSystem::replaceFile(const tstring &oldFilePath, const tstring &newFilePath)
{
#ifdef _WIN32
    return MoveFileExW(oldFilePath.c_str(), newFilePath.c_str(), MOVEFILE_REPLACE_EXISTING |
                           MOVEFILE_WRITE_THROUGH | MOVEFILE_COPY_ALLOWED) != 0;
#else
    if (rename(oldFilePath.c_str(), newFilePath.c_str()) == 0)
        return true;
    if (errno == EXDEV) {
        errno = 0;
        return copyFile(oldFilePath, newFilePath) && unlink(oldFilePath.c_str()) == 0;
    }
    return false;
#endif
}

// CRITICAL: An incorrect implementation or misuse of this function
// may lead to failed file operations, missing resources, or inconsistent filesystem state.
bool UIFileSystem::replaceDir(const tstring &sourceDir, const tstring &destDir)
{
#ifdef _WIN32
    tstring pFrom = toNativeSeparators(sourceDir);
    if (pFrom.back() == _T('\\'))
        pFrom.pop_back();

    pFrom.push_back('\0');

    tstring pTo = toNativeSeparators(destDir);
    if (pTo.back() == _T('\\'))
        pTo.pop_back();

    pTo.push_back('\0');

    SHFILEOPSTRUCT fileOp = {0};
    fileOp.wFunc  = FO_MOVE;
    fileOp.pFrom  = pFrom.c_str();
    fileOp.pTo    = pTo.c_str();
    fileOp.fFlags = FOF_NOCONFIRMATION |
                    FOF_SILENT |
                    FOF_NOERRORUI |
                    FOF_NOCONFIRMMKDIR;

    return SHFileOperation(&fileOp) == 0;
#else
    if (rename(sourceDir.c_str(), destDir.c_str()) == 0)
        return true;

    if (errno == EXDEV || errno == EEXIST || errno == ENOTEMPTY) {
        size_t srcLen = sourceDir.length(); // rename() will return a higher priority errno ENOENT
        size_t dstLen = destDir.length();   // if length == 0 or ENAMETOOLONG if lenght >= PATH_MAX

        bool can_use_rename = !(errno == EXDEV); // EXDEV has higher priority than EEXIST and ENOTEMPTY
        errno = 0;

        char dstPath[PATH_MAX];
        snprintf(dstPath, sizeof(dstPath), "%s", destDir.c_str());

        tstring src(sourceDir);
        char * const paths[] = {&src[0], NULL};
        FTS *fts = fts_open(paths, FTS_PHYSICAL | FTS_XDEV /*| FTS_NOSTAT*/, NULL);
        if (fts == NULL)
            return false;

        bool res = true;
        FTSENT *ent;
        while (res && (ent = fts_read(fts)) != NULL) {
            switch (ent->fts_info) {
            case FTS_DOT:       // "." or ".."
                break;
            case FTS_D:         // preorder directory
                if (strlcat(dstPath, ent->fts_path + srcLen, PATH_MAX) >= PATH_MAX) {
                    errno = ENAMETOOLONG;
                    res = false;
                    break;
                }
                if (can_use_rename && ent->fts_level != 0 && rename(ent->fts_path, dstPath) == 0) {
                    dstPath[dstLen] = '\0';
                    fts_set(fts, ent, FTS_SKIP);
                    // Ensure that we do not process FTS_DP
                    (void)(fts_read(fts));
                    break;
                }

                if (mkdir(dstPath, ent->fts_statp->st_mode) != 0 && errno != EEXIST)
                    res = false;
                dstPath[dstLen] = '\0';
                break;
            case FTS_DP:		// postorder directory
                if (rmdir(ent->fts_path) != 0)
                    res = false;
                break;
            case FTS_F:			// regular file
            case FTS_SL:		// symbolic link
            case FTS_SLNONE:	// symbolic link without target
            case FTS_DEFAULT:   // file type not described by any other value
                if (strlcat(dstPath, ent->fts_path + srcLen, PATH_MAX) >= PATH_MAX) {
                    errno = ENAMETOOLONG;
                    res = false;
                    break;
                }
                if (can_use_rename) {
                    if (rename(ent->fts_path, dstPath) != 0)
                        res = false;
                } else
                    if (!copyFile(ent->fts_path, dstPath) || unlink(ent->fts_path) != 0)
                        res = false;
                dstPath[dstLen] = '\0';
                break;
            case FTS_NSOK:      // stat(2) information was not requested
            case FTS_NS:		// stat(2) information was not available
            case FTS_DC:		// directory that causes cycles
            case FTS_DNR:		// unreadable directory
            case FTS_ERR:
            default:
                res = false;
                break;
            }
        }

        if (fts_close(fts) != 0)
            return false;

        return res;
    }

    return false;
#endif
}

bool UIFileSystem::removeFile(const tstring &filePath)
{
#ifdef _WIN32
    return DeleteFile(filePath.c_str()) != 0;
#else
    return unlink(filePath.c_str()) == 0;
#endif
}

// CRITICAL: An incorrect implementation or misuse of this function
// may lead to failed file operations, missing resources, or inconsistent filesystem state.
bool UIFileSystem::removeDirRecursively(const tstring &dir)
{
#ifdef _WIN32
    tstring pFrom = toNativeSeparators(dir);
    if (pFrom.back() == _T('\\'))
        pFrom.pop_back();

    pFrom.push_back('\0');

    SHFILEOPSTRUCT fileOp = {0};
    fileOp.pFrom  = pFrom.c_str();
    fileOp.wFunc  = FO_DELETE;
    fileOp.fFlags = FOF_NOCONFIRMATION |
                    FOF_SILENT |
                    FOF_NOERRORUI |
                    FOF_NOCONFIRMMKDIR;

    return SHFileOperation(&fileOp) == 0;
#else
    tstring _dir = dir;
    char * const paths[] = {&_dir[0], NULL};
    FTS *fts = fts_open(paths, FTS_PHYSICAL | FTS_XDEV /*| FTS_NOSTAT*/, NULL);
    if (fts == NULL)
        return false;

    bool res = true;
    FTSENT *ent;
    while (res && (ent = fts_read(fts)) != NULL) {
        switch (ent->fts_info) {
        case FTS_DOT:       // "." or ".."
            break;
        case FTS_D:         // preorder directory
            break;
        case FTS_DP:		// postorder directory
            if (rmdir(ent->fts_path) != 0)
                res = false;
            break;
        case FTS_F:			// regular file
        case FTS_SL:		// symbolic link
        case FTS_SLNONE:	// symbolic link without target
        case FTS_DEFAULT:   // file type not described by any other value
            if (unlink(ent->fts_path) != 0)
                res = false;
            break;
        case FTS_NSOK:      // stat(2) information was not requested
        case FTS_NS:		// stat(2) information was not available
        case FTS_DC:		// directory that causes cycles
        case FTS_DNR:		// unreadable directory
        case FTS_ERR:
        default:
            res = false;
            break;
        }
    }

    if (fts_close(fts) != 0)
        return false;

    return res;
#endif
}

tstring UIFileSystem::fromNativeSeparators(const tstring &path)
{
#ifdef _WIN32
    tstring _path(path);
    std::replace(_path.begin(), _path.end(), L'\\', L'/');
    return _path;
#else
    return path;
#endif
}

tstring UIFileSystem::toNativeSeparators(const tstring &path)
{
    tstring _path(path);
#ifdef _WIN32
    std::replace(_path.begin(), _path.end(), L'/', L'\\');
#else
    std::replace(_path.begin(), _path.end(), '\\', '/');
#endif
    return _path;
}

// CRITICAL: An incorrect implementation or misuse of this function
// may lead to failed file operations, missing resources, or inconsistent filesystem state.
tstring UIFileSystem::parentPath(const tstring &path)
{
    size_t len = path.length();
    if (len > 1) {
        const tchar *buf = path.c_str();
        const tchar *it = buf + len - 1;
        while (*it == '/'
#ifdef _WIN32
                || *it == '\\'
#endif
               ) {
            if (it == buf)
                return {};
            it--;
        }
        while (*it != '/'
#ifdef _WIN32
               && *it != '\\'
#endif
               ) {
            if (it == buf)
                return {};
            it--;
        }
        if (it == buf)
#ifdef _WIN32
            return L"";
#else
            return "/";
#endif
        return tstring(buf, it - buf);
    }
    return {};
}

tstring UIFileSystem::tempPath()
{
#ifdef _WIN32
    WCHAR buff[MAX_PATH + 2] = {0};
    DWORD res = ::GetTempPath(MAX_PATH + 1, buff);
    if (res != 0 && res <= MAX_PATH + 1) {
        buff[res - 1] = '\0';
        return fromNativeSeparators(buff);
    }
    return {};
#else
    const char *path = getenv("TMP");
    if (!path)
        path = getenv("TEMP");
    if (!path)
        path = getenv("TMPDIR");
    if (!path)
        path = "/tmp";
    return tstring(path);
#endif
}

tstring UIFileSystem::appPath()
{
#ifdef _WIN32
    WCHAR buff[MAX_PATH];
    DWORD res = ::GetModuleFileName(NULL, buff, MAX_PATH);
    return (res != 0) ? fromNativeSeparators(parentPath(buff)) : L"";
#else
    char path[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", path, PATH_MAX);
    return (count > 0) ? parentPath(tstring(path, count)) : "";
#endif
}
