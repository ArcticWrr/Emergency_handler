
#define SCALL_ERROR -1


#define SCALL(r, c, e) do { if((r = c) == SCALL_ERROR) { perror(e); exit(EXIT_FAILURE); } } while(0)
#define SNCALL(r, c, e) do { if((r = c) == NULL) { perror(e); exit(EXIT_FAILURE); } } while(0)
#define FCALL(c, e) do { if ((c) == EOF) { perror(e); exit(EXIT_FAILURE); } } while(0)
#define MALLOC(ptr, size, e, dim) do { if((ptr = malloc(size * dim)) == NULL) { perror(e); exit(EXIT_FAILURE); } } while(0)