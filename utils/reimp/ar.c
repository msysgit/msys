#include <reimp.h>
#include <ctype.h>

#define LONG_NAMES_NAME "//              "
char *long_names = NULL;

/* read an archive header */
int
ar_read_header (struct ar_hdr *hdr, FILE *f)
{
  size_t size;
  //fseek(f, (ftell(f) + 1) & ~1, SEEK_SET);	// or should it be fixed here?
  if (fread (hdr, sizeof (*hdr), 1, f) == 1)
    {
      if (memcmp (ARFMAG, hdr->ar_fmag, 2) != 0)
        return 0;
      if (memcmp (hdr->ar_name, LONG_NAMES_NAME, 16) == 0)
        {
          size = strtol (hdr->ar_size, NULL, 10);
          long_names = xmalloc (size);
          if (fread (long_names, size, 1, f) != 1)
            error (0, "unexpected end-of-file\n");
        }
    }
  else
    return 0;
}

char *
get_ar_name (struct ar_hdr *hdr)
{
  int i;
  static char buf[32];
  if (hdr->ar_name[0] == '/' && isdigit (hdr->ar_name[1]) && long_names)
    return long_names + strtol (hdr->ar_name + 1, NULL, 10);

  for (i = 0; i < 16; i++)
    {
      if (hdr->ar_name[i] == ' ')
        {
          hdr->ar_name[i] = '\0';
          return hdr->ar_name;
        }
    }
  memcpy (buf, hdr->ar_name, 16);
  buf[16] = '\0';
  return buf;
}
