
#include "tracked_install.hpp"


void ChangeSet::RevertAll()
{
	for (std::list< TrackedChange >::iterator it = cset_misc.begin();
	 it != cset_misc.end();
	 ++it)
	{
		it->Revert();
	}
}


void ChangeSet::Push(const TrackedChange& ch)
{
	cset_misc.push_front(ch);
}


#if 0
InstFailure::InstFailure(ChangeSet::Ref cset, const std::string& reason)
 : std::runtime_error("Installation failure"),
 fail_msg(reason),
 changeset(cset)
{
}


void InstFailure::RevertChangeSet()
{
	if (changeset)
		changeset->RevertAll();
}
#endif
