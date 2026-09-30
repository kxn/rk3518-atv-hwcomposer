#include "utils/drmfence.h"
#include <dirent.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
using namespace android;
static int fd_count(){DIR* d=opendir("/proc/self/fd");assert(d);int n=0;while(readdir(d))++n;closedir(d);return n;}
int main(){
 int start=fd_count();
 {SyncTimeline t;t.iTimelineCnt_=UINT32_MAX-1;assert(t.IncTimeline()==UINT32_MAX);assert(t.IncTimeline()==0);}
 for(int i=0;i<500;++i){
  SyncTimeline t;assert(t.isValid());assert(fcntl(t.getFd(),F_GETFD)&FD_CLOEXEC);
  sp<ReleaseFence> f=new ReleaseFence(t,1);
  assert(f->isValid());assert(f->getName().find("allocReleaseFence")==0);
  assert(fcntl(f->getFd(),F_GETFD)&FD_CLOEXEC);
  assert(f->wait(0)<0);assert(errno==ETIME);
  assert(f->signal()==0);assert(f->wait(100)==0);
  int owned=f->getFd();f.clear();assert(fcntl(owned,F_GETFD)<0&&errno==EBADF);
 }
 {SyncTimeline t;t.destroy();sp<ReleaseFence> f=new ReleaseFence(t,1);assert(!f->isValid());}
 {SyncTimeline t;char name[128];memset(name,'a',127);name[127]=0;sp<ReleaseFence> f=new ReleaseFence(t,1,name);assert(f->isValid());assert(f->signal()==0);assert(f->wait(100)==0);}
 assert(fd_count()==start);
 printf("PASS: 500 real timelines/fences, null/long names, CLOEXEC, timeout/signal, invalid timeline, uint32 wrap, no FD leak\n");
 return 0;
}
