/** \file ctemplate.cpp
 *
 * Created: JohnE, 2008-07-29
 */


/*
DISCLAIMER:
The author(s) of this file's contents release it into the public domain, without
express or implied warranty of any kind. You may use, modify, and redistribute
this file freely.
*/


#include <list>
#include <string>
#include "tinyxml/tinyxml.h"
#include "ref.hpp"


static void CharArrayDeleter(char* a)
{
	delete[] a;
}


static void Replace
 (TiXmlElement* ar_el,
  const std::string& var,
  FILE* out)
{
	const char* attrval = ar_el->Attribute(var.c_str());
	if (attrval && strlen(attrval) > 0)
		fputs(attrval, out);
}


int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		fprintf(stderr,
		 "\nctemplate needs 2 arguments: a component manifest file and a "
		 "template file\n");
		return 1;
	}

	TiXmlDocument doc(argv[1]);
	if (!doc.LoadFile())
	{
		fprintf(stderr,
		 "\nCouldn't load '%s' as a component manifest\n", argv[1]);
		return 1;
	}

	FILE* infile = fopen(argv[2], "r");
	if (!infile)
	{
		fprintf(stderr, "\nCouldn't load '%s' for input\n", argv[2]);
		return 1;
	}
	RefType< FILE >::Ref ifcloser(infile, fclose);

	RefType< char >::Ref str(new char[1024 * 1024], CharArrayDeleter);
	char* estr = RefGetPtr(str);

	std::list< TiXmlElement* > search_els;
	search_els.push_back(doc.RootElement());
	while (!search_els.empty())
	{
		TiXmlElement* el = search_els.front();
		search_els.pop_front();
		if (strcmp(el->Value(), "Archive") == 0)
		{
			fseek(infile, 0, SEEK_SET);
			while (fgets(estr, 1024 * 1024 - 1, infile))
			{
				for (size_t at = 0; estr[at]; ++at)
				{
					if (estr[at] == '%')
					{
						size_t end = at + 1;
						while (estr[end] && estr[end] != '%')
							++end;
						if (estr[end])
						{
							if (end - at == 1)
								fputc('%', stdout);
							else
							{
								Replace(el,
								 std::string(estr + at + 1, end - at - 1),
								 stdout);
							}
						}
						at = end;
					}
					else
						fputc(estr[at], stdout);
				}
			}
		}
		else
		{
			for (TiXmlElement* child = el->FirstChildElement();
			 child;
			 child = child->NextSiblingElement())
				search_els.push_back(child);
		}
	}

	return 0;
}

