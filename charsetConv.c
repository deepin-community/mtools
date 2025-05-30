/*  Copyright 2008,2009 Alain Knaff.
 *  This file is part of mtools.
 *
 *  Mtools is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Mtools is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Mtools.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Various character set conversions used by mtools
 */
#include "sysincludes.h"
#include "mtools.h"

#include <stdio.h>
#include <errno.h>
#include "file_name.h"


#ifdef HAVE_ICONV_H
#include <iconv.h>

struct doscp_t {
	iconv_t from;
	iconv_t to;
};

static const char *wcharCp=NULL;

/* FIXME: encodings after WCHAR_T should not all be tried in sequence,
 * but rather according to platform's sizeof(wchar_t) and endianness
 * https://stackoverflow.com/questions/62032729/using-iconv-with-wchar-t-on-linux
 */
static const char* wcharTries[] = {
	"WCHAR_T",
	"UTF-32BE", "UTF-32LE",
	"UTF-16BE", "UTF-16LE",
	"UTF-32", "UTF-16",
	"UCS-4BE", "UCS-4LE",
	"UCS-2BE", "UCS-2LE",
	"UCS-4", "UCS-2"
};

static const char *asciiTries[] = {
	"ASCII", "US-ASCII", "ASCII-GR", "ISO8859-1"
};

static const wchar_t *testString = L"ab";

/* Use SUSv2 prototype for iconv, makes more sense than POSIX-1.2007
 * https://stackoverflow.com/questions/45938990/why-does-the-iconv-function-need-a-non-const-inbuffer
 */
static inline size_t mt_iconv(iconv_t cd, const char **inbuf,
			      size_t *inbytesleft,
			      char **outbuf, size_t *outbytesleft) {
#ifdef HAVE_PRAGMA_DIAGNOSTIC
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
#endif
    return iconv(cd, inbuf, inbytesleft, outbuf, outbytesleft);
#ifdef HAVE_PRAGMA_DIAGNOSTIC
# pragma GCC diagnostic pop
#endif
}

static int try(const char *testCp) {
	size_t res;
	const char *inbuf = (const char *) testString;
	size_t inbufLen = 2*sizeof(wchar_t);
	char outbuf[3];
	char *outbufP = outbuf;
	size_t outbufLen = 2*sizeof(char);
	iconv_t test = 0;
	size_t i;

	for(i=0; i < sizeof(asciiTries) / sizeof(asciiTries[0]); i++) {
		test = iconv_open(asciiTries[i], testCp);
		if(test != (iconv_t) -1)
			break;
	}
	if(test == (iconv_t) -1)
		goto fail0;
	res = mt_iconv(test,
		       &inbuf, &inbufLen,
		       &outbufP, &outbufLen);
	iconv_close(test);
	if(res != 0 || outbufLen != 0 || inbufLen != 0)
		goto fail;
	if(memcmp(outbuf, "ab", 2))
		goto fail;
	/* fprintf(stderr, "%s ok\n", testCp); */
	return 1;
 fail:
 fail0:
	/*fprintf(stderr, "%s fail\n", testCp);*/
	return 0;
}

static const char *getWcharCp(void) {
	unsigned int i;
	if(wcharCp != NULL)
		return wcharCp;
	for(i=0; i< sizeof(wcharTries) / sizeof(wcharTries[0]); i++) {
		if(try(wcharTries[i]))
			return (wcharCp=wcharTries[i]);
	}
	fprintf(stderr, "No codepage found for wchar_t\n");
	return NULL;
}


doscp_t *cp_open(unsigned int codepage)
{
	char dosCp[17];
	doscp_t *ret;
	iconv_t from;
	iconv_t to;

	if(codepage == 0)
		codepage = mtools_default_codepage;
	if(codepage > 9999) {
		fprintf(stderr, "Bad codepage %d\n", codepage);
		return NULL;
	}

	if(getWcharCp() == NULL)
		return NULL;

	sprintf(dosCp, "CP%d", codepage);
	from = iconv_open(wcharCp, dosCp);
	if(from == (iconv_t)-1) {
		fprintf(stderr, "Error converting to codepage %d %s\n",
			codepage, strerror(errno));
		return NULL;
	}

	sprintf(dosCp, "CP%d//TRANSLIT", codepage);
	to   =  iconv_open(dosCp, wcharCp);
	if(to == (iconv_t)-1) {
		/* Transliteration not supported? */
		sprintf(dosCp, "CP%d", codepage);
		to   =  iconv_open(dosCp, wcharCp);
	}
	if(to == (iconv_t)-1) {
		iconv_close(from);
		fprintf(stderr, "Error converting to codepage %d %s\n",
			codepage, strerror(errno));
		return NULL;
	}

	ret = New(doscp_t);
	if(ret == NULL)
		return ret;
	ret->from = from;
	ret->to   = to;
	return ret;
}

void cp_close(doscp_t *cp)
{
	iconv_close(cp->to);
	iconv_close(cp->from);
	free(cp);
}

size_t dos_to_wchar(doscp_t *cp, const char *dos, wchar_t *wchar, size_t len)
{
	size_t r;
	size_t in_len=len;
	size_t out_len=len*sizeof(wchar_t);
	wchar_t *dptr=wchar;
	r=mt_iconv(cp->from, &dos,	&in_len, (char **)&dptr, &out_len);
	if(r == (size_t) -1)
		return r;
	*dptr = L'\0';
	return (size_t) (dptr-wchar);
}

/**
 * Converts len wide character to destination. Caller's responsibility to
 * ensure that dest is large enough.
 * mangled will be set if there has been an untranslatable character.
 */
static size_t safe_iconv(iconv_t conv, const wchar_t *wchar, char *dest,
			 size_t in_len, size_t out_len, int *mangled)
{
	size_t r;
	unsigned int i;
	char *dptr = dest;
	size_t len;

	in_len=in_len*sizeof(wchar_t);

	while(in_len > 0 && out_len > 0) {
		r=mt_iconv(conv, (const char**)&wchar,
			   &in_len, &dptr, &out_len);
		if(r == (size_t) -1 || errno != EILSEQ) {
			/* everything transformed, or error that is _not_ a bad
			 * character */
			break;
		}
		*mangled |= 1;

		if(out_len <= 0)
			break;
		if(dptr)
			*dptr++ = '_';
		in_len -= sizeof(wchar_t);

		wchar++;
		out_len--;
	}

	len = (size_t) (dptr-dest); /* how many dest characters have there been
				       generated */

	/* eliminate question marks which might have been formed by
	   untransliterable characters */
	for(i=0; i<len; i++) {
		if(dest[i] == '?') {
			dest[i] = '_';
			*mangled |= 1;
		}
	}
	return len;
}

void wchar_to_dos(doscp_t *cp,
		  wchar_t *wchar, char *dos, size_t len, int *mangled)
{
	safe_iconv(cp->to, wchar, dos, len, len, mangled);
}

#else

#include "codepage.h"

struct doscp_t {
	char *from_dos;
	char to_dos[0x80];
};

doscp_t *cp_open(unsigned int codepage)
{
	doscp_t *ret;
	int i;
	Codepage_t *cp;

	if(codepage == 0)
		codepage = 850;

	ret = New(doscp_t);
	if(ret == NULL)
		return ret;

	for(cp=codepages; cp->nr ; cp++)
		if(cp->nr == codepage) {
			ret->from_dos = cp->tounix;
			break;
		}

	if(ret->from_dos == NULL) {
		fprintf(stderr, "Bad codepage %d\n", codepage);
		free(ret);
		return NULL;
	}

	for(i=0; i<0x80; i++) {
		char native = ret->from_dos[i];
		if(! (native & 0x80))
			continue;
		ret->to_dos[native & 0x7f] = (char) (0x80 | i);
	}
	return ret;
}

void cp_close(doscp_t *cp)
{
	free(cp);
}

size_t dos_to_wchar(doscp_t *cp, const char *dos, wchar_t *wchar, size_t len)
{
	size_t i;

	for(i=0; i<len && dos[i]; i++) {
		char c = dos[i];
		if(c >= ' ' && c <= '~')
			wchar[i] = c;
		else {
			wchar[i] = cp->from_dos[c & 0x7f];
		}
	}
	wchar[i] = '\0';
	return i;
}


void wchar_to_dos(doscp_t *cp,
		  wchar_t *wchar, char *dos, size_t len, int *mangled)
{
	size_t i;
	for(i=0; i<len && wchar[i]; i++) {
		char c = wchar[i];
		if(c >= ' ' && c <= '~')
			dos[i] = c;
		else {
			dos[i] = cp->to_dos[c & 0x7f];
			if(dos[i] == '\0') {
				dos[i]='_';
				*mangled=1;
			}
		}
	}
}

#endif


#ifndef HAVE_WCHAR_H

typedef int mbstate_t;

static inline size_t wcrtomb(char *s, wchar_t wc, mbstate_t *ps UNUSEDP)
{
	*s = wc;
	return 1;
}

static inline size_t mbrtowc(wchar_t *pwc, const char *s,
			     size_t n UNUSEDP, mbstate_t *ps UNUSEDP)
{
	*pwc = *s;
	return *pwc ? 1 : 0;
}

#endif

#ifdef HAVE_ICONV_H

#include <langinfo.h>

static iconv_t to_native = 0;

static void initialize_to_native(void)
{
	char *li, *cp;
	size_t len;
	if(to_native != 0)
		return;
	li = nl_langinfo(CODESET);
	len = strlen(li) + 11;
	if(getWcharCp() == NULL)
		exit(1);
	cp = safe_malloc(len);
	strcpy(cp, li);
	strcat(cp, "//TRANSLIT");
	to_native = iconv_open(cp, wcharCp);
	if(to_native == (iconv_t) -1)
		to_native = iconv_open(li, wcharCp);
	if(to_native == (iconv_t) -1)
		fprintf(stderr, "Could not allocate iconv for %s\n", cp);
	free(cp);
	if(to_native == (iconv_t) -1)
		exit(1);
}



#endif


/**
 * Convert wchar string to native, converting at most len wchar characters
 * Returns number of generated native characters
 */
size_t wchar_to_native(const wchar_t *wchar, char *native, size_t len,
		       size_t out_len
#ifndef HAVE_ICONV_H
		       UNUSEDP
#endif
		       )
{
#ifdef HAVE_ICONV_H
	int mangled;
	size_t r;
	initialize_to_native();
	len = wcsnlen(wchar,len);
	r=safe_iconv(to_native, wchar, native, len, out_len, &mangled);
	native[r]='\0';
	return r;
#else
	size_t i;
	char *dptr = native;
	mbstate_t ps;
	memset(&ps, 0, sizeof(ps));
	for(i=0; i<len && wchar[i] != 0; i++) {
		size_t r = wcrtomb(dptr, wchar[i], &ps);
		if(r == (size_t) -1
#ifdef EILSEQ
		   && errno == EILSEQ
#endif
		   ) {
			r=1;
			*dptr='_';
		}
		dptr+=r;
	}
	*dptr='\0';
	return (size_t) (dptr-native);
#endif
}

/**
 * Convert native string to wchar string, generating at most len wchar
 * characters. If end is supplied, stop conversion when source pointer
 * exceeds end. Returns number of generated wchars
 */
size_t native_to_wchar(const char *native, wchar_t *wchar, size_t len,
		       const char *end, int *mangled)
{
	mbstate_t ps;
	unsigned int i;
	memset(&ps, 0, sizeof(ps));

	for(i=0; i<len && (native < end || !end); i++) {
		size_t r = mbrtowc(wchar+i, native, len, &ps);
		if(r == (size_t) -1) {
			/* Unconvertible character. Just pretend it's Latin1
			   encoded (if valid Latin1 character) or substitute
			   with an underscore if not
			*/
			char c = *native;
			if(c >= '\xa0' && c < '\xff')
				wchar[i] = (wchar_t) (c & 0xff);
			else
				wchar[i] = '_';
			memset(&ps, 0, sizeof(ps));
			r=1;
		}
		if(r == 0)
			break;
		native += r;
	}
	if(mangled && ((end && native < end) || (!end && *native &&  i == len)))
		*mangled |= 3;
	wchar[i]='\0';
	return i;
}

