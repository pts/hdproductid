/* by pyts@fazekas.hu at Sun Jan 14 14:58:25 CET 2018
 *
 * $ xstatic gcc -s -O2 -W -Wall -Wextra -Werror -o hdid hdid.c
 */
#define _LARGEFILE64_SOURCE 1
#define _FILE_OFFSET_BITS 64
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>

#ifndef __linux__
#error This C program needs Linux.
#endif

#ifndef HDIO_GET_IDENTITY  /* <linux/hdreg.h> */
#define HDIO_GET_IDENTITY    0x030d  /* get IDE identification info */
#endif

/*
 * Structure returned by HDIO_GET_IDENTITY, as per ANSI NCITS ATA6 rev.1b spec.
 *
 * If you change something here, please remember to update fix_driveid() in
 * ide/probe.c.
 */
struct my_hd_driveid {  /* <linux/hdreg.h> defines struct hd_driveid. */
  unsigned short	config;		/* lots of obsolete bit flags */
  unsigned short	cyls;		/* Obsolete, "physical" cyls */
  unsigned short	reserved2;	/* reserved (word 2) */
  unsigned short	heads;		/* Obsolete, "physical" heads */
  unsigned short	track_bytes;	/* unformatted bytes per track */
  unsigned short	sector_bytes;	/* unformatted bytes per sector */
  unsigned short	sectors;	/* Obsolete, "physical" sectors per track */
  unsigned short	vendor0;	/* vendor unique */
  unsigned short	vendor1;	/* vendor unique */
  unsigned short	vendor2;	/* Retired vendor unique */
  unsigned char	serial_no[20];	/* 0 = not_specified */
  unsigned short	buf_type;	/* Retired */
  unsigned short	buf_size;	/* Retired, 512 byte increments
  				 * 0 = not_specified
  				 */
  unsigned short	ecc_bytes;	/* for r/w long cmds; 0 = not_specified */
  unsigned char	fw_rev[8];	/* 0 = not_specified */
  unsigned char	model[40];	/* 0 = not_specified */
  unsigned char	max_multsect;	/* 0=not_implemented */
  unsigned char	vendor3;	/* vendor unique */
  unsigned short	dword_io;	/* 0=not_implemented; 1=implemented */
  unsigned char	vendor4;	/* vendor unique */
  unsigned char	capability;	/* (upper byte of word 49)
  				 *  3:	IORDYsup
  				 *  2:	IORDYsw
  				 *  1:	LBA
  				 *  0:	DMA
  				 */
  unsigned short	reserved50;	/* reserved (word 50) */
  unsigned char	vendor5;	/* Obsolete, vendor unique */
  unsigned char	tPIO;		/* Obsolete, 0=slow, 1=medium, 2=fast */
  unsigned char	vendor6;	/* Obsolete, vendor unique */
  unsigned char	tDMA;		/* Obsolete, 0=slow, 1=medium, 2=fast */
  unsigned short	field_valid;	/* (word 53)
  				 *  2:	ultra_ok	word  88
  				 *  1:	eide_ok		words 64-70
  				 *  0:	cur_ok		words 54-58
  				 */
  unsigned short	cur_cyls;	/* Obsolete, logical cylinders */
  unsigned short	cur_heads;	/* Obsolete, l heads */
  unsigned short	cur_sectors;	/* Obsolete, l sectors per track */
  unsigned short	cur_capacity0;	/* Obsolete, l total sectors on drive */
  unsigned short	cur_capacity1;	/* Obsolete, (2 words, misaligned int)     */
  unsigned char	multsect;	/* current multiple sector count */
  unsigned char	multsect_valid;	/* when (bit0==1) multsect is ok */
  unsigned int	lba_capacity;	/* Obsolete, total number of sectors */
  unsigned short	dma_1word;	/* Obsolete, single-word dma info */
  unsigned short	dma_mword;	/* multiple-word dma info */
  unsigned short  eide_pio_modes; /* bits 0:mode3 1:mode4 */
  unsigned short  eide_dma_min;	/* min mword dma cycle time (ns) */
  unsigned short  eide_dma_time;	/* recommended mword dma cycle time (ns) */
  unsigned short  eide_pio;       /* min cycle time (ns), no IORDY  */
  unsigned short  eide_pio_iordy; /* min cycle time (ns), with IORDY */
  unsigned short	words69_70[2];	/* reserved words 69-70
  				 * future command overlap and queuing
  				 */
  unsigned short	words71_74[4];	/* reserved words 71-74
  				 * for IDENTIFY PACKET DEVICE command
  				 */
  unsigned short  queue_depth;	/* (word 75)
  				 * 15:5	reserved
  				 *  4:0	Maximum queue depth -1
  				 */
  unsigned short  words76_79[4];	/* reserved words 76-79 */
  unsigned short  major_rev_num;	/* (word 80) */
  unsigned short  minor_rev_num;	/* (word 81) */
  unsigned short  command_set_1;	/* (word 82) supported
  				 * 15:	Obsolete
  				 * 14:	NOP command
  				 * 13:	READ_BUFFER
  				 * 12:	WRITE_BUFFER
  				 * 11:	Obsolete
  				 * 10:	Host Protected Area
  				 *  9:	DEVICE Reset
  				 *  8:	SERVICE Interrupt
  				 *  7:	Release Interrupt
  				 *  6:	look-ahead
  				 *  5:	write cache
  				 *  4:	PACKET Command
  				 *  3:	Power Management Feature Set
  				 *  2:	Removable Feature Set
  				 *  1:	Security Feature Set
  				 *  0:	SMART Feature Set
  				 */
  unsigned short  command_set_2;	/* (word 83)
  				 * 15:	Shall be ZERO
  				 * 14:	Shall be ONE
  				 * 13:	FLUSH CACHE EXT
  				 * 12:	FLUSH CACHE
  				 * 11:	Device Configuration Overlay
  				 * 10:	48-bit Address Feature Set
  				 *  9:	Automatic Acoustic Management
  				 *  8:	SET MAX security
  				 *  7:	reserved 1407DT PARTIES
  				 *  6:	SetF sub-command Power-Up
  				 *  5:	Power-Up in Standby Feature Set
  				 *  4:	Removable Media Notification
  				 *  3:	APM Feature Set
  				 *  2:	CFA Feature Set
  				 *  1:	READ/WRITE DMA QUEUED
  				 *  0:	Download MicroCode
  				 */
  unsigned short  cfsse;		/* (word 84)
  				 * cmd set-feature supported extensions
  				 * 15:	Shall be ZERO
  				 * 14:	Shall be ONE
  				 * 13:6	reserved
  				 *  5:	General Purpose Logging
  				 *  4:	Streaming Feature Set
  				 *  3:	Media Card Pass Through
  				 *  2:	Media Serial Number Valid
  				 *  1:	SMART selt-test supported
  				 *  0:	SMART error logging
  				 */
  unsigned short  cfs_enable_1;	/* (word 85)
  				 * command set-feature enabled
  				 * 15:	Obsolete
  				 * 14:	NOP command
  				 * 13:	READ_BUFFER
  				 * 12:	WRITE_BUFFER
  				 * 11:	Obsolete
  				 * 10:	Host Protected Area
  				 *  9:	DEVICE Reset
  				 *  8:	SERVICE Interrupt
  				 *  7:	Release Interrupt
  				 *  6:	look-ahead
  				 *  5:	write cache
  				 *  4:	PACKET Command
  				 *  3:	Power Management Feature Set
  				 *  2:	Removable Feature Set
  				 *  1:	Security Feature Set
  				 *  0:	SMART Feature Set
  				 */
  unsigned short  cfs_enable_2;	/* (word 86)
  				 * command set-feature enabled
  				 * 15:	Shall be ZERO
  				 * 14:	Shall be ONE
  				 * 13:	FLUSH CACHE EXT
  				 * 12:	FLUSH CACHE
  				 * 11:	Device Configuration Overlay
  				 * 10:	48-bit Address Feature Set
  				 *  9:	Automatic Acoustic Management
  				 *  8:	SET MAX security
  				 *  7:	reserved 1407DT PARTIES
  				 *  6:	SetF sub-command Power-Up
  				 *  5:	Power-Up in Standby Feature Set
  				 *  4:	Removable Media Notification
  				 *  3:	APM Feature Set
  				 *  2:	CFA Feature Set
  				 *  1:	READ/WRITE DMA QUEUED
  				 *  0:	Download MicroCode
  				 */
  unsigned short  csf_default;	/* (word 87)
  				 * command set-feature default
  				 * 15:	Shall be ZERO
  				 * 14:	Shall be ONE
  				 * 13:6	reserved
  				 *  5:	General Purpose Logging enabled
  				 *  4:	Valid CONFIGURE STREAM executed
  				 *  3:	Media Card Pass Through enabled
  				 *  2:	Media Serial Number Valid
  				 *  1:	SMART selt-test supported
  				 *  0:	SMART error logging
  				 */
  unsigned short  dma_ultra;	/* (word 88) */
  unsigned short	trseuc;		/* time required for security erase */
  unsigned short	trsEuc;		/* time required for enhanced erase */
  unsigned short	CurAPMvalues;	/* current APM values */
  unsigned short	mprc;		/* master password revision code */
  unsigned short	hw_config;	/* hardware config (word 93)
  				 * 15:	Shall be ZERO
  				 * 14:	Shall be ONE
  				 * 13:
  				 * 12:
  				 * 11:
  				 * 10:
  				 *  9:
  				 *  8:
  				 *  7:
  				 *  6:
  				 *  5:
  				 *  4:
  				 *  3:
  				 *  2:
  				 *  1:
  				 *  0:	Shall be ONE
  				 */
  unsigned short	acoustic;	/* (word 94)
  				 * 15:8	Vendor's recommended value
  				 *  7:0	current value
  				 */
  unsigned short	msrqs;		/* min stream request size */
  unsigned short	sxfert;		/* stream transfer time */
  unsigned short	sal;		/* stream access latency */
  unsigned int	spg;		/* stream performance granularity */
  unsigned long long lba_capacity_2;/* 48-bit total number of sectors */
  unsigned short	words104_125[22];/* reserved words 104-125 */
  unsigned short	last_lun;	/* (word 126) */
  unsigned short	word127;	/* (word 127) Feature Set
  				 * Removable Media Notification
  				 * 15:2	reserved
  				 *  1:0	00 = not supported
  				 *	01 = supported
  				 *	10 = reserved
  				 *	11 = reserved
  				 */
  unsigned short	dlf;		/* (word 128)
  				 * device lock function
  				 * 15:9	reserved
  				 *  8	security level 1:max 0:high
  				 *  7:6	reserved
  				 *  5	enhanced erase
  				 *  4	expire
  				 *  3	frozen
  				 *  2	locked
  				 *  1	en/disabled
  				 *  0	capability
  				 */
  unsigned short  csfo;		/*  (word 129)
  				 * current set features options
  				 * 15:4	reserved
  				 *  3:	auto reassign
  				 *  2:	reverting
  				 *  1:	read-look-ahead
  				 *  0:	write cache
  				 */
  unsigned short	words130_155[26];/* reserved vendor words 130-155 */
  unsigned short	word156;	/* reserved vendor word 156 */
  unsigned short	words157_159[3];/* reserved vendor words 157-159 */
  unsigned short	cfa_power;	/* (word 160) CFA Power Mode
  				 * 15 word 160 supported
  				 * 14 reserved
  				 * 13
  				 * 12
  				 * 11:0
  				 */
  unsigned short	words161_175[15];/* Reserved for CFA */
  unsigned short	words176_205[30];/* Current Media Serial Number */
  unsigned short	words206_254[49];/* reserved words 206-254 */
  unsigned short	integrity_word;	/* (word 255)
  				 * 15:8 Checksum
  				 *  7:0 Signature
  				 */
};

static void *my_memmem(const void* haystack, size_t hl, const void* needle, size_t nl) {
  const void *haystack2;
  size_t i;
  if (nl > hl) return 0;
  if (nl == 0) return (void*)haystack;
  for (i = hl - nl + 1; i > 0; ) {
    if (!(haystack2 = memchr(haystack, *(char*)needle, hl - nl + 1))) return 0;
    if (!memcmp(haystack2,needle,nl)) return (void*)haystack2;
    i -= ++haystack2 - haystack;
    haystack = haystack2;
  }
  return 0;
}

static int read_first_line(const char *filename, char *buf, size_t bufsize) {
  char *p, *pend;
  int fd = open(filename, O_RDONLY, 0);
  size_t got;
  if (fd < 1) {
    fprintf(stderr, "error: open: %s: %s\n", filename, strerror(errno));
    return -1;
  }
  if ((got = read(fd, buf, bufsize)) + 1U == 0U) {
    close(fd);
    fprintf(stderr, "error: read: %s: %s\n", filename, strerror(errno));
    return -1;
  }
  close(fd);
  for (p = buf, pend = buf + bufsize;
       p != pend && *p != '\n' && *p != '\0'; ++p) {}
  if (p == pend || *p == '\0') {
    fprintf(stderr, "error: Bad line in: %s\n", filename);
    return -1;
  }
  *p = '\0';
  return 0;
}

/* Strips ' ' and '\t' in place, changes a single byte in p[...] to '\0'. */
static char *strip(char *p) {
  char *q;
  char c;
  for (; (c = *p) == ' ' || c == '\t'; ++p) {}
  for (q = p + strlen(p); q != p && ((c = q[-1]) == ' ' || c == '\t'); --q) {}
  *q = '\0';
  return p;
}

static int hdid_usb(const char *devname, unsigned rdev) {
  char fnbuf1[128], fnbuf2[512], *p, *q;
  int got;
  /* A normal (non-root) user can do these below. */
  sprintf(fnbuf1, "/sys/dev/block/%d:%d", rdev >> 8, (unsigned char)rdev);
  got = readlink(fnbuf1, fnbuf2, sizeof(fnbuf2) - 1);
  if ((unsigned)got - 1 > sizeof(fnbuf2) - 2) {
    fprintf(stderr, "error: readlink for %s: %s: %s\n", devname, fnbuf1, strerror(errno));
    return -1;
  }
  fnbuf2[got] = '\0';
  /* Now fnbuf2 looks like this: "../../devices/pci0000:00/0000:00:1a.7/usb1/1-1/1-1:1.0/host7/target7:0:0/7:0:0:0/block/sdc".
   * We want to extract the "1-1" from it.
   */
  if (!(p = my_memmem(fnbuf2, got, "/usb", 4)) || p[4] - '0' + 0U > 9U) { not_usb:
    fprintf(stderr, "error: Product info not found for device: %s\n", devname);
    return -1;
  }
  for (p += 5; *p - '0' + 0U <= 9U; ++p) {}
  if (*p++ != '/' || p - fnbuf2 < 21) goto not_usb;
  for (q = p; *q != '\0' && *q != '/'; ++q) {}
  if (strlen(q) < 14) goto not_usb;
  ++q;
  memcpy(p -= 21, "/sys/bus/usb/devices/", 21);
  /* Now p looks like: /sys/bus/usb/devices/1-1/...". */
  strcpy(q, "manufacturer");
  if (0 == read_first_line(p, fnbuf1, sizeof(fnbuf1))) {
    printf("manufacturer: (%s)\n", strip(fnbuf1));
  }
  strcpy(q, "product");
  if (0 == read_first_line(p, fnbuf1, sizeof(fnbuf1))) {
    printf("product:      (%s)\n", strip(fnbuf1));
  }
  strcpy(q, "serial");
  if (0 == read_first_line(p, fnbuf1, sizeof(fnbuf1))) {
    printf("serial:       (%s)\n", strip(fnbuf1));
  }
  return 0;
}

static int hdid(const char *devname) {
  struct stat st;
  struct my_hd_driveid did;
  off64_t size;
  int fd = open(devname, O_RDONLY, 0);
  printf("\n");
  printf("device:       (%s)\n", devname);
  if (fd < 0) {
    fprintf(stderr, "error: Cannot open device: %s: %s\n", devname, strerror(errno));
    return -1;
  }
  if (0 != fstat(fd, &st)) {
    fprintf(stderr, "error: fstat: %s: %s\n", devname, strerror(errno));
    close(fd);
    return -1;
  }
  if (!S_ISBLK(st.st_mode)) {
    fprintf(stderr, "error: Not a block device: %s\n", devname);
    close(fd);
    return -1;
  }
  /* TODO(pts): Indicate USB, read-only, removable, SSD. */
  printf("blockdev:     %d:%d\n", ((unsigned)st.st_rdev >> 8) & 0xffff, (unsigned char)st.st_rdev);
  /* always 0: printf("size:         %llu bytes\n", (unsigned long long)st.st_size); */
  size = lseek64(fd, 0, SEEK_END);
  if ((unsigned long long)size + 1 == 0U) {
    close(fd);
    fprintf(stderr, "error: lseek: %s: %s\n", devname, strerror(errno));
    return -1;
  }
  if ((unsigned long long)size < 100000000000ULL) {  /* 123 GB instead of 123456 MB. */
    printf("size:         %llu bytes, %llu%s MiB\n", (unsigned long long)size, (unsigned long long)(size >> 20), (size & 0xfffff) ? "+" : "");
  } else {
    printf("size:         %llu bytes, %llu%s GiB\n", (unsigned long long)size, (unsigned long long)(size >> 30), (size & 0x3fffffff) ? "+" : "");
  }
  if (ioctl(fd, HDIO_GET_IDENTITY, &did) != 0) {
    const int e = errno;
    close(fd);
    if (e == EINVAL || e == ENOTTY) {  /* Try USB in /sys. */
      if (0 != hdid_usb(devname, (unsigned)st.st_rdev)) return -1;
    } else if (e == ENOMSG || e == ENOMEDIUM) {
      /* TODO(pts): Report the trype of DVD reader in case of ENOMEDIUM. */
      fprintf(stderr, "error: Product info not found for device: %s\n", devname);
      return -1;
    } else {
      fprintf(stderr, "error: HDIO_GET_IDENTITY: %s: %s\n", devname, strerror(e));
      return -1;
    }
  } else {
    close(fd);
    /* Remove 2 bytes of junk. */
    did.model[sizeof(did.model) - 2] = '\0';
    printf("product:      (%s)\n", strip((char*)did.model));
    printf("serial:       (%s)\n", strip((char*)did.serial_no));
  }
  return 0;
}

static void hdid_all(void) {
  char fnbuf[32];
  DIR *dd = opendir("/sys/block");
  struct dirent *de;
  if (!dd) {
    fprintf(stderr, "error: opendir: /sys/block: %s\n", strerror(errno));
    return;
  }
  while ((de = readdir(dd)) != NULL) {
    const char *name = de->d_name;
    if (name[0] == '.' ||
        0 == strncmp(name, "dm-", 3) ||
        0 == strncmp(name, "loop", 4) ||
        0 == strncmp(name, "ram", 3) ||
        0 == strncmp(name, "fd", 2) ||  /* Floppy disk. */
        0 == strncmp(name, "sr", 2) ||  /* SCSI CD-ROM. */
        strlen(name) >= sizeof(fnbuf) - 6
       ) continue;
    /* Typically name is "sda" or "sdc", or "hda" on Linux <= 2.4.
     * Partitions (e.g. "sda1") are not included.
     */
    sprintf(fnbuf, "/dev/%s", name);
    hdid(fnbuf);
  }
  closedir(dd);
}

int main(int argc, char **argv) {
  (void)argc;
  if (!*++argv) {
    hdid_all();
  } else {
    while (*argv) {
      hdid(*argv++);
    }
  }
  return 0;
}
