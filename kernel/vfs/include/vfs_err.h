#ifndef VFS_ERRNO_H
#define VFS_ERRNO_H

#ifdef __cplusplus
extern "C" {
#endif

#define    VFS_SUCCESS               0u
#define    E_VFS_K_ERR              -1
#define    E_VFS_NULL_PTR           -2  /* null ptr */
#define    E_VFS_ERR_PARAM          -3  /* null ptr */

#define    E_VFS_INODE_NO_AVAIL     -10 /* no inode alloc for dev register */
#define    E_VFS_INODE_NOT_FOUND    -11 /* can't find an inode */
#define    E_VFS_INODE_NOT_INIT     -12 /* inode is not allocated for use */
#define    E_VFS_INODE_TYPE_ILLEAGL -13 /* inode type illegal */

#define    E_VFS_FD_ILLEGAL         -31 /* fd is not in scope */

#define    E_VFS_NOSYS              -38 /* Function not implemented */

#define    E_VFS_BUSY               -41 /* device is opened */

#define    E_VFS_NO_MEM             -51 /* no mem */

#define    E_VFS_REGISTERED         -52 /* registered */

#define VFS_NULL_PARA_CHK(para)     do { if (!(para)) return E_VFS_NULL_PTR; } while(0)

#ifdef __cplusplus
}
#endif

#endif /* VFS_ERRNO_H */

