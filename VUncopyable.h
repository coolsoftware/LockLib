//////////////////////////////////////////////////////////////////
//
// Copyright 2013 CoolSoftware. http://blog.coolsoftware.ru/
//
// Single-write-multiple-read.
//
// You can freely use this program and code for your needs.
//
// Please, don't remove this copyright.
//
//////////////////////////////////////////////////////////////////

#pragma once

#ifndef V_UNCOPYABLE_H_
#define V_UNCOPYABLE_H_

class VUncopyable
{
protected:
	VUncopyable() {}
	~VUncopyable() {}

private:
	VUncopyable(const VUncopyable&);
	VUncopyable& operator=(const VUncopyable&);
};

#endif //V_UNCOPYABLE_H_