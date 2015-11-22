/*
 *  Minimalistic (auto) mount daemon
 *  Copyright (C) 2015  Ernest Vogelsang <stagprom@posteo.de>
 * 
 *  This file is part of mmountd.
 * 
 *  mmountd is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  mmountd is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 * 
 *  You should have received a copy of the GNU General Public License
 *  along with mmountd.  If not, see <http://www.gnu.org/licenses/>.
 * 
 *  Original provided by fon.com
 *  Copyright (C) 2009 John Crispin <blogic@openwrt.org> 
 *
 */
 
 
#include <sys/types.h>
#include <linux/types.h>
#include <paths.h>
#include <limits.h>
#include <time.h>

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <fcntl.h>

#include <errno.h>

#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <poll.h>
#include <linux/auto_fs4.h>
#include <pthread.h>

#include "include/log.h"
#include "include/sys.h"
#include "include/autofs.h"
#include "include/signal.h"


int autofs_exit	= 0;
int mounted	= 0;

int fdin 	= 0; /* data coming out of the kernel */
int fdout 	= 0;/* data going into the kernel */
dev_t dev;

pthread_t thread;
pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;
char mount_cmd[256];
char mount_helper_cmd[256];
char umount_cmd[256];
char umount_helper_cmd[256];

static void send_ready(unsigned int wait_queue_token)
{
	if(ioctl(fdin, AUTOFS_IOC_READY, wait_queue_token) < 0)
		log_printf("failed to report ready to kernel\n");
}

static void send_fail(unsigned int wait_queue_token)
{
	if(ioctl(fdin, AUTOFS_IOC_FAIL, wait_queue_token) < 0)
		log_printf("failed to report fail to kernel\n");
}

static int autofs_process_missing( const struct autofs_v5_packet *pkt ) {
	int err;
	
	log_printf( "kernel is requesting a mount -> %s\n", pkt->name );
	err = system(mount_cmd);
	if(!err) {
		mounted = 1;
		send_ready(pkt->wait_queue_token);
		if(helper) {
			system(mount_helper_cmd);
		}
		pthread_mutex_unlock( &mutex );		
	} else {
		send_fail(pkt->wait_queue_token);
		log_printf("mount error-> %d %s\n", err, strerror(errno));
	}		
	return err;
}

static int autofs_process_expire(const struct autofs_v5_packet *pkt) {
	int err;
	
	log_printf("kernel is requesting a umount -> %s\n", pkt->name);
	err = system(umount_cmd);
	if(!err) {
		mounted = 0;
		send_ready(pkt->wait_queue_token);
		if(helper) {
			system(umount_helper_cmd);
		}
		pthread_mutex_lock( &mutex );		
	} else {
		send_fail(pkt->wait_queue_token);
		log_printf("umount -> %d %s\n", err, strerror(errno));
	}
	return err;
}

static void *autofs_expire_thread( void *arg ) {
	int err;
	
	while(1) {
/*		log_printf( "AUTOFS_IOC_EXPIRE_MULTI: -> " ); */
		pthread_mutex_lock( &mutex );		
		pthread_mutex_unlock( &mutex );		
		err = ioctl( fdin, AUTOFS_IOC_EXPIRE_MULTI, NULL );
/*		log_printf(" %d %s\n",err ,strerror( errno )); */
		sleep( 1 );
	}
}


static void umount_autofs(void) {
	int err;
	
	err = system_printf("umount %s", mountp );
	if(err) log_printf("umount_autofs %d %s\n",err, strerror(errno));
}

static int mount_autofs(void) {
	int err;
	int pipefd[2];
	struct stat st;
	
	fdout = fdin = -1;
	if(pipe( pipefd ) < 0 ) {
		log_printf("failed to get kernel pipe\n");
		return -1;
	}
	err = system_printf("/bin/mount -t autofs -o fd=%d,pgrp=%u,minproto=5,maxproto=5,direct \"mountd(pid%u)\" %s", pipefd[1], (unsigned) getpgrp(), getpid(), mountp);
	if(err) {
		log_printf("unable to mount autofs on %s %d %s\n", mountp, err, strerror(errno));
		close(pipefd[0]);
		close(pipefd[1]);
		return -1;
	}
	log_printf("mounted %s as the autofs root with device: %s\n", mountp, device );
	close(pipefd[1]);
	fdout = pipefd[0];
	fdin = open(mountp, O_RDONLY);
	if( fdin < 0 ) {
		umount_autofs();
		return -1;
	}
	stat( mountp, &st);
	return 0;
}

static void autofs_cleanup(void) {
	int err;

	err = ioctl(fdin, AUTOFS_IOC_CATATONIC, NULL);
	log_printf("cleaning up ... AUTOFS_IOC_CATATONIC: %d %s\n",err, strerror(errno));
	close(fdin);
	close(fdout);
	umount_autofs();
	rmdir(mountp);
}

static void autofs_sig_handler(int sig) {
	int err;
	
	if( mounted ) {
		log_printf("caught sig %s mount busy ... delaying\n", strsignal(sig));
		autofs_exit = 1;
	} else {
		log_printf("caught sig %s\n", strsignal(sig));
		autofs_cleanup();
		exit(0);
	}
}	

void autofs_loop(void)
{
	ssize_t ret;
	int 	err, res;
	struct pollfd fds[1];
	union autofs_v5_packet_union pkt;

	fds[0].fd = fdout;
	fds[0].events = POLLIN;
	while(1)
	{
		res = poll(fds, 1, -1);
		if( res > 0 ) {
			err = read(fdout, &pkt, sizeof(pkt));
		} else {
			if( !res ) {
				log_printf("poll packet from kernel Timeout %d\n", res);
				continue;
			}
			else {
				if (errno == EINTR)
					continue;
				log_printf("failed while trying to read packet from kernel %d %s\n", res, strerror(errno));
				continue;
			}
		}				
		log_printf("Got a autofs packet %d\n", err);
		switch(pkt.hdr.type) {
			case autofs_ptype_expire_multi:
				log_printf("packet type %d expire multi\n", pkt.hdr.type);
				break;
			case autofs_ptype_missing_indirect: 
				log_printf("packet type %d missing indirect\n", pkt.hdr.type);
				break;
			case autofs_ptype_expire_indirect:
				log_printf("packet type %d expire indirect\n", pkt.hdr.type);
				break;
			case autofs_ptype_missing_direct:
				log_printf("packet type %d missing direct\n", pkt.hdr.type);
				autofs_process_missing(&pkt.missing_direct);
				break;
			case autofs_ptype_expire_direct:
				log_printf("packet type %d expire direct\n", pkt.hdr.type);
				autofs_process_expire(&pkt.expire_direct);
				break;
			default:
				log_printf("unknown packet type %d\n", pkt.hdr.type);
		}
		if(autofs_exit) break;
	}
	autofs_cleanup();
}

void autofs_init(void)
{
	int kproto_version;
	int err;
	
	signal_init(&autofs_sig_handler);
	err=mkdir(mountp,0777);
	if(err < 0)
	{
		log_printf("Cant mkdir %s err: %d %s\n", mountp, err, strerror(errno));
		closelog();
		exit(1);
	}
	if(mount_autofs() < 0)
	{
		closelog();
		exit(1);
	}
	ioctl(fdin, AUTOFS_IOC_PROTOVER, &kproto_version);
	if(kproto_version != 5)
	{
		log_printf("only kernel protocol version 5 is tested. You have %d.\n",
			kproto_version);
		closelog();
		exit(1);
	}
	snprintf(mount_cmd, 256 , "mount %s %s %s", mountopt, device, mountp);
	snprintf(umount_cmd, 256 , "umount %s", mountp);
	if(helper) {
		snprintf(mount_helper_cmd, 256 , "%s mount", helper);
		snprintf(umount_helper_cmd, 256 , "%s umount", helper);
	}
	err = ioctl(fdin, AUTOFS_IOC_SETTIMEOUT, &timeout);
	log_printf("AUTOFS_IOC_SETTIMEOUT: %d\n",err);
	pthread_mutex_lock( &mutex );
	err = pthread_create( &thread, NULL, &autofs_expire_thread, NULL);
	log_printf("pthread create: %d\n",err);
}
