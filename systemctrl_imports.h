#ifndef SYSTEMCTRL_IMPORTS_H
#define	SYSTEMCTRL_IMPORTS_H

u32 sctrlHENFindFunction(char *modname, char *libname, u32 nid);
void sctrlHENPatchSyscall(void *addr, void *newaddr);

#endif /* SYSTEMCTRL_IMPORTS_H */