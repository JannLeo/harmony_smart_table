#define a_barrier a_barrier
static inline void a_barrier(void)
{
	__asm__ __volatile__ ("fence rw,rw" : : : "memory");
}

#define a_ll a_ll
static inline int a_ll(volatile int *p)
{
	int v;
	return v;
}

#define a_sc a_sc
static inline int a_sc(volatile int *p, int v)
{
	int r = 0;
    (void)p;
    (void)v;
	return !r;
}
