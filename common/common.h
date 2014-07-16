/*
 * Alper Akcan (alper.akcan@gmail.com) - 15.03.2011
 */

#if defined(DEBUG)
#define debugf(a...) { \
	fprintf(stderr, "debug: "); \
	fprintf(stderr, a); \
	fprintf(stderr, " [%s (%s:%d)]\n", __FUNCTION__, __FILE__, __LINE__); \
}
#else
#define debugf(a...)
#endif

#define errorf(a...) { \
	fprintf(stderr, "error: "); \
	fprintf(stderr, a); \
	fprintf(stderr, " [%s (%s:%d)]\n", __FUNCTION__, __FILE__, __LINE__); \
	exit(-1); \
}

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MIN3(a, b, c) MIN(a, MIN(b, c))

#define alloc_malloc(a) ({ \
	void *__ptr = malloc(a); \
	if (__ptr == NULL) { \
		errorf("malloc() failed"); \
	} \
	__ptr; \
})

#define alloc_free(a) ({ \
	if (a != NULL) { \
		free(a); \
		a = NULL; \
	} \
})

static inline int alloc_tolower (char c)
{
	if (c < 'a') {
		return c + ('a' - 'A');
	}
	return c;
}

#if 1
static inline int alloc_strcmp (char *a, char *b)
{
	while (*a && *b) {
		if (*a++ != *b++) {
			return -1;
		}
	}
	if (*a || *b) {
		return -1;
	}
	return 0;
}
#else
/*
 * Fast strcmp.  This works one int at a time, using aligned pointers
 * if possible, misaligned pointers if necessary.  To avoid taking
 * faults from going off the end of a page, the code is careful to go
 * a byte-at-a-time when a misaligned pointer is near a page boundary.
 * The code is almost portable, but see the assumptions below.
 */

/*
 * ASSUMPTIONS:
 * sizeof (int) is not greater than 8.
 * sizeof (int) is a power of 2.
 * An int pointer can always be dereferenced even if it is not properly
 *   aligned (though aligned references are assumed to be faster).
 * It is OK to assign bogus values to a pointer (in particular, a
 *   value that is before the beginning of the string) as long as that
 *   pointer is only used with indices big enough to bring us back into
 *   the string.
 * It is OK to reference bytes past the end of a string as long as we
 *   don't cross a page boundary.
 */

#include <limits.h>
#include <unistd.h>
#include <sys/user.h>

/*
 * This strange expression will test to see if *any* byte in the int is
 * a NUL.  The constants are big enough to allow for ints up to 8 bytes.
 * The two arguments are actually two copies of the same value; this
 * allows the compiler freedom to play with both values for efficiency.
 */
#define	ANYNUL(i1, i2)	(((i1) - (int)0x0101010101010101LL) & ~(i2) & \
		(int)0x8080808080808080ULL)

static inline int alloc_strcmp (const char *str1, const char *str2)
{
	int *s1, *s2;
	int i1, i2;
	int count;
	static int pagesize;

	if (str1 == str2)
		return (0);

	/*
	 * Go 1 byte at a time until at least one pointer is word aligned.
	 * Assumes that sizeof (int) is a power of 2.
	 */
	while ((((int) str1) & (sizeof (int) - 1)) &&
	    (((int) str2) & (sizeof (int) - 1))) {
one_byte:
		if (*str1 != *str2)
			return ((unsigned char)*str1 - (unsigned char)*str2);
		if (*str1 == '\0')
			return (0);
		++str1;
		++str2;
	}

	/*
	 * If one pointer is misaligned, we must be careful not to
	 * dereference it when it points across a page boundary.
	 * If we did, we might go past the end of the segment and
	 * get a SIGSEGV.  Set "count" to the number of ints we can
	 * scan before running into such a boundary.
	 */
	count = INT_MAX;
	if (((int) str1) & (sizeof (int) - 1)) {
		if (pagesize == 0)
			pagesize = PAGE_SIZE;
		count = (pagesize - ((int)str1 & (pagesize - 1))) /
			sizeof (int);
	} else if (((int) str2) & (sizeof (int) - 1)) {
		if (pagesize == 0)
			pagesize = PAGE_SIZE;
		count = (pagesize - ((int)str2 & (pagesize - 1))) /
			sizeof (int);
	}

	s1 = (void *) str1;
	s2 = (void *) str2;

	/*
	 * Go "sizeof (int)" bytes at a time until at least one pointer
	 * is word aligned.
	 *
	 * Unwrap the loop for even a bit more speed.
	 */
	for (;;) {
		/*
		 * Check whether we can test the next 4 ints without
		 * hitting a page boundary.  If we can only test 1, 2,
		 * or 3, go and do that first.  If we can't check any
		 * more, go and test one byte, realign, and start again.
		 */
		count -= 4;
		switch (count) {
		case -1:
			--s1;
			--s2;
			goto do3;	/* check only 3 ints */
		case -2:
			s1 -= 2;
			s2 -= 2;
			goto do2;	/* check only 2 ints */
		case -3:
			s1 -= 3;
			s2 -= 3;
			goto do1;	/* check only 1 int */
		case -4:
		case -5:		/* -5, -6, and -7 come up on the */
		case -6:		/* next time around after we do one */
		case -7:		/* of the 3 gotos above */
			str1 = (void *) s1;
			str2 = (void *) s2;
			goto one_byte;
			/*
			 * The goto above should be explained.  By going
			 * into the middle of the loop, it makes sure
			 * that we advance at least one byte.  We will
			 * stay in that loop until the misaligned pointer
			 * becomes aligned (at the page boundary).  We
			 * will then break out of that loop with the
			 * formerly misaligned pointer now aligned, the
			 * formerly aligned pointer now misaligned, and
			 * we will come back into this loop until the
			 * latter pointer reaches a page boundary.
			 */
		default:		/* at least 4 ints to go */
			break;
		}

		i1 = s1[0];
		i2 = s2[0];
		if (i1 != i2)
			break;
		else if (ANYNUL(i1, i2))
			return (0);

do3:
		i1 = s1[1];
		i2 = s2[1];
		if (i1 != i2)
			break;
		else if (ANYNUL(i1, i2))
			return (0);

do2:
		i1 = s1[2];
		i2 = s2[2];
		if (i1 != i2)
			break;
		else if (ANYNUL(i1, i2))
			return (0);

do1:
		i1 = s1[3];
		i2 = s2[3];
		if (i1 != i2)
			break;
		else if (ANYNUL(i1, i2))
			return (0);

		s1 += 4;
		s2 += 4;
	}

	return -1;
}
#endif

static inline int alloc_strncmp (char *a, unsigned int al, char *b, unsigned int bl)
{
	while (*a && *b && al-- && bl--) {
		if (*a++ != *b++) {
			return -1;
		}
	}
	if (!al && !bl) {
		return 0;
	}
	if (*a || *b) {
		return -1;
	}
	return 0;
}

static inline unsigned int alloc_hash (const char *str)
{
	#define HASHWORDBITS 32
	unsigned int hval, g;
	hval = 0;
	while (*str) {
		hval <<= 4;
		hval += (unsigned char) *str++;
		g = hval & ((unsigned long int) 0xf << (HASHWORDBITS - 4));
		if (g != 0) {
			hval ^= g >> (HASHWORDBITS - 8);
			hval ^= g;
		}
	}
	return hval;
}

static inline unsigned int alloc_nhash (const char *str, unsigned int n)
{
	#define HASHWORDBITS 32
	unsigned int hval, g;
	hval = 0;
	while (*str && n--) {
		hval <<= 4;
		hval += (unsigned char) *str++;
		g = hval & ((unsigned long int) 0xf << (HASHWORDBITS - 4));
		if (g != 0) {
			hval ^= g >> (HASHWORDBITS - 8);
			hval ^= g;
		}
	}
	return hval;
}
