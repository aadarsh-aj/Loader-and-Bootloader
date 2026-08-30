#include "loader.h"

Elf32_Ehdr *ehdr;
Elf32_Phdr *phdr;
int fd;

/* raw bytes of the whole ELF file, and the segment we mmap for execution -
 * kept as globals so loader_cleanup() can actually free/unmap them later */
static char *file_buf = NULL;
static void *seg_mem = NULL;
static size_t seg_size = 0;

/*
 * release memory and other cleanups
 */
void loader_cleanup() {
  if (seg_mem != NULL) {
    munmap(seg_mem, seg_size);
    seg_mem = NULL;
  }

  if (file_buf != NULL) {
    free(file_buf);
    file_buf = NULL;
  }

  if (fd > 0) {
    close(fd);
  }
}

/*
 * Load and run the ELF executable file
 *
 * argv[1] is expected to be the path of a statically linked, 32-bit ELF
 * executable (see factorial.c in test/). We don't touch any libelf style
 * API here - the file is just read into memory and the header structs
 * are laid directly on top of the bytes, the way the assignment wants it.
 */
void load_and_run_elf(char** argv) {
  fd = open(argv[1], O_RDONLY);
  if (fd < 0) {
    perror("open");
    exit(1);
  }

  /* figure out how big the file is so we know how much to read/malloc */
  off_t file_size = lseek(fd, 0, SEEK_END);
  if (file_size <= 0) {
    fprintf(stderr, "loader: could not determine size of %s\n", argv[1]);
    exit(1);
  }
  lseek(fd, 0, SEEK_SET);

  file_buf = malloc(file_size);
  if (file_buf == NULL) {
    fprintf(stderr, "loader: malloc failed while reading ELF file\n");
    exit(1);
  }

  ssize_t n = read(fd, file_buf, file_size);
  if (n != file_size) {
    fprintf(stderr, "loader: short read on %s\n", argv[1]);
    exit(1);
  }

  /* 1. the ELF header always sits at offset 0 */
  ehdr = (Elf32_Ehdr *)file_buf;

  if (memcmp(ehdr->e_ident, ELFMAG, SELFMAG) != 0) {
    fprintf(stderr, "loader: %s is not an ELF file\n", argv[1]);
    exit(1);
  }
  if (ehdr->e_ident[EI_CLASS] != ELFCLASS32) {
    fprintf(stderr, "loader: only 32-bit ELF binaries are supported\n");
    exit(1);
  }

  /* 2. walk the PHDR table looking for the PT_LOAD segment that
   *    contains the entry point address (e_entry) */
  phdr = (Elf32_Phdr *)(file_buf + ehdr->e_phoff);

  Elf32_Phdr *load_phdr = NULL;
  for (int i = 0; i < ehdr->e_phnum; i++) {
    Elf32_Phdr *cur = &phdr[i];
    if (cur->p_type == PT_LOAD &&
        ehdr->e_entry >= cur->p_vaddr &&
        ehdr->e_entry < cur->p_vaddr + cur->p_memsz) {
      load_phdr = cur;
      break;
    }
  }

  if (load_phdr == NULL) {
    fprintf(stderr, "loader: no PT_LOAD segment contains the entry point\n");
    exit(1);
  }

  /* 3. mmap p_memsz bytes and copy the segment content in, exactly as
   *    described in the assignment PDF (we're told to ignore p_filesz,
   *    p_paddr, p_flags, p_align for this assignment) */
  seg_size = load_phdr->p_memsz;
  seg_mem = mmap(NULL, seg_size, PROT_READ | PROT_WRITE | PROT_EXEC,
                 MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
  if (seg_mem == MAP_FAILED) {
    perror("mmap");
    exit(1);
  }

  memcpy(seg_mem, file_buf + load_phdr->p_offset, seg_size);

  /* 4. the entry point isn't necessarily the start of the segment, so
   *    walk forward from p_vaddr to e_entry inside our mapped copy */
  unsigned int entry_offset = ehdr->e_entry - load_phdr->p_vaddr;
  void *entry_addr = (char *)seg_mem + entry_offset;

  /* 5. typecast to a function pointer matching _start() in factorial.c */
  int (*_start)(void) = (int (*)(void))entry_addr;

  /* 6. call it and print what we got back */
  int result = _start();
  printf("User _start return value = %d\n", result);
}
