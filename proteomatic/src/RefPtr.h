#pragma once

#include "stdio.h"


template <class TClass> class RefPtr
{
public:
	inline RefPtr();
	inline ~RefPtr();
	explicit inline RefPtr(TClass* pCppObject);
	inline RefPtr(const RefPtr<TClass>& src);
	inline void swap(RefPtr<TClass>& other);
	inline RefPtr<TClass>& operator=(const RefPtr<TClass>& src);
	inline TClass* operator->() const;
	inline TClass* get_Pointer() const;
	inline TClass operator*() const;
	inline operator bool() const;

private:
	void unref();

	TClass* mk_Object_;
	mutable int* mi_RefCount_;
};


template <class TClass> inline
TClass* RefPtr<TClass>::operator->() const
{
	return mk_Object_;
}


template <class TClass> inline
TClass* RefPtr<TClass>::get_Pointer() const
{
	return mk_Object_;
}


template <class TClass> inline
TClass RefPtr<TClass>::operator*() const
{
	return *mk_Object_;
}


template <class TClass> inline
RefPtr<TClass>::RefPtr()
	: mk_Object_(NULL)
	, mi_RefCount_(NULL)
{
}


template <class TClass> inline
RefPtr<TClass>::~RefPtr()
{
	unref();
}


template <class TClass> inline
void RefPtr<TClass>::unref()
{
	if (mi_RefCount_)
	{
		--(*mi_RefCount_);

		if (*mi_RefCount_ == 0)
		{
			if (mk_Object_)
			{
				delete mk_Object_;
				mk_Object_ = NULL;
			}

			delete mi_RefCount_;
			mi_RefCount_ = NULL;
		}
	}
}


template <class TClass> inline
RefPtr<TClass>::RefPtr(TClass *ak_Object_)
	: mk_Object_(ak_Object_)
	, mi_RefCount_(NULL)
{
	if (mk_Object_)
	{
		mi_RefCount_ = new int;
		*mi_RefCount_ = 1;
	}
}


template <class TClass> inline
RefPtr<TClass>::RefPtr(const RefPtr<TClass>& src)
	: mk_Object_(src.mk_Object_)
	, mi_RefCount_(src.mi_RefCount_)
{
	if (mk_Object_ && mi_RefCount_)
		++(*mi_RefCount_);
}


template <class TClass> inline
void RefPtr<TClass>::swap(RefPtr<TClass>& other)
{
	TClass *const mk_TempObject_ = mk_Object_;
	int* li_TempCount_ = mi_RefCount_;

	mk_Object_ = other.mk_Object_;
	mi_RefCount_ = other.mi_RefCount_;

	other.mk_Object_ = mk_TempObject_;
	other.mi_RefCount_ = li_TempCount_;
}


template <class TClass> inline
RefPtr<TClass>& RefPtr<TClass>::operator=(const RefPtr<TClass>& src)
{
	RefPtr<TClass> temp(src);
	this->swap(temp);
	return *this;
}


template <class TClass> inline
RefPtr<TClass>::operator bool() const
{
	return (mk_Object_ != NULL);
}
