#ifndef TRACKED_INSTALL_HPP_INC
#define TRACKED_INSTALL_HPP_INC


#include <stdexcept>
#include <list>
#include "ref.hpp"


class TCHolderBase
{
public:
	virtual ~TCHolderBase() {}

	virtual bool Revert() = 0;
	virtual TCHolderBase* Clone() const = 0;
};

template< class T >
class TCHolder : public TCHolderBase
{
public:
	TCHolder(const T& ch)
	 : change(ch)
	{
	}

	virtual bool Revert()
	{
		return change.Revert();
	}

	virtual TCHolderBase* Clone() const
	{
		return new TCHolder< T >(change);
	}

private:
	T change;
};

class TrackedChange
{
public:
	template< class T >
	TrackedChange(const T& ch)
	 : tchb(new TCHolder< T >(ch))
	{
	}
	TrackedChange(const TrackedChange& c)
	 : tchb(c.tchb->Clone())
	{
	}
	~TrackedChange()
	{
		delete tchb;
	}

	TrackedChange& operator = (const TrackedChange& c)
	{
		tchb = c.tchb->Clone();
		return *this;
	}

	bool Revert()
	{
		return tchb->Revert();
	}

private:
	TCHolderBase* tchb;
};


class ChangeSet
{
public:
	typedef RefType< ChangeSet >::Ref Ref;

	void RevertAll();

	void Push(const TrackedChange& ch);

private:
	std::list< TrackedChange > cset_misc;
};


#if 0
class InstFailure : public std::runtime_error
{
public:
	InstFailure
	 (const std::string& reason,
	  ChangeSet::Ref cset = ChangeSet::Ref());
	virtual ~InstFailure() throw () {}

	std::string Reason()
	{
		return fail_msg;
	}

	void RevertChangeSet();

private:
	std::string fail_msg;
	ChangeSet::Ref changeset;
};
#endif


#endif // TRACKED_INSTALL_HPP_INC
